% DLT-EXAMPLE-USER(1)

# NAME

**dlt-example-user** - Console based application for sending a custom dlt message

# SYNOPSIS

**dlt-example-user** \[**-h**\] \[**-g**\] \[**-a**\] \[**-k**\] \[**-d** delay\] \[**-f** filename\] \[**-S** filesize\] \[**-n** count\] \[**-m** mode\] \[**-l** level\] \[**-A** appID\] \[**-C** contextID\] \[**-t** timeout\] \[**-s** size\] message

# DESCRIPTION

Sends the given message as DLT messages to DLT daemon or prints the raw DLT messages into a local file.

## OPTIONS

-h

: Display a short help text.

-g

: Switch to non-verbose mode (Default: verbose mode).

-a

: Enable local printing of DLT messages (Default: disabled).

-k

: Send marker message.

-d

: Milliseconds to wait between sending messages (Default: 500).

-f

: Use local log file instead of sending to daemon.

-S

: Set maximum size of local log file (Default: UINT\_MAX).

-n

: Number of messages to be generated (Default: 10).

-m

: Set log mode 0=off, 1=external, 2=internal, 3=both.

-l

: Set log level, level=-1..6 (Default: 3).

-A

: Set app ID for send message (Default: LOG).

-C

: Set context ID for send message (Default: TEST).

-t

: Set timeout when sending messages at exit, in ms (Default: 10000 = 10sec).

-r

: Send raw data with specified size instead of string.


# EXAMPLES

Send "HelloWorld" with default settings (10 times, every 0.5 seconds) as DLT message to dlt-daemon::

    dlt-example-user HelloWorld

Set app ID to `APP1`, context Id to `TEST` and log level to `error` for send message::

    dlt-example-user -l 2 -A APP1 -C TEST HelloWorld

Send 100 DLT messages every second::

    dlt-example-user -n 100 -d 1000 HelloWorld

Send "HelloWorld" can log to local file with maximum size 1000 bytes::

    dlt-example-user -f helloworld.dlt -S 1000 HelloWorld

# EXIT STATUS

Non zero is returned in case of failure.

# Notes

The default descriptions for application and context registration are used irrespective of the IDs that could be set. App will always register with "Test Application for Logging" and context with "Test Context for Logging".

# AUTHOR

Darian Biastoch (dbiastoch@de.adit-jv.com)

# COPYRIGHT

Copyright (C) 2020 ADIT GmbH. License MPL-2.0: Mozilla Public License version 2.0 <http://mozilla.org/MPL/2.0/>.

# BUGS

See Github issue: <https://github.com/COVESA/dlt-daemon/issues>

# SEE ALSO

**dlt-daemon(1)**
