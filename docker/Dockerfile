# SPDX-License-Identifier: MPL-2.0
#
# dlt-daemon code-quality container
#
# Usage:
#   docker build -t dlt-daemon-ci docker/
#   docker run --rm -v "$PWD:/workspace" -w /workspace dlt-daemon-ci ./check.sh format
#   docker run --rm -v "$PWD:/workspace" -w /workspace dlt-daemon-ci ./check.sh tidy
#   docker run --rm -v "$PWD:/workspace" -w /workspace dlt-daemon-ci ./check.sh fix-format
#   docker run --rm -v "$PWD:/workspace" -w /workspace dlt-daemon-ci ./check.sh fix-tidy
#
# Or use the helper script:
#   ./docker/run.sh format
#   ./docker/run.sh tidy
#   ./docker/run.sh fix-format
#   ./docker/run.sh fix-tidy

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install build tools, cppcheck, and ca-certificates from Ubuntu 22.04 repos
# (cmake >= 3.22 on Jammy, satisfies the >= 3.21 requirement)
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    cmake \
    cppcheck \
    git \
    pkg-config \
    libudev-dev \
    libsystemd-dev \
    zlib1g-dev \
    nlohmann-json3-dev \
    libjsoncpp-dev \
    libgtest-dev \
    gdb \
    lcov \
    netcat-openbsd \
    strace \
    valgrind \
    && rm -rf /var/lib/apt/lists/*

# Install clang-format-16 and clang-tidy-16 from the LLVM apt repository.
# The project is formatted and analysed with clang-16 for consistency.
# Uses [trusted=yes] to avoid GPG key download issues behind proxies.
RUN echo "deb [trusted=yes] http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main" \
      > /etc/apt/sources.list.d/llvm-16.list \
    && apt-get update \
    && apt-get install -y --no-install-recommends \
       clang-format-16 \
       clang-tidy-16 \
    && rm -rf /var/lib/apt/lists/*

# Create symlinks so unversioned names resolve to -16
RUN ln -sf /usr/bin/clang-format-16 /usr/local/bin/clang-format \
    && ln -sf /usr/bin/clang-tidy-16 /usr/local/bin/clang-tidy

# Create a writable home directory for arbitrary UIDs (when run with --user)
RUN mkdir -p /home/user && chmod 777 /home/user
ENV HOME=/home/user

WORKDIR /workspace
