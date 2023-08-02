% DLT-ADAPTOR-UDP(1)

# NAME

**dlt-adaptor-udp** - Forward received UDP messages to DLT Daemon

# SYNOPSIS

**dlt-adaptor-udp** \[**-a** apid\] \[**-c** ctid\] \[**-p**\] \[**-h**\]

# DESCRIPTION

This is a small external program for forwarding received UDP messages to DLT
Daemon.

This also can be used for e.g. sending syslog messages to the DLT daemon.
Therefore a syslog daemon called *syslog-ng* is necessary. This syslog daemon
must be configured to send the syslog messages to a specific UDP port. For
configuration of this syslog daemon, see the documentation for *syslog-ng*.
This tools is already integrated into *dlt-system*.

## OPTIONS

-a

:    Set application ID to apid (default: UDPA)

-c

:    Set context ID tp ctid (default: UDPC)

-p

:    Set receive port number for UDP messages (default: 47111)

-h

:    Show help

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
