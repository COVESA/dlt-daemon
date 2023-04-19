# DLT Offline Logstorage

Back to [README.md](../README.md)

## Introduction to DLT Offline Logstorage

Logstorage is a mechanism to store DLT logs on the target system or an external
device (e.g. USB stick) connected to the target. It can be seen as an
improvement of the Offline Trace functionality which is already part of DLT.

Logstorage provides the following features:
- Store logs in sets of log files defined by configuration files
- Log file content is configurable
    - Configurable options are:
        - Application identifier (single entry, list, wildcard)
        - Context identifier (single entry, list, wildcard)
        - Log level
        - ECU identifier
- Log files are configurable in terms of:
    - File name and naming scheme
    - File size
    - Number of files
- Log message synchronization strategy is configurable
- Trigger start and stop logging using a control application
- Integration into Udev device management


## Configuration

### General Configuration

General configuration is done inside dlt.conf. The following configuration
options exist:

```
##############################################################################
# Offline logstorage                                                         #
##############################################################################
# Store DLT log messages, if not set offline logstorage is off (Default: off)
# Maximum devices to be used as offline logstorage devices
# OfflineLogstorageMaxDevices = 1

# Path to store DLT offline log storage messages (Default: off)
# OfflineLogstorageDirPath = /opt

# File options
# Appends timestamp in log file name, Disable by setting to 0 (Default: 1)
# OfflineLogstorageTimestamp = 0

# Appends delimiter in log file name, allowed punctutations only (Default: _)
# OfflineLogstorageDelimiter = _

# Wrap around value for log file count in file name (Default: UINT_MAX)
# OfflineLogstorageMaxCounter = 999

# Maximal used memory for Logstorage Cache in KB (Default: 30000 KB)
# OfflineLogstorageCacheSize = 30000
```

### Configuration file format

For DLT daemon to store logs the configuration file named “dlt\_logstorage.conf”
should be present in external storage or internal storage device (= given path
in the file system).

```
[Filter<unique number>]              # filter configration name
LogAppName=<APID>                    # Name of application to store logs from. Multiple applications can be separated by "," and ".*" denotes all applications
ContextName=<CTID>                   # Name or names of contexts to store logs from. Multiple contexts can be separated by "," and ".*" denotes all contexts of the application
LogLevel=<Log level>                 # Define log level, e.g. DLT_LOG_INFO or DLT_LOG_FATAL
File=<file name>                     # Base name of the created files that containing the logs, e.g. "example". For further file naming scheme configurations see man dlt.conf
FileSize=<file size in bytes>        # Maximum file size in bytes
NOFiles=<number of files>            # Number of created files before oldest is deleted and a new one is created
SyncBehavior=<strategy>              # Specify sync strategy. Default: Sync'ed after every message. See Logstorage Rinbuffer Implementation below.
EcuID=<ECUid>                        # Specify ECU identifier
SpecificSize=<spec size in bytes>    # Store logs in storage devices after specific size is reached.
GzipCompression=<on or off>          # Write the logfiles with gzip compression.
```

The Parameter "SyncBehavior","EcuID" and "SpecificSize" are optional - all
others are mandatory.

If both of the parameter "LogAppName" and "ContextName" are set to wildcard or
not present in the configuration file, "EcuID" must be specified.

A typical configuration file may look like:

```
[FILTER1]
LogAppName=APP1
ContextName=CON1,CON2
LogLevel=DLT_LOG_INFO
File=App
FileSize=10000
NOFiles=10

[FILTER2]
LogAppName=TEST
ContextName=.*
LogLevel=DLT_LOG_ERROR
File=Test
FileSize=250000
NOFiles=5
EcuID=ECU1
SyncBehavior=ON_SPECIFIC_SIZE
SpecificSize=5000
GzipCompression=on

[FILTER3]
LogAppName=TEST
ContextName=.*
LogLevel=DLT_LOG_ERROR
File=Test
FileSize=250000
NOFiles=5
SyncBehavior=ON_FILE_SIZE,ON_DEMAND
EcuID=ECU1
```

In case of Non-Verbose mode, following filters should be used.

```
[NON-VERBOSE-STORAGE-FILTER<unique number>]    # filter configuration name for a Non-Verbose passive node
EcuID=<ECUid>                                  # Specify ECU identifier
File=<file name>                               # Base name of the created files that containing the logs, e.g. "example". For further file naming scheme configurations see man dlt.conf
FileSize=<file size in bytes>                  # Maximum file size in bytes
NOFiles=<number of files>                      # Number of created files before oldest is deleted and a new one is created
GzipCompression=on                             # Compress the log files

[NON-VERBOSE-LOGLEVEL-CTRL<unique number>]     # filter configuration name to control log level of Non-Verbose applications
LogAppName=<APID>                              # Name of application (wildcard allowed)
ContextName=<CTID>                             # Name of context (wildcard allowed)
LogLevel=<Log level>                           # Define log level, e.g. DLT_LOG_INFO or DLT_LOG_FATAL
EcuID=<ECUid>                                  # Specify ECU identifier
```

A typical configuration file may look like:

```
[NON-VERBOSE-STORAGE-FILTER1]
EcuID=PASV
File=scc
FileSize=50000
NOFiles=5

[NON-VERBOSE-LOGLEVEL-CTRL1]
LogAppName=LOG
ContextName=TEST
LogLevel=DLT_LOG_DEBUG
EcuID=PASV

[NON-VERBOSE-LOGLEVEL-CTRL2]
LogAppName=.*
ContextName=.*
LogLevel=DLT_LOG_WARN
EcuID=PASV
```


