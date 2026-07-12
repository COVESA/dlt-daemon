#!/bin/bash

set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

ROOT_DIR=$(git rev-parse --show-toplevel)
cd "${ROOT_DIR}"

DEFAULT_BRANCH=$(git remote show origin 2>/dev/null | sed -n '/HEAD branch/s/.*: //p')
DEFAULT_BRANCH=${DEFAULT_BRANCH:-master}
EXCLUDE_PATTERNS=(
  # Fixme: exclude unsupported / external / non-target trees
  '^include/'
  '^src/android/'
  '^src/dlt-qnx-system/'
  '^src/core_dump_handler/'
  '^src/dbus/'
  '^src/kpi/'
  '^src/tests/'
  '^qnx/'
  '^googletest/'
  '^doc/'
  '^build/'
  '^tests/'
  '^tests/mod_system_logger/'
)

EXCLUDE_REGEX="$(IFS='|'; echo "${EXCLUDE_PATTERNS[*]}")"

echo_exclude_regex()
{
    print_header "EXCLUDE REGEX"
    echo "${EXCLUDE_REGEX}"
}

show_help()
{
cat <<EOF
DLT Local CI Helper

Usage:
    ./check.sh <command>

Commands:

    debug
        Show commits and files included in pipeline mode

    local
        Check modified and staged files only

    pipeline
        Check files touched by commits not yet present
        in origin/${DEFAULT_BRANCH}

    all
        Run pipeline checks plus cppcheck

    format
        Run clang-format on pipeline files

    fix-format
        Apply clang-format on pipeline files

    tidy
        Run clang-tidy on pipeline files

    fix-tidy
        Apply clang-tidy fixes on pipeline files

    cppcheck
        Run cppcheck on pipeline files

    configure
        Regenerate build directory and compile database

    commit
        Validate latest commit message

    help
        Show this help

Examples:

    ./check.sh debug
    ./check.sh local
    ./check.sh pipeline
    ./check.sh all

EOF
}

print_header()
{
    echo
    echo "=================================================="
    echo "$1"
    echo "=================================================="
}

check_tool()
{
    command -v "$1" >/dev/null 2>&1 || {
        echo "Missing dependency: $1"
        exit 1
    }
}

# Prefer versioned clang tools (clang-format-16 / clang-tidy-16) to match the
# CI pipeline (Docker container with LLVM 16).  Fall back to unversioned names.
CLANG_FORMAT=$(command -v clang-format-16 || command -v clang-format-18 || command -v clang-format)
CLANG_TIDY=$(command -v clang-tidy-16 || command -v clang-tidy-18 || command -v clang-tidy)

get_local_files()
{
    {
    git diff --name-only HEAD
    git diff --cached --name-only
    } \
    | sort -u \
    | grep -E '^(src|include|tests)/.*\.(c|cc|cpp|cxx|h|hh|hpp|hxx)$' \
    | grep -Ev "${EXCLUDE_REGEX}" \
    || true
}

get_all_files()
{
    git ls-files \
    | grep -E '^(src|include|tests)/.*\.(c|cc|cpp|cxx|h|hh|hpp|hxx)$' \
    | grep -Ev "${EXCLUDE_REGEX}" \
    || true
}

get_pipeline_files()
{
    git fetch origin "${DEFAULT_BRANCH}" >/dev/null 2>&1 || true

    git log \
        --format="" \
        --name-only \
        "origin/${DEFAULT_BRANCH}..HEAD" \
    | sort -u \
    | grep -E '^(src|include|tests)/.*\.(c|cc|cpp|cxx|h|hh|hpp|hxx)$' \
    | grep -Ev "${EXCLUDE_REGEX}" \
    | while read -r FILE
      do
          [ -z "$FILE" ] && continue

          git ls-files \
              --error-unmatch \
              "$FILE" >/dev/null 2>&1 \
              && echo "$FILE"
      done \
    || true
}

get_files()
{
    if [ "${FILE_MODE}" = "local" ]
    then
        get_local_files
    elif [ "${FILE_MODE}" = "all" ]
    then
        get_all_files
    else
        get_pipeline_files
    fi
}

