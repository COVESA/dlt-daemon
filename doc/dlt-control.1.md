% DLT-CONTROL(1)

# NAME

**dlt-control** - Send control messages to DLT Daemon

# SYNOPSIS

**dlt-control** \[**-v**\] \[**-h**\] \[**-S**\] \[**-R**\] \[**-y**\] \[**-b** baudrate\] \[**-e** ecuid\] \[**-a** id\] \[**-c** id\] \[**-s** id\] \[**-m** message\] \[**-x** message\] \[**-t** milliseconds\] \[**-l** level\] \[**-r** tracestatus\] \[**-d** loglevel\] \[**-f** tracestatus\] \[**-i** enable\] \[**-o**\] \[**-g**\] \[**-j**\] \[**-u**\] \[**-p** port\] hostname/serial\_device\_name

# DESCRIPTION

Send control messages to DLT Daemon.
This is useful when there is no client (e.g. DLT Viewer) available.
It supports several control messages including:
- Setting log level/trace level
- Setting default log level/default trace level
- Enable timing packets
- Store configuration
- Reset to factory default
- Get logging information

**Note** Use -u option instead of hostname/serial\_device\_name if Unix Socket
is used. See example for detail.

## OPTIONS

-v

:    Verbose mode

-h

:    Usage

-S

:    Send message with serial header (Default: Without serial header)

-R

:    Enable resync serial header

-y

:    Serial device mode

-b

:    Serial device baudrate (Default: 115200)

-e

:    Set ECU ID (Default: RECV)

-a

:    Control message application id

-c

:    Control message context id

-s

:    Control message injection service id

-m

:    Control message injection in ASCII

-x

:    Control message injection in Hex e.g. 'ad 01 24 ef'

-t

:    Timeout to terminate application (Default:1000)

-l

    Set the log level (0=off - 6=verbose, default= -1)
    supported options:
      -l level -a apid -c ctid
      -l level -a abc* (set level for all ctxts of apps name starts with abc)
      -l level -a apid (set level for all ctxts of this app)
      -l level -c xyz* (set level for all ctxts whose name starts with xyz)
      -l level -c ctid (set level for the particular ctxt)
      -l level (set level for all the registered contexts)

-r

:    Set the trace status (0=off - 1=on, default=255)
     supported options:
       -r tracestatus -a apid -c ctid
       -r tracestatus -a abc* (set status for all ctxts of apps name starts with abc)
       -r tracestatus -a apid (set status for all ctxts of this app)
       -r tracestatus -c xyz* (set status for all ctxts whose name starts with xyz)
       -r tracestatus -c ctid (set status for the particular ctxt)
       -r tracestatus (set status for all the registered contexts)

-d

:    Set the default log level (0=off - 5=verbose)

-f

:    Set the default trace status (0=off - 1=on)

-i

:    Enable timing packets (0=off - 1=on)

-o

:    Store configuration

-g

:    Reset to factory default

-j

:    Get log info

-u
:    unix port

-p

:    Port for TCP communication (Default: 3490).

# EXAMPLES

Change log level of application "APP1" to DEBUG with unix port
    **dlt-control -a APP1 -l 5 -u**

Change log level of application "APP1" and context "CON1" to ERROR
    **dlt-control -a APP1 -c CON1 -l 2 localhost**

Get logging information of current running applications with unix port (IPC: Unix Socket)
    **dlt-control -j -u**

Get logging information of current running applications (IPC:FIFO)
    **dlt-control -j localhost**

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