## Usage DLT Offline Logstorage

Enable OfflineLogstorage by setting ```OfflineLogstorageMaxDevices = 1``` in
dlt.conf. Be aware that the performance of DLT may drop if multiple Logstorage
devices are used; the performance depends on the write speed of the used device,
too.

Create the device folder:

```mkdir -p /var/dltlogs```

Create a configuration file and store it on into that folder or mount an
external device containing a configuration file.

Start the DLT Daemon. This is not necessary if the DLT Daemon was started
already with Offline Logstorage enabled.

Trigger DLT Daemon to use the new logstorage device:

```dlt-logstorage-ctrl -c 1 -p /var/dltlogs```

Afterwards, logs that match the filter configuration are stored onto the
Logstorage device.

```dlt-logstorage-ctrl -c 0 -p /var/dltlogs```

The configured logstorage device is disconnected from the DLT Daemon.


### Using dlt-logstorage-ctrl application

```
Usage: dlt-logstorage-ctrl [options]
Send a trigger to DLT daemon to connect/disconnect a certain logstorage device

Options:
  -c         Connection type: connect = 1, disconnect = 0
  -d[prop]   Run as daemon: prop = use proprietary handler
             'prop' may be replaced by any meaningful word
  -e         Set ECU ID (Default: ECU1)
  -h         Usage
  -p         Mount point path
  -s         Sync Logstorage cache
  -t         Specify connection timeout (Default: 10s)
  -S         Send message with serial header (Default: Without serial header)
  -R         Enable resync serial header
  -v         Set verbose flag (Default:0)
```

## Testing DLT Offline Logstorage

The following procedure can be used to test Offline Logstorage:

- Enable OfflineLogstorage by setting OfflineLogstorageMaxDevices = 1 in dlt.conf
- Start dlt-daemon
- The default search path of logstorage is: /tmp/dltlogs/dltlogsdevX
  where X is a number in the range [1..OfflineLogstorageMaxDevices]

- Create the device folder

  ```$ mkdir -p /var/dltlog```

- Create the configuration file "dlt\_logstorage.conf" in this folder
  and define filter configuration(s):

  ```
  [FILTER1]
  LogAppName=LOG
  ContextName=TEST
  LogLevel=DLT_LOG_WARN
  File=example
  FileSize=50000
  NOFiles=5
  ```

- Trigger dlt-daemon to use a new device

  ```$ dlt-logstorage-ctrl -c 1 -p /var/dltlog```

- Start dlt-example-user

  ```$ dlt-example-user Hello123```

- After execution, a log file is created in /var/dltlogs
  e.g. example\_001\_20150512\_133344.dlt

- To check the content of the file open it with dlt-convert or DLT Viewer.

## Logstorage Ring Buffer Implementation

The DLT Logstorage is mainly used to store a configurable set of logs on an
external mass storage device attached to the target. In this scenario, writing
each incoming log message directly onto the external storage device is
appreciate, because the storage device might be un-mounted/suddenly removed at
any time. Writing each log message immediately avoids the problem of losing too
many messages because the file system sync could not be finished before the
device has been removed physically from the target. On the other hand the DLT
Logstorage feature might be used as well to store a configurable set of logs on
any internal, nonvolatile memory (e.g. FLASH storage device). Due to the reason
of limited write cycles of a FLASH device the number of write cycles has to be
reduced as much as possible. But the drawback of losing log messages in case of
an unexpected operating system crash has to be taking into account as well. The
obvious idea is to cache incoming log messages in memory and write the log data
to disk based on a certain strategy. Incoming log messages are stored in a data
cache with a specific size. Depending on user defined strategy, the data cache
is written onto the storage device、without relying on the sync mechanism of the
file system.

The following strategies are implemented:
- ON\_MSG - sync every message(Default)
- ON\_DAEMON\_EXIT - sync on daemon exit
- ON\_DEMAND - sync on demand
- ON\_FILE\_SIZE - sync on file size reached
- ON\_SPECIFIC\_SIZE - sync after specific size is reached

Note :
1. Combinations (not allowed: combinations with ON_MSG,combination of ON\_FILE\_SIZE with ON\_SPECIFIC\_SIZE)
2. If on\_demand sync strategy alone is specified, it is advised to concatenate the log files in sequential order before viewing it on viewer.
3. In case multiple FILTERs use the same `File` value, it is recommened that the following settings must also have same values: `NOFiles`, `FileSize` and `SpecificSize`

## Maintain Logstorage Log Level Implementation

The log level setting of each user context in the logstorage FILTER will be
treated as the highest priority. Other clients (e.g: dlt-control, dlt-viewer)
can update the user context's log level to a lower level but can not update
to a higher level. In case the clients need to update the user context's log
level to a higher level, the new macro ```MaintainLogstorageLogLevel``` is
implemented in the ```[General]``` session to allow changing user context's
log level to any level or maintain the log level of logstorage configuration.

A typical configuration file may look like:

  ```
  [General]
  MaintainLogstorageLogLevel=OFF
  ```

By setting ```MaintainLogstorageLogLevel=OFF``` or ```MaintainLogstorageLogLevel=0```,
the clients are able to update any log level to user contexts.

By setting ```MaintainLogstorageLogLevel=ON``` or ```MaintainLogstorageLogLevel=1```
or not set, the logstorage will maintain its log level as the highest priority.