show_pipeline_debug()
{
    print_header "PIPELINE COMMITS"

    git fetch origin "${DEFAULT_BRANCH}" >/dev/null 2>&1 || true

    git log --oneline \
        "origin/${DEFAULT_BRANCH}..HEAD"

    echo

    print_header "RAW PIPELINE FILES"

    git log \
        --format="" \
        --name-only \
        "origin/${DEFAULT_BRANCH}..HEAD" \
    | sort -u \
    | grep -E '^(src|include|tests)/.*\.(c|cc|cpp|cxx|h|hh|hpp|hxx)$' \
    || true

    echo

    print_header "FILTERED PIPELINE FILES"

    get_pipeline_files
}

ensure_compile_database()
{
    check_tool cmake

    local RECONFIGURE=0

    if [ ! -f build/compile_commands.json ]
    then
        RECONFIGURE=1
    fi

    if [ ! -f build/CMakeCache.txt ]
    then
        RECONFIGURE=1
    fi

    if [ "${RECONFIGURE}" -eq 0 ] && [ -f build/compile_commands.json ]
    then
        local FIRST_DIR
        FIRST_DIR=$(python3 -c "
import json, sys
with open('build/compile_commands.json') as f:
    data = json.load(f)
if data:
    print(data[0]['directory'])
" 2>/dev/null || true)

        if [ -n "${FIRST_DIR}" ] && [ ! -d "${FIRST_DIR}" ]
        then
            echo "compile_commands.json has stale paths (${FIRST_DIR}), reconfiguring..."
            rm -rf build/CMakeCache.txt build/CMakeFiles
            RECONFIGURE=1
        fi
    fi

    if [ "${RECONFIGURE}" -eq 1 ]
    then
        print_header "CONFIGURE"

        cmake \
            -B build \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
            -DWITH_DLT_SYSTEM=ON \
            -DWITH_DLT_FILETRANSFER=ON \
            -DWITH_UDP_CONNECTION=ON \
            -DWITH_SYSTEMD=ON \
            -DWITH_SYSTEMD_WATCHDOG=ON \
            -DWITH_SYSTEMD_JOURNAL=ON
    fi
}

run_format()
{
    print_header "CLANG-FORMAT"

    check_tool "${CLANG_FORMAT}"

    local FILES
    FILES=$(get_files)

    if [ -z "${FILES}" ]
    then
        echo "No matching files."
        return
    fi

    local FAILED=0

    while IFS= read -r FILE
    do
        [ -z "${FILE}" ] && continue

        echo "Checking ${FILE}"

        if ! "${CLANG_FORMAT}" \
            --style=file \
            --dry-run \
            --Werror \
            "${FILE}"
        then
            FAILED=1
        fi
    done <<< "${FILES}"

    [ "${FAILED}" -eq 0 ] || exit 1

    echo -e "${GREEN}PASS${NC}"
}

fix_format()
{
    print_header "CLANG-FORMAT FIX"

    check_tool "${CLANG_FORMAT}"

    local FILES
    FILES=$(get_files)

    if [ -z "${FILES}" ]
    then
        echo "No matching files."
        return
    fi

    while IFS= read -r FILE
    do
        [ -z "${FILE}" ] && continue

        echo "Formatting ${FILE}"

        "${CLANG_FORMAT}" \
            --style=file \
            -i \
            "${FILE}"
    done <<< "${FILES}"

    echo -e "${GREEN}DONE${NC}"
}

run_tidy()
{
    print_header "CLANG-TIDY"

    check_tool "${CLANG_TIDY}"
    check_tool cmake

    ensure_compile_database

    local FILES
    FILES=$(get_files)

    if [ -z "${FILES}" ]
    then
        echo "No matching files."
        return
    fi

    local FAILED=0

    while IFS= read -r FILE
    do
        [ -z "${FILE}" ] && continue

        echo "Analyzing ${FILE}"

        if ! "${CLANG_TIDY}" \
            "${FILE}" \
            -p build \
            --config-file=.clang-tidy \
            --header-filter='.*(src|include|tests)/.*' \
            --extra-arg=-Wno-unknown-warning-option
        then
            FAILED=1
        fi
    done <<< "${FILES}"

    [ "${FAILED}" -eq 0 ] || exit 1

    echo -e "${GREEN}PASS${NC}"
}

fix_tidy()
{
    print_header "CLANG-TIDY FIX"

    check_tool "${CLANG_TIDY}"
    check_tool cmake

    ensure_compile_database

    local FILES
    FILES=$(get_files)

    if [ -z "${FILES}" ]
    then
        echo "No matching files."
        return
    fi

    local FAILED=0

    while IFS= read -r FILE
    do
        [ -z "${FILE}" ] && continue

        echo "Fixing ${FILE}"

        if ! "${CLANG_TIDY}" \
            -p build \
            --config-file=.clang-tidy \
            --header-filter='.*(src|include|tests)/.*' \
            --fix \
            -fix-errors \
            --format-style=file \
            --extra-arg=-Wno-unknown-warning-option \
            "${FILE}"
        then
            FAILED=1
        fi
    done <<< "${FILES}"

    [ "${FAILED}" -eq 0 ] || exit 1

    echo -e "${GREEN}DONE${NC}"
}

run_cppcheck()
{
    print_header "CPPCHECK"

    check_tool cppcheck

    local FILES
    FILES=$(get_files)

    if [ -z "${FILES}" ]
    then
        echo "No matching files."
        return
    fi

    local JOBS
    JOBS=$(nproc 2>/dev/null || echo 4)

    mkdir -p .cppcheck-cache

    cppcheck \
        --enable=warning,style,performance,portability \
        --inconclusive \
        --force \
        --error-exitcode=1 \
        -j "${JOBS}" \
        --cppcheck-build-dir=.cppcheck-cache \
        --library=cppcheck.cfg \
        ${FILES}

    echo -e "${GREEN}PASS${NC}"
}

run_commit_check()
{
    print_header "COMMIT POLICY"

    local SUBJECT
    local BODY
    local FULL

    SUBJECT=$(git log -1 --pretty=%s)
    BODY=$(git log -1 --pretty=%b)
    FULL=$(git log -1 --pretty=%B)

    [ ${#SUBJECT} -gt 0 ] || {
        echo "Commit subject is empty."
        exit 1
    }

    [ ${#SUBJECT} -le 72 ] || {
        echo "Commit subject exceeds 72 characters."
        exit 1
    }

    echo "${FULL}" | grep -q '^Signed-off-by:' || {
        echo "Missing Signed-off-by."
        echo "Use: git commit -s"
        exit 1
    }

    while IFS= read -r LINE
    do
        [ -z "${LINE}" ] && continue

        [ ${#LINE} -le 72 ] || {
            echo "Commit body line exceeds 72 characters:"
            echo "${LINE}"
            exit 1
        }
    done <<< "${BODY}"

    echo -e "${GREEN}PASS${NC}"
}

configure_project()
{
    print_header "CONFIGURE"

    rm -rf build

    cmake \
        -B build \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

    echo -e "${GREEN}PASS${NC}"
}

run_local()
{
    FILE_MODE="local"

    run_format
    run_tidy
}

run_pipeline()
{
    FILE_MODE="pipeline"

    run_commit_check
    run_format
    run_tidy
}

run_all()
{
    FILE_MODE="pipeline"

    run_commit_check
    run_format
    run_tidy
    run_cppcheck
}

FILE_MODE="pipeline"

# Parse global --all flag: check all tracked source files instead of
# only pipeline-changed files.
if [ "${2:-}" = "--all" ]
then
    FILE_MODE="all"
fi

case "${1:-help}" in
    debug)
        show_pipeline_debug
        ;;
    local)
        run_local
        ;;
    pipeline)
        run_pipeline
        ;;
    all)
        run_all
        ;;
    format)
        FILE_MODE="pipeline"
        run_format
        ;;
    fix-format)
        FILE_MODE="pipeline"
        fix_format
        ;;
    tidy)
        FILE_MODE="pipeline"
        run_tidy
        ;;
    fix-tidy)
        FILE_MODE="pipeline"
        fix_tidy
        ;;
    cppcheck)
        FILE_MODE="pipeline"
        run_cppcheck
        ;;
    commit)
        run_commit_check
        ;;
    configure)
        configure_project
        ;;
    debug)
        echo_exclude_regex
        show_pipeline_debug
        ;;
    help|-h|--help)
        show_help
        ;;
    *)
        echo "Unknown command: $1"
        echo
        show_help
        exit 1
        ;;
esac