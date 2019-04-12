# Diagnostic Log and Trace

Build and Test status: [![build and test status](https://travis-ci.org/GENIVI/dlt-daemon.svg?branch=master)](https://travis-ci.org/GENIVI/dlt-daemon)

## Overview

This component provides a log and trace interface, based on the
standardised protocol specified in the AUTOSAR standard 4.0 DLT.
This software can be used by GENIVI components and other applications as logging framework.

DLT basically consists of 3 components:

- **DLT Library**: Enables DLT logging for DLT user applications and temporary storage of log messages if daemon isn't available.
- **DLT Daemon**: Receiving log messages from DLT user applications and temporary storage of log messages if client isn't available. Transmit log messages to DLT Client and response to control messages.
- **DLT Client**: Receiving and storage of log messages from DLT Daemon into one single trace file and sending control message

![alt text](doc/images/dlt_overview.png "DLT Overview")

Furthermore, the repository contains several adaptors, console utilities as well as test applications.

## Build and install

The following packages need to be installed in order to be able to build and install DLT daemon:

- cmake
- zlib
- dbus

On Ubuntu those dependencies can be installed with the following command:

`sudo apt-get install cmake zlib1g-dev libdbus-glib-1-dev`

To build and install the DLT daemon, follow these steps:

```bash
mkdir build
cd build
cmake ..
make
optional: sudo make install
optional: sudo ldconfig`
```

### Configuration

#### General Options

Option | Value | Comment
:--- | :--- | :---
BUILD_SHARED_LIBS | ON | Set to OFF to build static libraries
DLT_IPC                       |"FIFO"          | Set to either "UNIX_SOCKET" or "FIFO"
WITH_DLT_USE_IPv6             | ON             | Set to ON for IPv6 support
WITH_DLT_EXAMPLES             | ON             | Set to ON to build src/examples binaries
DLT_USER                      | genivi         | Set user for process not run as root
WITH_CHECK_CONFIG_FILE        | OFF            | Set to ON to create a configure file of CheckIncludeFiles and CheckFunctionExists
CMAKE_INSTALL_PREFIX          | /usr/local
CMAKE_BUILD_TYPE              | RelWithDebInfo

#### Command Line Tool Options

 Option | Value | Comment
 :--- | :--- | :---
WITH_DLT_ADAPTOR              | OFF             | Set to ON to build src/adaptor binaries
WITH_DLT_CONSOLE              | ON             | Set to ON to build src/console binaries
WITH_DLT_SYSTEM               | OFF             | Set to ON to build src/system binaries
WITH_DLT_LOGSTORAGE_CTRL_UDEV | OFF            | PROTOTYPE! Set to ON to build
WITH_DLT_LOGSTORAGE_CTRL_PROP | OFF            | PROTOTYPE! Set to ON to build logstorage control application with proprietary support
WITH_DLT_KPI                  | OFF            | Set to ON to build src/kpi binaries

#### Linux OS Integration Options

 Option | Value | Comment
 :--- | :--- | :---
WITH_SYSTEMD                  | OFF            | Set to ON to run CMakeLists.txt in systemd
WITH_SYSTEMD_WATCHDOG         | OFF            | Set to ON to use the systemd watchdog in dlt-daemon
WITH_SYSTEMD_JOURNAL          | OFF            | Set to ON to use the systemd journal in dlt-system
WITH_DLT_DBUS                 | OFF             | Set to ON to build src/dbus binaries

#### Documentation Options

Option | Value | Comment
 :--- | :--- | :---
WITH_DOC                      | OFF            | Set to ON to build documentation target
WITH_MAN                      | OFF             | Set to OFF to skip building of man pages

#### Test Options

Option | Value | Comment
:--- | :--- | :---
WITH_TESTSCRIPTS              | OFF            | Set to ON to run CMakeLists.txt in test scripts
WITH_DLT_TESTS                | ON             | Set to ON to build src/test binaries
WITH_DLTTEST                  | OFF            | Set to ON to build with modifications to test User-Daemon communication with corrupt messages
WITH_DLT_UNIT_TESTS           | OFF             | Set to ON to build unit test binaries
WITH_GPROF                    | OFF            | Set \-pg to compile flag

#### Experimental Features Options

Option | Value | Comment
:--- | :--- | :---
WITH_DLT_SHM_ENABLE           | OFF            | Set to OFF to use FIFO as IPC from user to daemon
WITH_DLT_CXX11_EXT            | OFF            | Set to ON to build C++11 extensions
WITH_DLT_COREDUMPHANDLER      | OFF            | EXPERIMENTAL! Set to ON to build src/core_dump_handler binaries. EXPERIMENTAL

In order to change these options, you can modify these values
with cmake, do the appropriate changes in CmakeList.txt or via
the commandline for cmake

Change a value with: cmake -D\<Variable\>=\<Value\>, E.g.

```bash
cmake .. -DWITH_SYSTEMD=ON -DWITH_SYSTEMD_JOURNAL=ON -DCMAKE_INSTALL_PREFIX=/usr
```

## Documentation

Specific documentation can be found in the following files:

- [ReleaseNotes](ReleaseNotes.md)
- [Glossary](doc/dlt_glossary.md)
- [For Developers](doc/dlt_for_developers.md)
- [Logstorage](doc/dlt_offline_logstorage.md)
- [MultiNode](doc/dlt_multinode.md)
- [Extended Network Trace](doc/dlt_extended_network_trace.md)
- [DLT Filetransfer](doc/dlt_filetransfer.md)
- [DLT KPI](doc/dlt_kpi.md)
- [DLT Core Dump Handler](/doc/dlt_cdh.md)

All text based documentation will be replaced with by Markdown-based documentation for convinient access.

Old documentation (not maintained - will be removed in future releases):

- DLT User Manual: doc/dlt\_user\_manual.txt
- DLT Cheatsheet: doc/dlt_cheatsheet.txt
- DLT Design Specification: doc/dlt\_design\_specification.txt
- DLT Compilation of all documentation: doc/dlt_book.txt

### API Documentation

The API documentation is generated with _doxygen_.

```bash
mkdir build
cd build
cmake -DWITH_DOC=ON ..
make doc
```

### Manpages

- [dlt-daemon(1)](doc/dlt-daemon.1.md)
- [dlt.conf(5)](doc/dlt.conf.5.md)
- [dlt-system(1)](doc/dlt-system.1.md)
- [dlt-system.conf(5)](doc/dlt-system.conf.5.md)
- [dlt-convert(1)](doc/dlt-convert.1.md)
- [dlt-sortbytimestamp(1)](doc/dlt-sortbytimestamp.1.md)
- [dlt-receive(1)](doc/dlt-receive.1.md)
- [dlt-logstorage-ctrl(1)](doc/dlt-logstorage-ctrl.1.md)
- [dlt-passive-node-ctrl(1)](doc/dlt-passive-node-ctrl.1.md)

The man pages are generated with *pandoc*.

If the man pages are changed the following command must be executed.

```bash
mkdir build
cd build
cmake -DWITH_MAN=ON ..
make generate_man
```

The generated man pages overwrite the existing ones.

## Contribution

Start working, best practice is to commit smaller, compilable pieces during the work that makes it easier to handle later on.

If you want to commit your changes, create a [Pull Request](https://github.com/genivi/dlt-daemon/pulls) in Github.

### Coding Rules

Before contributing code, run uncrustify to harmonize code style.

Configuration: util/uncrustify.cfg
uncrustify version: 0.68_f

## Known issues

List of open issues can be found on [Github](https://github.com/GENIVI/dlt-daemon/issues)

- DLT library: Usage of dlt\_user\_log\_write\_float64() and DLT_FLOAT64() leads to "Illegal instruction (core dumped)" on ARM target.
- DLT library: Nested calls to DLT\_LOG\_ ... are not supported, and will lead to a deadlock.
- For Non linux platforms [eg: QNX] IPC supported is UNIX_SOCKET. For Linux Platforms both IPC FIFO and UNIX_SOCKET are supported

## Software/Hardware

Developed and tested with Ubuntu Linux 16 64-bit / Intel PC

## License

Full information on the license for this software is available in the "LICENSE" file.
Full information on the license for the cityhash code is available in "COPYING" file in src/core\_dump\_handler/cityhash_c.

## Mailinglist

https://lists.genivi.org/mailman/listinfo/genivi-diagnostic-log-and-trace_lists.genivi.org

## Contact

Christoph Lipka <clipka@de.adit-jv.com>,
Manikandan Chockalingam <Manikandan.Chockalingam@in.bosch.com>

![alt text](doc/images/genivilogo.png "GENIVI")
