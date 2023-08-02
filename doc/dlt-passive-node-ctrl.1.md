% DLT-PASSIVE-NODE-CTRL(1)

# NAME

**dlt-passive-node-ctrl** - Send a trigger to DLT daemon to (dis)connect a passive node or get current passive node status

# SYNOPSIS

**dlt-passive-node-ctrl** \[**-h**\] \[**-c**\] \[**-n** ecu\] \[**-s**\] \[**-t** timeout\] \[-v\]

# DESCRIPTION

Send a trigger to DLT daemon to (dis)connect a passive node or get current passive node status

## OPTIONS

-h

:   Usage

-c

:    Connection status (1 - connect, 0 - disconnect)

-n

:   Passive Node identifier (e.g. ECU2)

-s

:   Show passive node(s) connection status

-t

:   Specify connection timeout (Default: 10s)

-S

:   Send message with serial header (Default: Without serial header)

-R

:   Enable resync serial header

-v

:   Set verbose flag (Default:0)

# EXAMPLES

Get status about connected passives nodes
    **dlt-passive-node-ctrl -s**

Connect to passive node ECU2
    **dlt-passive-node-ctrl -c 1 -n ECU2**

Disconnect to passive node ECU2
    **dlt-passive-node-ctrl -c 0 -n ECU2**

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
