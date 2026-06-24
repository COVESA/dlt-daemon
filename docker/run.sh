#!/bin/bash
#
# SPDX-License-Identifier: MPL-2.0
#
# Helper script to run code-quality checks inside the dlt-daemon-ci Docker container.
#
# Usage:
#   ./docker/run.sh <command>
#
# Commands:
#   configure     Generate CMake build with compile_commands.json
#   format        Check clang-format
#   fix-format    Apply clang-format fixes
#   tidy          Run clang-tidy
#   fix-tidy      Apply clang-tidy fixes
#   cppcheck      Run cppcheck
#   shell         Drop into an interactive shell in the container
#   build         Full build (cmake + make)
#
# The image is pulled from Docker Hub (drmint/dlt-daemon-ci) on first run.
# Use './docker/run.sh rebuild' to force a local rebuild instead.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
IMAGE_NAME="drmint/dlt-daemon-ci"
CONTAINER_NAME="dlt-daemon-ci-run"

# Track the running container so we can clean it up on Ctrl+C
RUNNING_CONTAINER=""

cleanup()
{
    if [[ -n "${RUNNING_CONTAINER}" ]]; then
        echo ""
        echo "Cleaning up container ${RUNNING_CONTAINER}..."
        docker stop "${RUNNING_CONTAINER}" >/dev/null 2>&1 || true
        docker rm -f "${RUNNING_CONTAINER}" >/dev/null 2>&1 || true
        RUNNING_CONTAINER=""
    fi
}

trap cleanup INT TERM

# Pull the image from Docker Hub if it doesn't exist locally
build_image()
{
    if ! docker image inspect "${IMAGE_NAME}" >/dev/null 2>&1; then
        echo "Pulling Docker image ${IMAGE_NAME}..."
        docker pull "${IMAGE_NAME}"
    fi
}

# Force rebuild the image locally from Dockerfile
rebuild_image()
{
    echo "Rebuilding Docker image ${IMAGE_NAME} from Dockerfile..."
    docker build --network=host -t "${IMAGE_NAME}" "${SCRIPT_DIR}"
}

# Print usage
show_help()
{
    cat <<EOF
Usage: ./docker/run.sh <command>

Commands:
    configure      Generate CMake build with compile_commands.json
    format         Check clang-format
    fix-format     Apply clang-format fixes
    tidy           Run clang-tidy
    fix-tidy       Apply clang-tidy fixes
    cppcheck       Run cppcheck
    build          Full build (cmake + make)
    test           Build and run unit tests (cmake + make + ctest)
    coverage       Build with coverage, run tests, generate lcov report
    asan           Build with AddressSanitizer and run tests
    devtest        Build and run DLT regression tests
    shell          Drop into an interactive shell in the container
    all            Run format + tidy + cppcheck
    rebuild        Force rebuild the Docker image

The Docker image is built automatically on first run.
EOF
}

# Run a command in the container
run_in_container()
{
    build_image
    local cid="dlt-daemon-ci-$$-${RANDOM}"
    RUNNING_CONTAINER="${cid}"
    docker run --rm --name "${cid}" --init \
        --network=host \
        --user "$(id -u):$(id -g)" \
        -v "${ROOT_DIR}:/workspace" \
        -v "/tmp:/tmp" \
        -w /workspace \
        "${IMAGE_NAME}" "$@"
    local rc=$?
    RUNNING_CONTAINER=""
    return ${rc}
}

case "${1:-}" in
    configure)
        run_in_container cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
        ;;
    format)
        run_in_container ./check.sh format
        ;;
    fix-format)
        run_in_container ./check.sh fix-format
        ;;
    tidy)
        run_in_container ./check.sh tidy --all
        ;;
    fix-tidy)
        run_in_container ./check.sh fix-tidy --all
        ;;
    cppcheck)
        run_in_container ./check.sh cppcheck
        ;;
    build)
        run_in_container cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
        run_in_container cmake --build build -j"$(nproc)"
        ;;
    test)
        run_in_container cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
        run_in_container cmake --build build -j"$(nproc)"
        run_in_container bash -c 'cd build && ctest --rerun-failed --output-on-failure'
        ;;
    coverage)
        run_in_container cmake -B build-cov \
            -DCMAKE_BUILD_TYPE=Release \
            -DWITH_DLT_COVERAGE=ON \
            -DBUILD_GMOCK=OFF
        run_in_container cmake --build build-cov --config Release -- -j"$(nproc)"
        run_in_container bash -c 'cd build-cov && ctest -C Release --rerun-failed --output-on-failure'
        run_in_container bash util/dlt_coverage_report/lcov_report_generator.sh build-cov -xe
        ;;
    asan)
        run_in_container cmake -B build-asan \
            -DCMAKE_BUILD_TYPE=Debug \
            -DWITH_DLT_DEBUGGERS=ON \
            -DWITH_DLT_COVERAGE=OFF \
            -DBUILD_GMOCK=OFF
        run_in_container cmake --build build-asan --config Debug -- -j"$(nproc)"
        run_in_container bash -c 'cd build-asan && ASAN_OPTIONS=detect_leaks=1:abort_on_error=1:print_summary=1 ctest -C Debug --rerun-failed --output-on-failure'
        ;;
    devtest)
        run_in_container cmake -B build \
            -DCMAKE_BUILD_TYPE=Debug \
            -DWITH_DLT_TESTS=ON \
            -DWITH_DLT_EXAMPLES=ON \
            -DWITH_DLT_CONSOLE=ON \
            -DWITH_DLT_SYSTEM=ON \
            -DWITH_DLT_FILETRANSFER=ON \
            -DWITH_DLT_ADAPTOR=ON \
            -DWITH_DLT_ADAPTOR_STDIN=ON \
            -DWITH_DLT_ADAPTOR_UDP=ON \
            -DWITH_DLT_LOGSTORAGE_GZIP=OFF \
            -DWITH_EXTENDED_FILTERING=OFF \
            -DWITH_DLT_LOG_LEVEL_APP_CONFIG=OFF \
            -DWITH_DLT_TRACE_LOAD_CTRL=OFF \
            -DWITH_UDP_CONNECTION=OFF \
            -DWITH_DLT_USE_IPv6=ON \
            -DWITH_SYSTEMD=OFF \
            -DWITH_DLT_SHM_ENABLE=OFF
        run_in_container cmake --build build -- -j"$(nproc)"
        run_in_container bash testscripts/oss_regression_test.sh
        ;;
    shell)
        build_image
        docker run --rm -it --init \
            --network=host \
            --user "$(id -u):$(id -g)" \
            -v "${ROOT_DIR}:/workspace" \
            -v "/tmp:/tmp" \
            -w /workspace \
            "${IMAGE_NAME}" /bin/bash
        ;;
    all)
        echo "=== clang-format ==="
        run_in_container ./check.sh format
        echo "=== clang-tidy ==="
        run_in_container ./check.sh tidy
        echo "=== cppcheck ==="
        run_in_container ./check.sh cppcheck
        echo "=== ALL CHECKS PASSED ==="
        ;;
    rebuild)
        rebuild_image
        docker tag "${IMAGE_NAME}" "dlt-daemon-ci:latest"
        ;;
    ""|-h|--help|help)
        show_help
        ;;
    *)
        # Pass through to check.sh for any other command
        run_in_container ./check.sh "$@"
        ;;
esac
