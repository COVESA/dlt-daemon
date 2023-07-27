% DLT.CONF(5)

# NAME

**dlt.conf** - DLT daemon configuration file

# DESCRIPTION

The DLT daemon is the central application which gathers logs and traces from different applications, stores them temporarily or permanently and transfers them to a DLT client application, which could run directly on the COVESA system or more likely on some external tester device.

The configuration file dlt.conf allows to configure the different
runtime behaviour of the dlt-daemon. It is loaded during startup of dlt-daemon.

# GENERAL OPTIONS

## Verbose

Start daemon in debug mode, so that all internal debug information is printed out on the console.

    Default: Off

## Daemonize

If set to 1 DLT daemon is started in background as daemon. This option is only needed in System V init systems. In systemd based startup systems the daemon is started by spawning own process.

    Default: 0

## SendSerialHeader

If set to 1 DLT daemon sends each DLT message to the client with prepanding the serial header "DLS0x01".

    Default: 0

## SendContextRegistration

If set to 1 each context which is registered from an application in the DLT daemon generates a message to inform the DLT client about the new context.

    Default: 1

## SendMessageTime

If set to 1 DLt daemon sends each second a DLT control message to the client with the current timestamp from the system.

    Default: 0

## ECUId

This value sets the ECU Id, which is sent with each DLT message.

    Default: ECU1

## SharedMemorySize

This value sets the size of the shared memory, which is used to exchange DLT messages between applications and daemon. This value is defined in bytes. If this value is changed the system must be rebooted to take effect.

    Default: 100000

## PersistanceStoragePath

This is the directory path, where the DLT daemon stores its runtime configuration. Runtime configuration includes stored log levels, trace status and changed logging mode.

    Default: /tmp

## LoggingMode

The logging console for internal logging of dlt-daemon. 0 = log to stdout, 1 = log to syslog, 2 = log to file (see LoggingFilename), 3 = log to stderr

    Default: 0

## LoggingLevel

The internal log level, up to which logs are written. LOG_EMERG = 0, LOG_ALERT = 1, LOG_CRIT = 2, LOG_ERR = 3, LOG_WARNING = 4, LOG_NOTICE = 5, LOG_INFO = 6, LOG_DEBUG = 7

    Default: 6

## LoggingFilename

If LoggingMode is set to 2 logs are written to the file path given here.

    Default: /tmp/dlt.log

## EnableLoggingFileLimit

Only relevant for logging in file (LoggingMode = 2).
If EnableLoggingFileLimit is set to 0, the daemon logs to one logging file without any size limit.
If EnableLoggingFileLimit is set to 1, the daemon considers the size limits configured by LoggingFileSize and LoggingFileMaxSize. If the limits are configured accordingly, multiple log files are used.

    Default: 0

## LoggingFileSize

Only considered for logging in file (LoggingMode = 2) and EnableLoggingFileLimit = 1. Maximum size in bytes of one logging file.
    
    Default: 250000

## LoggingFileMaxSize

Only considered for logging in file (LoggingMode = 2) and EnableLoggingFileLimit = 1. Maximum size in bytes of all logging files.

    Default: 1000000

## TimeOutOnSend

Socket timeout in seconds for sending to clients.

    Default: 4

## RingbufferMinSize

The minimum size of the Ringbuffer, used for storing temporary DLT messages, until client is connected.

    Default: 500000

## RingbufferMaxSize

The max size of the Ringbuffer, used for storing temporary DLT messages, until client is connected.

    Default: 10000000

## RingbufferStepSize

The step size the Ringbuffer is increased, used for storing temporary DLT messages, until client is connected.

    Default: 500000

## Daemon FIFOSize

The size of Daemon FIFO (MinSize: depend on pagesize of system, MaxSize: please check `/proc/sys/fs/pipe-max-size`)
This is only supported for Linux.

    Default: 65536

## ContextLogLevel

Initial log-level that is sent when an application registers. DLT_LOG_OFF = 0, DLT_LOG_FATAL = 1, DLT_LOG_ERROR = 2, DLT_LOG_WARN = 3, DLT_LOG_INFO = 4, DLT_LOG_DEBUG = 5, DLT_LOG_VERBOSE = 6

    Default: 4

## ContextTraceStatus

Initial trace-status that is sent when an application registers. DLT_TRACE_STATUS_OFF = 0, DLT_TRACE_STATUS_ON = 1

    Default: 0

## ForceContextLogLevelAndTraceStatus

Force log level and trace status of contexts to not exceed "ContextLogLevel" and "ContextTraceStatus". If set to 1 (ON) whenever a context registers or changes the log-level it has to be lower or equal to ContextLogLevel.

    Default: 0
    
## InjectionMode

