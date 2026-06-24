# Contributing to dlt-daemon

Thank you for your interest in contributing to the COVESA dlt-daemon project!
This document describes the workflow, coding standards, and tooling used in this
repository.

## Quick Start

### 1. Enable Git Hooks (Recommended)

The repository ships local git hooks that mirror the CI pipeline. Enable them
once after cloning:

```bash
git config core.hooksPath .githooks
```

This activates three hooks:

| Hook           | When it runs      | What it checks                                      |
|----------------|-------------------|-----------------------------------------------------|
| `pre-commit`   | Before a commit   | `clang-format` dry-run, SPDX license headers        |
| `commit-msg`   | On commit message | Subject rules, imperative verb, `Signed-off-by`, line length |
| `pre-push`     | Before a push     | Commit message validation for all pushed commits    |

To bypass hooks in an emergency:

```bash
git commit --no-verify
git push --no-verify
```

### 2. Local CI Helper

The `check.sh` script at the repository root provides the same checks as CI,
plus additional tools:

```bash
./check.sh local       # Check modified/staged files (format + tidy)
./check.sh pipeline    # Check commits ahead of origin/master
./check.sh all          # Pipeline checks + cppcheck
./check.sh format       # clang-format dry-run on pipeline files
./check.sh fix-format   # Apply clang-format fixes
./check.sh tidy         # clang-tidy on pipeline files
./check.sh fix-tidy     # Apply clang-tidy fixes
./check.sh cppcheck     # Run cppcheck
./check.sh commit       # Validate latest commit message
./check.sh configure    # Regenerate build directory + compile_commands.json
./check.sh debug        # Show which files/commits are in scope
```

## Commit Message Policy

All commits must follow these rules (enforced by CI and local hooks):

1. **Subject line** must:
   - Be non-empty and at most **72 characters**
   - **Not** end with a period
   - Start with an **imperative verb**:
     `Add`, `Fix`, `Remove`, `Update`, `Refactor`, `Implement`, `Improve`,
     `Introduce`, `Rename`, `Replace`, `Cleanup`, `Document`, `Convert`,
     `Move`, `Enable`, `Disable`
   - Not be a `fixup!` or `squash!` commit

2. **Body** must:
   - Be separated from the subject by a **blank line**
   - Have all lines at most **72 characters**

3. **Signed-off-by** (DCO):
   - Every commit must include a `Signed-off-by:` line
   - Use `git commit -s` to add it automatically

4. **No merge commits** in PRs (rebase instead)

### Example

```
Fix memory leak in dlt_buffer_push

The push function did not free the temporary buffer on error paths,
causing a leak when the ring buffer was full. Free the buffer in all
error branches.

Signed-off-by: Jane Doe <jane.doe@example.com>
```

## Coding Standards

### Docker (recommended)

The easiest way to ensure consistent code-quality checks is to use the provided
Docker container, which pins **clang-format 10**, **clang-tidy 10**, and
**cppcheck** on Ubuntu 20.04:

```bash
# Build the container (first time only)
docker build -t dlt-daemon-ci docker/

# Or use the helper script (builds automatically):
./docker/run.sh format       # check clang-format
./docker/run.sh fix-format   # auto-fix formatting
./docker/run.sh tidy         # run clang-tidy
./docker/run.sh fix-tidy     # auto-fix clang-tidy issues
./docker/run.sh cppcheck     # run cppcheck
./docker/run.sh all          # format + tidy + cppcheck
./docker/run.sh shell        # interactive shell in container
```

### clang-format

All C/C++ source files must conform to the `.clang-format` configuration at the
repository root (LLVM-based, 80-column limit, 4-space indentation).

Check formatting:

```bash
./check.sh format
# or via Docker:
./docker/run.sh format
```

Auto-fix:

```bash
./check.sh fix-format
# or via Docker:
./docker/run.sh fix-format
```

> **Note:** The project uses **clang-format 16** and **clang-tidy 16**
> (via the Docker container `docker/Dockerfile` on Ubuntu 22.04 with LLVM 16).
> The CI pipelines use the same container to ensure consistent results.
> `check.sh` and the git hooks automatically prefer the `-16` suffixed binaries
> when available, falling back to `-18` or unversioned names.

### clang-tidy

Static analysis is performed with `clang-tidy` using `.clang-tidy` at the
repository root. The configuration enables `bugprone-*`, `clang-analyzer-*`,
`performance-*`, `portability-*`, and `misc-*` checks.

```bash
./check.sh tidy       # check
./check.sh fix-tidy   # auto-fix
```

A compile database is required. Generate it with:

```bash
./check.sh configure
# or:
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

#### Safe library wrappers

`clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling` flags
standard C functions (`snprintf`, `memcpy`, `memset`, `strncpy`, etc.) and
demands C11 Annex K `_s` variants that are not available in glibc (Linux) or
QNX's libc.  Instead, `include/dlt/dlt_safe_lib.h` redirects these functions to
`__builtin_*` variants (which clang-tidy does not flag) via macros.  This header
is included in all source files that use these functions.  In addition,
`_FORTIFY_SOURCE=2` (enabled in `CMakeLists.txt` for optimized builds on GCC and
Clang) provides compile-time and run-time buffer overflow checking.

### cppcheck

```bash
./check.sh cppcheck
```

### SPDX License Headers

All C/C++ files in `src/`, `include/`, and `tests/` must contain an
SPDX license identifier:

```c
/* SPDX-License-Identifier: MPL-2.0 */
```

## CI Pipeline

The following GitHub Actions workflows run on pull requests:

| Workflow             | Description                                    |
|----------------------|------------------------------------------------|
| `clang-format`       | Checks formatting of changed C/C++ files       |
| `static-analysis`    | Runs `clang-tidy` and `cppcheck`               |
| `commit-check`       | Validates commit messages                      |
| `spdx-check`         | Checks for SPDX license headers                |
| `cmake-ctest`        | Builds and runs the test suite                 |
| `macos-build`        | Verifies the build on macOS                    |
| `codeql-analysis`    | GitHub CodeQL security analysis                |

## Excluded Directories

The following directories are excluded from format/tidy/cppcheck checks (they
contain platform-specific, third-party, or test-only code):

- `include/` (legacy headers - being migrated)
- `src/android/`
- `src/dlt-qnx-system/`
- `src/core_dump_handler/`
- `src/dbus/`
- `src/kpi/`
- `src/tests/`
- `qnx/`
- `googletest/`
- `doc/`
- `build/`

## Build

```bash
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
ctest --test-dir build
```

## License

This project is licensed under the MPL-2.0 license. See `LICENSE` for details.
