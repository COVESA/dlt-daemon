# Homebrew Support

Homebrew tap for the COVESA DLT daemon.

The formula lives at [`Formula/dlt-daemon.rb`](../Formula/dlt-daemon.rb) in the repository root, which is where Homebrew expects it when using this repo as a tap.

## Install

```sh
brew tap COVESA/dlt-daemon https://github.com/COVESA/dlt-daemon
brew install COVESA/dlt-daemon/dlt-daemon
```

## What gets installed

The formula installs:

### Core components

* `bin/dlt-daemon` — DLT daemon binary
* `lib/libdlt.dylib` — DLT user library
* `include/dlt/` — public headers
* `lib/cmake/automotive-dlt/` — CMake package configuration for `find_package()`
* `lib/pkgconfig/automotive-dlt.pc` — pkg-config file

### Console tools

Including common utilities such as:

* `dlt-receive`
* `dlt-adaptor-stdin`
* `dlt-convert`
* `dlt-control`
* `dlt-filetransfer`
* and other DLT command-line tools built by the project

