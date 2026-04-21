% DLT-ADAPTOR-STDIN(1)

# NAME

**dlt-adaptor-stdin** - Forward input from stdin to DLT Daemon

# SYNOPSIS

**dlt-adaptor-stdin** \[**-a** apid\] \[**-c** ctid\] \[**-b**\] \[**-t** timeout\] \[**-v** level\] \[**-h**\]

# DESCRIPTION

This is a small external program for forwarding input from stdin to DLT Daemon.

## OPTIONS

-a

:    Set application ID to apid (default: SINA)

-c

:    Set context ID to ctid (default: SINC)

-b

:    Flush buffered logs before unregistering the application

-t

:    Set timeout when sending messages at exit, in ms (default: 10000 = 10sec)

-v

:    Set verbosity level (default: INFO; accepted values: FATAL ERROR WARN INFO DEBUG VERBOSE)

-h

:    Show help

# EXAMPLES

Forward all dmesg to DLT Daemon without discarding any messages
    **dmesg | dlt-adaptor-stdin -b**

Send DBUS messages to DLT Daemon using the program dbus-monitor
    **dbus-monitor | dlt-adaptor-stdin**

# EXIT STATUS

Non zero is returned in case of failure.

# AUTHOR

Saya Sugiura (ssugiura (at) jp.adit-jv (dot) com)

# COPYRIGHT

Copyright (C) 2019 Advanced Driver Information Technology, Bosch and DENSO. License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.

# BUGS

See Github issue: <https://github.com/COVESA/dlt-daemon/issues>

# SEE ALSO

**dlt-daemon(1)**