If set to 0, the injection mode (see [here](./dlt_for_developers.md#DLT-Injection-Messages)) is disabled.

    Default: 1

# GATEWAY CONFIGURATION

## GatewayMode

Enable Gateway mode

    Default: 0

## GatewayConfigFile

Read gateway configuration from another location

    Default: /etc/dlt_gateway.conf

# Permission configuration

DLT daemon runs with e.g.
 User: covesa_dlt
 Group: covesa_dlt

DLT user applications run with different user and group than dlt-daemon but with supplimentory group: dlt_user_apps_group

<basedir>/dlt FIFO will be created by dlt-daemon with
    User: covesa_dlt
    Group: dlt_user_apps_group
    Permission: 620

so that only dlt-daemon can read and only processes in dlt_user_apps_group can write.

<basedir>/dltpipes will be created by dlt-daemon with
    User: covesa_dlt
    Group: covesa_dlt
    Permission: 3733 (i.e Sticky bit and SGID turned on)

<basedir>/dltpipes/dlt<PID> FIFO will be created by dlt application (user lib) with
    User: <user of the application>
    Group: covesa_dlt (inherited from <basedir>dltpipes/ due to SGID)
    Permission: 620

Thus DLT user applications (and also or attackers) can create the dlt<PID> FIFO
(for communication from dlt-daemon to DLT user application) under <basedir>/dltpipes/. Since sticky bit is set the applications who creates the FIFO can only rename/delete it.

Since SGID of <basedir>/dltpipes is set the group of dlt<PID> FIFO will be covesa_dlt which enables dlt daemon to have write permission on all the dlt<PID> FIFO.

One dlt user application cannot access dlt<PID> FIFO created by other dlt user application(if they run with different user).

Owner group of daemon FIFO directory(Default: /tmp/dlt) (If not set, primary group of dlt-daemon process is used).
Application should have write permission to this group for tracing into dlt. For this opton to work, dlt-daemon should have this group in it's supplementary group.

## DaemonFifoGroup

Owner group of daemon FIFO directory
(If not set, primary group of dlt-daemon process is used)
Application should have write permission to this group for tracing into dlt
For this opton to work, dlt-daemon should have this group in it's Supplementary group

    Default: group of dlt-daemon process (/tmp/dlt)

# CONTROL APPLICATION OPTIONS

## ControlSocketPath

Path to control socket.

    Default: /tmp/dlt-ctrl.sock

# OFFLINE TRACE OPTIONS

## OfflineTraceDirectory

Store DLT messages to local directory, if not set offline Trace is off.

    Default: /tmp

## OfflineTraceFileSize

This value defines the max size of a offline trace file, if offline trace is enabled. This value is defined in bytes. If the files size of the current used log file is exceeded, a new log file is created.

    Default: 1000000

## OfflineTraceMaxSize

This value defines the max offline Trace memory size, if offline trace is enabled. This value is defined in bytes. If the overall offline trace size is excedded, the oldest log files are deleted, until a new trace file fits the overall offline trace max size.

    Default: 4000000

## OfflineTraceFileNameTimestampBased

Filename timestamp based or index based. 1 = timestamp based, 0 = index based

    Default: Function is disabled

# LOCAL CONSOLE OUTPUT OPTIONS

## PrintASCII

Prints each received DLT message from the application in ASCII to the local console. This option should only be anabled for debugging purpose.

    Default: Function is disabled

## PrintHex

Prints each received DLT message from the application in ASCII to the local console. The payload is printed in Hex. This option should only be anabled for debugging purpose.

    Default: Function is disabled

## PrintHeadersOnly

Prints each received DLT message from the application in ASCII to the local console. Only the header is printed. This option should only be anabled for debugging purpose.

    Default: Function is disabled

# SERIAL CLIENT OPTIONS

## RS232DeviceName

If this value is set to a serial device name, e.g. /dev/ttyS0, a serial port is used for logging to a client.

    Default: Serial port for logging is disabled

## RS232Baudrate

The used serial baud rate, if serial loggin is enabled. The RS232DeviceName must be set to enable serial logging.

    Default: 115200

## RS232SyncSerialHeader

If serial logging is enabled, each received DLT message is checked to contain a serial header. If the DLT message contains no serial header, the message is ignored.

    Default: Function is disabled

# TCP CLIENT OPTIONS

## TCPSyncSerialHeader

Each received DLT message on a TCP connection is checked to contain a serial header. If the DLT message contains no serial header, the message is ignored.

    Default: Function is disabled

# ECU SOFTWARE VERSION OPTIONS

## SendECUSoftwareVersion

Periodically send ECU version info. 0 = disabled, 1 = enabled

    Default: Function is disabled

# PathToECUSoftwareVersion

Absolute path to file storing version information - if disabled the DLT version will be send.

    Default: Function is disabled.

# TIMEZONE INFO OPTIONS

# SendTimezone

Periodically send timezone info. 0 = disabled, 1 = enabled

    Default: Function is disabled

# OFFLINE LOGSTORAGE OPTIONS

## OfflineLogstorageMaxDevices

Maximum devices to be used as offline logstorage devices. 0 = disabled, 1 .. DLT_OFFLINE_LOGSTORAGE_MAX_DEVICES

    Default: 0 (Function is disabled)

## OfflineLogstorageDirPath

Path to store DLT offline log storage messages.

    Default: off

## OfflineLogstorageTimestamp

Appends timestamp in log file name. 0 = disabled, 1 = enabled

    Default: 0

## OfflineLogstorageDelimiter

Appends delimiter in log file name, only punctuation characters allowed.

    Default: _

## OfflineLogstorageMaxCounter

Wrap around value for log file count in file name.

    Default: UINT_MAX

## OfflineLogstorageCacheSize

Maximal used memory for log storage cache in KB.

    Default: 30000 KB

## UDPConnectionSetup

Enable or disable UDP connection. 0 = disabled, 1 = enabled

## UDPMulticastIPAddress

The address on which daemon multicasts the log messages

## UDPMulticastIPPort

The Multicase IP port. Default: 3491

# AUTHOR

Alexander Wenzel (alexander.aw.wenzel (at) bmw (dot) de)

# COPYRIGHT

Copyright (C) 2015 BMW AG. License MPL-2.0: Mozilla Public License version 2.0 <http://mozilla.org/MPL/2.0/>.

# BUGS

See Github issue: <https://github.com/COVESA/dlt-daemon/issues>

# SEE ALSO

**dlt-daemon(1)**, **dlt-system(1)**
