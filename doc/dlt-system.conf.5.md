% DLT-SYSTEM.CONF(5)

# NAME

**dlt-system.conf** - DLT system process configuration file

# DESCRIPTION

The DLT system logging process is the central application, which logs system information from the platform. It provides the features
filetransfer, syslog adapater, logging of any kind of files and
procfilesystem logger. The individual features can be enabled and
disabled in the configuration file.

The configuration file dlt-system.conf allows to configure the different runtime behaviour of dlt-system. The configuration file is loaded during startup of dlt-system.

dlt-system loads by default the configuration file  /etc/dlt-system.conf.
An alternative configuration file can be loaded with the option -c.

# GENERAL OPTIONS

## ApplicationId

The application Id used for the dlt-system process.

    Default: SYS

# SHELL OPTIONS

## ShellEnable

Enable the Shell for command line injections. Be careful when you enable this feature. The user can send any kind of shell commands. The commands are executed with the rights of the dlt-system process. Dlt-system is started by default as user covesa.

    Default: 0

# SYSLOG ADAPTER OPTIONS

## SyslogEnable

If this option is set to 1, the syslog adapter feature is enabled. SyslogPort needs to be configured too if Syslog is enabled.

    Default: 0

## SyslogContextId

This value defines context id of the syslog adapter.

    Default: SYSL

## SyslogPort

This value defines the UDP port opened for receiving log messages from syslog. Configuration for syslog to forward log to this port is necessary. Adding this config `` *.*    @localhost:47111 `` in config file of syslog (usually in /etc/rsyslog.d/50-default.conf) and restart the syslog service by command "sudo systemctl restart rsyslog.service".

    Default: 47111

# SYSTEMD JOURNAL ADAPTER OPTIONS

## JournalEnable

Enable the Systemd Journal Adapter. This feature is only available, when dlt is compiled with the option "WITH_SYSTEMD_JOURNAL".  Dlt-system is started by default as user covesa, see dlt-system.service file. The user covesa must be added to one of the groups 'adm', 'wheel' or 'systemd-journal' to have access to all journal entries.

    Default: 0

## JournalContextId

The Context Id of the journal adapter.

    Default: JOUR

## JournalCurrentBoot

Show only log entries of current boot and follow. If JournalCurrentBoot and JournalFollow are not set all persistent journal entries will be logged.

    Default: 1

## JournalFollow

Show only the last 10 entries and follow.

    Default: 0

## JournalMapLogLevels

Map journal log levels to DLT log levels.

    - 0       Emergency     DLT_LOG_FATAL
    - 1       Alert         DLT_LOG_FATAL
    - 2       Critical      DLT_LOG_FATAL
    - 3       Error         DLT_LOG_ERROR
    - 4       Warning       DLT_LOG_WARN
    - 5       Notice        DLT_LOG_INFO
    - 6       Informational DLT_LOG_INFO
    - 7       Debug         DLT_LOG_DEBUG

    Default: 1


# FILETRANSFER OPTIONS

## FiletransferEnable

Enable the Filetransfer feature. 0 = disabled, 1 = enabled

    Default: 0

## FiletransferContextId

The Context Id of the filetransfer.

    Default: FILE

## FiletransferTimeStartup

Time in seconds after startup of dlt-system when first file is transfered.

    Default: 0

## FiletransferTimeoutBetweenLogs

Time in seconds to wait between two file transfer logs of a single file to DLT.

    Default: 10

## FiletransferDirectory

You can define multiple file transfer directories. Define the directory to watch, whether to compress the file with zlib and the zlib compression level. For parsing purposes, FiletransferCompressionLevel must be the last one of three values.

## FiletransferCompression

See FiletransferDirectory option for explanation.

    Default: 0

## FiletransferCompressionLevel

See FiletransferDirectory option for explanation.

    Default: 5

# LOG FILES OPTIONS

## LogFileEnable

If this option is set to 1, the log files feature is enabled.

    Default: 0

## LogFileFilename

This value sets the full filename path to the file, which should be logged.

## LogFileMode

This value defines in which operation mode the file is logged. In mode 1 the file is only logged once when dlt-system is started. In mode 2 the file is logged regularly every time LogFileTimeDelay timer elapses. 0 = off, 1 = startup only, 2 = regular

## LogFileTimeDelay

This value is used in mode 3 and defines the number of seconds, after which the defined file is logged.

## LogFileContextId

This value defines the context id, which is used for logging the file.

# LOG PROCESSES OPTIONS

## LogProcessesEnable

Enable the logging of processes. 0 = disabled, 1 = enabled

    Default: 0

## LogProcessesContextId

This value defines the context id, which is used for logging processes files.

    Default: PROC

## LogProcessName

This value defines the name of the process to be logged, as used in the file stat of each process. If the value is defined as *, all processes are logged.

## LogProcessFilename

This value sets the relative filename path to the file, which should be logged. The path is relative to the procfilesystem folder of the process.

## LogProcessMode

This value the defines in which operation mode this process file is logged. In mode 1 the file is only logged once when dlt-system is started. In mode 2 the file is logged regularly every time LogFileTimeDelay timer elapses. 0 = off, 1 = startup only, 2 = regular.

    Default: 0

## LogProcessTimeDelay

This value is used in mode 3 and defines the number of seconds, after which the defined procfilesystem file is logged.

    Default: 0

# AUTHOR

Alexander Wenzel (alexander.aw.wenzel (at) bmw (dot) de)

# COPYRIGHT

Copyright (C) 2015 BMW AG. License MPL-2.0: Mozilla Public License version 2.0 <http://mozilla.org/MPL/2.0/>.

# BUGS

See Github issue: <https://github.com/COVESA/dlt-daemon/issues>

# SEE ALSO

**dlt-system(1)**, **dlt-daemon(1)**
