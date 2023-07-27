# DLT Build Options

DLT is highly configurable. It allows you to choose between certain technologies
or implementations and to turn on or off certain features. This way, you can
adjust it to your needs and keep the build process as simple as possible.

In order to change these options, you can modify these values with cmake, do the
appropriate changes in CmakeList.txt or via the commandline for cmake

Change a value with: cmake -D\<Variable\>=\<Value\>, E.g.

```bash
cmake .. -DWITH_SYSTEMD=ON -DWITH_SYSTEMD_JOURNAL=ON -DCMAKE_INSTALL_PREFIX=/usr
```

## General Options

Option | Value | Comment
:--- | :--- | :---
BUILD\_SHARED\_LIBS | ON | Set to OFF to build static libraries
DLT\_IPC                          |"FIFO"          | Set to either "UNIX\_SOCKET" or "FIFO"
WITH\_DLT\_USE\_IPv6              | ON             | Set to ON for IPv6 support
WITH\_DLT\_EXAMPLES               | ON             | Set to ON to build src/examples binaries
DLT\_USER                         | covesa         | Set user for process not run as root
WITH\_CHECK\_CONFIG\_FILE         | OFF            | Set to ON to create a configure file of CheckIncludeFiles and CheckFunctionExists
CMAKE\_INSTALL\_PREFIX            | /usr/local
CMAKE\_BUILD\_TYPE                | RelWithDebInfo
WITH\_UDP\_CONNECTION             | OFF            | Set to ON to enable dlt UDP multicast SUPPORT
WITH\_DLT\_DAEMON\_VSOCK\_IPC     | OFF            | Set to ON for VSOCK support in daemon.
WITH\_DLT\_LIB\_VSOCK\_IPC        | OFF            | Set to ON for VSOCK support in libdlt (DLT\_IPC is overridden in libdlt).
DLT\_VSOCK\_PORT                  | 13490          | Port to use for VSOCK communication.
WITH\_LEGACY\_INCLUDE\_PATH       | ON             | Set to ON to add <prefix>/dlt to include paths for the CMake config file, in addition to only <prefix>
WITH\_DLT\_LOG\_LEVEL\_APP\_CONFIG | OFF           | Set to ON to enable default log levels based on application ids

## Command Line Tool Options

 Option | Value | Comment
 :--- | :--- | :---
WITH\_DLT\_ADAPTOR                | OFF            | Set to ON to build src/adaptor binaries
WITH\_DLT\_CONSOLE                | ON             | Set to ON to build src/console binaries
WITH\_DLT\_SYSTEM                 | OFF            | Set to ON to build src/system binaries
WITH\_DLT\_LOGSTORAGE\_CTRL\_UDEV | OFF            | PROTOTYPE! Set to ON to build
WITH\_DLT\_KPI                    | OFF            | Set to ON to build src/kpi binaries
WITH\_EXTENDED\_FILTERING         | OFF            | Set to OFF to build without extended filtering. Using json filter files is only supported for Linux based system with json-c and QNX.

## Linux OS Integration Options

 Option | Value | Comment
 :--- | :--- | :---
WITH\_SYSTEMD                     | OFF            | Set to ON to run CMakeLists.txt in systemd
WITH\_SYSTEMD\_WATCHDOG           | OFF            | Set to ON to use the systemd watchdog in dlt-daemon
WITH\_SYSTEMD\_WATCHDOG\_ENFORCE\_MSG\_RX | OFF    | Set to ON to notify the watchdog only if new messages where received in dlt-daemon since last notify
WITH\_SYSTEMD\_WATCHDOG\_ENFORCE\_MSG\_RX\_DLT\_SYSTEM | OFF    | Set to ON to notify the watchdog only if new messages where received in dlt-system since last notify
WITH\_SYSTEMD\_JOURNAL            | OFF            | Set to ON to use the systemd journal in dlt-system
WITH\_DLT\_DBUS                   | OFF            | Set to ON to build src/dbus binaries

## QNX OS Integration Options

Option | Value | Comment
:--- | :--- | :---
WITH\_DLT\_QNX\_SYSTEM            | OFF            | Set to ON to build QNX system binary dlt-qnx-system
DLT\_QNX\_SLOG\_ADAPTER\_WAIT\_BUFFER\_TIMEOUT\_MS | 100 | Maximum time in milliseconds to wait for buffer space in dlt before messages from the slog will be discarded.

## Documentation Options

Option | Value | Comment
 :--- | :--- | :---
WITH\_DOC                         | OFF            | Set to ON to build API documentation
WITH\_MAN                         | OFF            | Set to ON to build man pages

## Test Options

Option | Value | Comment
:--- | :--- | :---
WITH\_TESTSCRIPTS                 | OFF            | Set to ON to run CMakeLists.txt in test scripts
WITH\_DLT\_TESTS                  | ON             | Set to ON to build src/test binaries
WITH\_DLTTEST                     | OFF            | Set to ON to build with modifications to test User-Daemon communication with corrupt messages
WITH\_DLT\_UNIT\_TESTS            | OFF            | Set to ON to build unit test binaries
WITH\_GPROF                       | OFF            | Set \-pg to compile flag

## Experimental Features Options (Dragons ahead!)

Option | Value | Comment
:--- | :--- | :---
WITH\_DLT\_SHM\_ENABLE            | OFF            | Set to ON to enable shared memory as IPC
WITH\_DLT\_CXX11\_EXT             | OFF            | Set to ON to build C++11 extensions
WITH\_DLT\_COREDUMPHANDLER        | OFF            | Set to ON to build src/core\_dump\_handler binaries.
