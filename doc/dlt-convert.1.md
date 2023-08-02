% DLT-CONVERT(1)

# NAME

**dlt-convert** - Convert DLT Logging files into ASCII

# SYNOPSIS

**dlt-convert** \[**-h**\] \[**-a**\] \[**-x**\] \[**-m**\] \[**-s**\] \[**-t**\] \[**-o** filename\] \[**-v**\] \[**-c**\] \[**-f** filterfile\] \[**-b** number\] \[**-e** number\] \[**-w**\] file1 \[file2\] \[file3\]

# DESCRIPTION

Read DLT files, print DLT messages as ASCII and store the messages again.
Use Ranges and Output file to cut DLT files.
Use two files and Output file to join DLT files.

## OPTIONS

-h

:   Display a short help text.

-a

:   Print DLT file; payload as ASCII.

-x

:   Print DLT file; payload as hex.

-m

:   Print DLT file; payload as hex and ASCII.

-s
    Print DLT file; only headers.

-o

:    Output messages in new DLT file.

-v

:    Verbose mode.

-c

:    Count number of messages.

-f

:   Enable filtering of messages.

-b

:   First messages to be handled.

-e

:   Last message to be handled.

-w

:   Follow dlt file while file is increasing.

-t

:   Handling the compressed input files (tar.gz).

# EXAMPLES

Convert DLT file into ASCII:
    **dlt-convert -a mylog.dlt**

Cut a specific range, e.g. from message 1 to message 3 from a file called log.dlt and store the result to a file called newlog.dlt:
    **dlt-convert -b 1 -e 3 -o newlog.dlt log.dlt**

Paste two dlt files log1.dlt and log2.dlt to a new file called newlog.dlt:
    **dlt-convert -o newlog.dlt log1.dlt log2.dlt**

Handle the compressed input files and join inputs into a new file called newlog.dlt:
    **dlt-convert -t -o newlog.dlt log1.dlt compressed_log2.tar.gz**

# EXIT STATUS

Non zero is returned in case of failure.

# AUTHOR

Alexander Wenzel (alexander.aw.wenzel (at) bmw (dot) de)

# COPYRIGHT

Copyright (C) 2015 BMW AG. License MPL-2.0: Mozilla Public License version 2.0 <http://mozilla.org/MPL/2.0/>.

# BUGS

See Github issue: <https://github.com/COVESA/dlt-daemon/issues>

# SEE ALSO

**dlt-daemon(1)**
