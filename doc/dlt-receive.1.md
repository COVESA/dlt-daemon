% DLT-RECEIVE(1)

# NAME

**dlt-receive** - Console based client for DLT Logging

# SYNOPSIS

**dlt-receive** \[**-h**\] \[**-a**\] \[**-x**\] \[**-m**\] \[**-s**\] \[**-o** filename\] \[**-c** limit\] \[**-v**\] \[**-y**\] \[**-b** baudrate\] \[**-e** ecuid\] \[**-f** filterfile\] \[**-j** filterfile\] \[**-p** port\] hostname/serial_device_name

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

-S

:   Send message with serial header (Default: Without serial header)

-R

:   Enable resync serial header

-y

:   Serial device mode.

-b

:   Serial device baudrate (Default: 115200).

-e

:   Set ECU ID (Default: RECV).

-f

:   Enable filtering of messages. Takes a space separated filter file (see [Space separated filter file](#Space-separated-filter-file)).

-j

:   Enable extended filtering of messages. Takes a json filter file ([Json filter file](#Json-filter-file)).

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

## Space separated filter file
File that defines multiple filters. Can be used as argument for `-f` option. With this it's only possible to filter messages depending on their Application ID and/or Context ID. The syntax is: first AppID and optional a CtxID behind it, with a space in between. Each line defines a filter and the maximum number of filters is 30.

Example:
```
DLTD INTM
DLT INT
TEST
```

## Json filter file
Only available, when builded with cmake option `WITH_EXTENDED_FILTERING`.

File that defines multiple filters. Can be used as argument for `-j` option. With this it's also possible to filter messages depending on their Application ID, Context ID, log level and payload size. The following example shows the syntax. Names of the filters can be customized, but not more than 15 characters long. The maximum number of filters is also 30.

Example:
```
{
"filter1": {
    "AppId": "LOG",
    "ContextId": "TEST",
    "LogLevel": "3"
    },
"filter2": {
    "AppId": "app",
    "LogLevel": "4"
    },
"filter3": {
    "AppId": "app2",
    "ContextId": "con2",
    "PayloadMin": "20",
    "PayloadMax": "50"
    }
}
```
# EXIT STATUS

Non zero is returned in case of failure.

# NOTES

Be aware that dlt-receive will never delete any files. Instead, it creates a new file.

# AUTHOR

Alexander Wenzel (alexander.aw.wenzel (at) bmw (dot) de)

# COPYRIGHT

Copyright (C) 2015 BMW AG. License MPL-2.0: Mozilla Public License version 2.0 <http://mozilla.org/MPL/2.0/>.

# BUGS

See Github issue: <https://github.com/COVESA/dlt-daemon/issues>

# SEE ALSO

**dlt-daemon(1)**
