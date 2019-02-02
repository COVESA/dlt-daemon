% DLT.CONF(5)

# NAME

**dlt.conf** - DLT daemon configuration file

# DESCRIPTION

The DLT daemon is the central application which gathers logs and traces from different applications, stores them temporarily or permanently and transfers them to a DLT client application, which could run directly on the GENIVI system or more likely on some external tester device.

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

The logging console for internal logging of dlt-daemon. 0 = log to stdout, 1 = log to syslog, 2 = log to file (see LoggingFilename)

    Default: 0

## LoggingLevel

The internal log level, up to which logs are written. LOG_EMERG = 0, LOG_ALERT = 1, LOG_CRIT = 2, LOG_ERR = 3, LOG_WARNING = 4, LOG_NOTICE = 5, LOG_INFO = 6, LOG_DEBUG = 7

    Default: 6

## LoggingFilename

If LoggingMode is set to 2 logs are written to the file path given here.

    Default: /tmp/dlt.log

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

## ContextLogLevel

Initial log-level that is sent when an application registers. DLT_LOG_OFF = 0, DLT_LOG_FATAL = 1, DLT_LOG_ERROR = 2, DLT_LOG_WARN = 3, DLT_LOG_INFO = 4, DLT_LOG_DEBUG = 5, DLT_LOG_VERBOSE = 6

    Default: 4

## ContextTraceStatus

Initial trace-status that is sent when an application registers. DLT_TRACE_STATUS_OFF = 0, DLT_TRACE_STATUS_ON = 1

    Default: 0

## ForceContextLogLevelAndTraceStatus

Force log level and trace status of contexts to not exceed "ContextLogLevel" and "ContextTraceStatus". If set to 1 (ON) whenever a context registers or changes the log-level it has to be lower or equal to ContextLogLevel.

    Default: 0

# GATEWAY CONFIGURATION

## GatewayMode

Enable Gateway mode

    Default: 0

## GatewayConfigFile

Read gateway configuration from another location

    Default: /etc/dlt_gateway.conf

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

# AUTHOR

Alexander Wenzel (alexander.aw.wenzel (at) bmw (dot) de)

# COPYRIGHT

Copyright (C) 2015 BMW AG. License MPL-2.0: Mozilla Public License version 2.0 <http://mozilla.org/MPL/2.0/>.

# BUGS

See Github issue: <https://github.com/GENIVI/dlt-daemon/issues>

# SEE ALSO

**dlt-daemon(1)**, **dlt-system(1)**
