# Build and Install DLT on MSYS2
Back to [README.md](../README.md)

In this document you will be instructed to build and install DLT on MSYS2
for your own usage. MSYS2 provides a Unix-like environment on Windows with
a native GCC toolchain, which is useful for building DLT without a full
Linux setup.

*Note: We assume that you installed MSYS2 in a proper way following the
instructions on the [MSYS2 website](https://www.msys2.org/), and that you
are using the **MSYS2 MSYS** shell (not the MinGW shells).*

## Install dependencies

Open the MSYS2 shell and install the required packages:

```bash
pacman -S --noconfirm git cmake ninja gcc gtest
```

## Build and install DLT

The build process is the same as on Linux, using CMake and Ninja:

```bash
git clone https://github.com/COVESA/dlt-daemon.git
cd dlt-daemon
mkdir build
cd build
cmake .. -G "Ninja"
ninja
ninja install
```

## Check for completion

The static and dynamic DLT libraries will be installed at the default
prefix, for instance:

```bash
-- Up-to-date: /usr/local/lib/libdlt.dll.a
-- Up-to-date: /usr/local/bin/msys-dlt-2.dll
```

## Continuous Integration

An MSYS2 build is tested automatically via GitHub Actions on every push
and pull request. See `.github/workflows/MSYS2.yaml` for details.
