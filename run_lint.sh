#!/bin/bash
#
# SPDX-License-Identifier: MPL-2.0
#
# run_lint.sh - clang-format helper for dlt-daemon
#
# Usage:
#   ./run_lint.sh              - install pre-commit hook and fix formatting (default)
#   ./run_lint.sh check        - check formatting (dry-run, fails on violations)
#   ./run_lint.sh fix          - fix formatting in-place
#   ./run_lint.sh install-hook - configure git to use .githooks/pre-commit
#
# In CI, set CHECK_ALL=1 to check all tracked files:
#   CHECK_ALL=1 ./run_lint.sh check
#
# clang-tidy support can be added later.
#

set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

# clang-format binary: prefer version-pinned clang-format-14, then newer versions
CLANG_FORMAT="${CLANG_FORMAT:-}"
if [ -z "${CLANG_FORMAT}" ]; then
    for cf in clang-format-14 clang-format-16 clang-format-17 clang-format-18 clang-format; do
        if command -v "$cf" >/dev/null 2>&1; then
            CLANG_FORMAT="$(command -v "$cf")"
            break
        fi
    done
fi

# File patterns to check
FILE_PATTERN='^(src|include|tests)/.*\.(c|cc|cpp|cxx|h|hh|hpp|hxx)$'

# Directories excluded from formatting checks
EXCLUDE_PATTERNS=(
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
)
EXCLUDE_REGEX="$(IFS='|'; echo "${EXCLUDE_PATTERNS[*]}")"

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

print_header()
{
    echo
    echo "=================================================="
    echo "$1"
    echo "=================================================="
}

get_files()
{
    # If files are passed as arguments, use them
    if [ "$#" -gt 0 ]; then
        printf '%s\n' "$@"
    elif [ "${CHECK_ALL:-0}" = "1" ]; then
        # CI mode: check all tracked files
        git ls-files
    else
        # Default: collect staged files (for pre-commit hook)
        git diff --cached --name-only --diff-filter=ACM
    fi | grep -E "${FILE_PATTERN}" \
      | grep -Ev "${EXCLUDE_REGEX}" \
      || true
}

# ---------------------------------------------------------------------------
# clang-format
# ---------------------------------------------------------------------------

run_clang_format()
{
    local mode="$1"  # "check" or "fix"
    shift

    if [ -z "${CLANG_FORMAT}" ]; then
        echo -e "${YELLOW}WARNING: clang-format not found, skipping format check${NC}"
        echo "Install with: sudo apt-get install clang-format-14"
        return 0
    fi

    print_header "CLANG-FORMAT (${mode})"

    local files
    files=$(get_files "$@")

    if [ -z "${files}" ]; then
        echo "No C/C++ files to check."
        return 0
    fi

    local failed=0

    if [ "${mode}" = "fix" ]; then
        while IFS= read -r file; do
            [ -z "${file}" ] && continue
            echo "Formatting ${file}"
            "${CLANG_FORMAT}" --style=file -i "${file}"
        done <<< "${files}"
        echo -e "${GREEN}Formatting complete${NC}"
    else
        while IFS= read -r file; do
            [ -z "${file}" ] && continue
            if ! "${CLANG_FORMAT}" --style=file --dry-run --Werror "${file}" 2>/dev/null; then
                echo "  FAIL: ${file}"
                failed=1
            fi
        done <<< "${files}"

        if [ "${failed}" -ne 0 ]; then
            echo ""
            echo -e "${RED}FAIL: One or more files are not formatted${NC}"
            echo "Fix with:"
            echo "  ./run_lint.sh fix"
            echo "Or for specific files:"
            echo "  clang-format-14 -i <file>"
            return 1
        fi

        echo -e "${GREEN}PASS${NC}"
    fi
}

# ---------------------------------------------------------------------------
# Install pre-commit hook
# ---------------------------------------------------------------------------

install_hook()
{
    local repo_root
    repo_root=$(git rev-parse --show-toplevel)
    local hooks_dir="${repo_root}/.githooks"

    if [ ! -d "${hooks_dir}" ]; then
        echo -e "${RED}ERROR: ${hooks_dir} does not exist${NC}"
        return 1
    fi

    git config core.hooksPath .githooks
    echo -e "${GREEN}Pre-commit hook installed${NC}"
    echo "  git config core.hooksPath .githooks"
    echo ""
    echo "To bypass (emergency only):"
    echo "  git commit --no-verify"
    echo "To uninstall:"
    echo "  git config --unset core.hooksPath"
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

main()
{
    local mode="auto"
    local files=()

    # Parse arguments: first arg is a subcommand if it matches one,
    # otherwise treat all args as file paths (auto mode)
    if [ "$#" -gt 0 ]; then
        case "$1" in
            auto|check|fix|install-hook)
                mode="$1"
                shift
                files=("$@")
                ;;
            *)
                files=("$@")
                ;;
        esac
    fi

    case "${mode}" in
        auto)
            # Default: install hook, then fix formatting
            install_hook
            run_clang_format "fix" "${files[@]+"${files[@]}"}"
            ;;
        check)
            run_clang_format "check" "${files[@]+"${files[@]}"}"
            ;;
        fix)
            run_clang_format "fix" "${files[@]+"${files[@]}"}"
            ;;
        install-hook)
            install_hook
            ;;
        *)
            echo "Usage: $0 [auto|check|fix|install-hook] [file...]"
            echo "  auto          - install hook and fix formatting (default)"
            echo "  check         - check formatting (dry-run, fails on violations)"
            echo "  fix           - fix formatting in-place"
            echo "  install-hook  - configure git to use .githooks/pre-commit"
            exit 1
            ;;
    esac
}

main "$@"
