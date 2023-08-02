% DLT-SORTBYTIMESTAMP(1)

# NAME

**dlt-sortbytimestamp** - Re-order DLT Logging files according to message creation time and timestamp.

# SYNOPSIS

**dlt-sortbytimestamp** \[**-h**\] \[**-v**\] \[**-c**\] \[**-f** filterfile\] \[**-b** number\] \[**-e** number\] dltfile_in dltfile_out

# DESCRIPTION

By default messages in DLT files are ordered according to the time the logger received them. This can unhelpful when tracing a sequence of events on a busy multi-threaded/multi-core system, because thread pre-emption combined with multiple processes attempting to log messages simultaneously means that the order in which the messages are received may vary significantly from the order in which they were created.

*dlt-sortbytimestamp* is able to re-order a DLT input file's messages according both their creation time and timestamp, and writes them to an output DLT file.

# NOTE

Use the \*-b\* and/or \*-e\* options to specify a range of messages within a single reboot cycle and all will be well.

Hint: use *dlt-viewer* to ascertain the endpoints of the range in question.

# OPTIONS

-h

:   Display a short help text.

-v

:   Verbose mode. Repeat to give more information.

-c

:   Count number of messages.

-f

:   Enable filtering of messages. Incompatible with range options.

-b

:   First message to be handled. Zero based index.

-e

:   Last message to be handled. Zero based index.

# EXAMPLES

Sort an entire file by message timestamp:
    **dlt-sortbytimestamp input.dlt output.dlt**

Sort a specific range, e.g. from message 1,000,000 to message 1,500,000 from a file called input.dlt and store the result in a file called output.dlt:
    **dlt-sortbytimestamp -b 1000000 -e 1500000 input.dlt output.dlt**

# EXIT STATUS

Non zero is returned in case of failure.

# AUTHOR

Jonathan Sambrook (jonathan.sambrook (at) codethink (dot) co (dot) uk)
Alexander Wenzel (alexander.aw.wenzel (at) bmw (dot) de)

# COPYRIGHT

Copyright (C) 2018 Codethink Ltd.
Copyright (C) 2015 BMW AG.
License MPL-2.0: Mozilla Public License version 2.0 <http://mozilla.org/MPL/2.0/>.

# BUGS

See Github issue: <https://github.com/COVESA/dlt-daemon/issues>

# SEE ALSO

**dlt-daemon(1)**, **dlt-convert(1)**
