# Build and Install DLT on Cygwin
Back to [README.md](../README.md)

In this document you will be instructed to build and install DLT on Cygwin
for your own usage. Most of the time we receive some requests from users for
dlt on Cygwin, so we hope this doc would help to some extent.

*Note: We assume that you installed Cygwin in a proper way
following some docs on the Internet, and also configure your
cyg-get with correct permission.*

## Install dependencies
On Cygwin the dependencies can be installed with the following command:

```bash
cyg-get install git cmake zlib-devel cygport
```
## Build and install DLT
The normal build process would be almost the same as on Ubuntu:

```bash
mkdir build
cd build
cmake ..
make
make install

```
## Check for completion
The static and dynammic dlt libs will be installed at some specific
directories, for instance:
```bash
make install
...
-- Up-to-date: /usr/local/lib/static/libdlt.dll.a
-- Up-to-date: /usr/local/bin/cygdlt-2.dll
...
```
## Current issues
Some functions/API/ABI in DLT cannot be compiled and run properly.
Team is planning for the fix/modification.