% DLT-LOGSTORAGE-CTRL(1)

# NAME

**dlt-logstorage-ctrl** - Trigger DLT Daemon to start/stop using an offline logstorage device

# SYNOPSIS

**dlt-logstorage-ctrl** \[**-h**\] \[**-c** ctype\] \[**-p** path\] \[**-e** ecu\] \[**-p\<prop\>**\] \[**-t** timeout\] \[**-v**\]

# DESCRIPTION

Send a trigger to DLT Daemon to connect/disconnect a certain offline logstorage device

# OPTIONS

-h

:   Display a short help text.

-c

:   Specify connection type: connect = 1, disconnect = 0.

-d

:   Run as daemon: prop = use proprietary handler

-p

:    Mount point path.

-e

:   Specify the ECU ID. Default is: ECU1.

-t

:    Specify connection timeout. Default is: 10s.

-S

:   Send message with serial header (Default: Without serial header)

-R

:   Enable resync serial header

# EXAMPLES

Activate the offline logstorage device mounted on /mnt/dltlog
    **dlt-logstorage-ctrl -c 1 -p /mnt/dltlog**

Deactivate the offline logstorage device mounted on /mnt/dltlog
    **dlt-logstorage-ctrl -c 0 -p /mnt/dltlog**

Run logstorage control application as daemon listen to udev events
    **dlt-logstorage-ctrl -d**

# EXIT STATUS

Non zero is returned in case of failure.

# AUTHOR

Christoph Lipka (clipka (at) jp.adit-jv (dot) com), Syed Hameed (shameed (at) jp.adit-jv (dot) com)

# COPYRIGHT

Copyright (C) 2015 Advanced Driver Information Technology, Bosch and DENSO. License MPL-2.0: Mozilla Public License version 2.0 <http://mozilla.org/MPL/2.0/>.

# BUGS

See Github issue: <https://github.com/COVESA/dlt-daemon/issues>

# SEE ALSO

**dlt-daemon(1)**
