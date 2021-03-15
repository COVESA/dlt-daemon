% DLT-RECEIVE(1)

# NAME

**dlt-receive** - Console based client for DLT Logging

# SYNOPSIS

**dlt-receive** \[**-h**\] \[**-a**\] \[**-x**\] \[**-m**\] \[**-s**\] \[**-o** filename\] \[**-c** limit\] \[**-v**\] \[**-y**\] \[**-b** baudrate\] \[**-e** ecuid\] \[**-f** filterfile\] \[**-p** port\] hostname/serial_device_name

# DESCRIPTION

Receive DLT messages from DLT daemon and print or store the messages.

## OPTIONS

-h

: Display a short help text.

-a

: Print DLT file; payload as ASCII.

-x

:   Print DLT file; payload as hex.

-m

:   Print DLT file; payload as hex and ASCII.

-s

:   Print DLT file; only headers.

-o

:   Output messages in new DLT file.

-c

:   Set limit when storing messages in file. When limit is reached, a new file is opened. Use K,M,G as suffix to specify kilo-, mega-, giga-bytes respectively, e.g. 1M for one megabyte (Default: unlimited).

-v

:   Verbose mode.

-y

:   Serial device mode.

-b

:   Serial device baudrate (Default: 115200).

-e

:   Set ECU ID (Default: RECV).

-f

:   Enable filtering of messages.

-p

:   Port for UDP and TCP communication (Default: 3490).
# EXAMPLES

Print received message headers received from a dlt-daemon running on localhost::
    **dlt-receive -s localhost**

Print received message headers received from a serila interface::
    **dlt-receive -s -y /dev/ttySO**

Store received message headers from a dlt-daemon to a log file called log.dlt and filter them for e.g. Application ID ABCD and Context ID EFGH (Write:ABCD EFGH as single line to a file called filter.txt)::
    **dlt-receive -s -o log.dlt -f filter.txt localhost**

Store incoming messages in file(s) and restrict file sizes to 1 megabyte. If limit is reached, log.dlt will be renamed into log.0.dlt, log.1.dlt, ... No files will be overwritten in this mode::
    **dlt-receive -o log.dlt -c 1M localhost**

# EXIT STATUS

Non zero is returned in case of failure.

# NOTES

Be aware that dlt-receive will never delete any files. Instead, it creates a new file.

# AUTHOR

Alexander Wenzel (alexander.aw.wenzel (at) bmw (dot) de)

# COPYRIGHT

Copyright (C) 2015 BMW AG. License MPL-2.0: Mozilla Public License version 2.0 <http://mozilla.org/MPL/2.0/>.

# BUGS

See Github issue: <https://github.com/GENIVI/dlt-daemon/issues>

# SEE ALSO

**dlt-daemon(1)**
