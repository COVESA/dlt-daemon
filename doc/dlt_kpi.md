# DLT KPI

Back to [README.md](../README.md)

## Overview

*DLT KPI* is a tool to provide log messages about **K**ey **P**erformance **I**ndicators
to the DLT Daemon. The log message format is designed to be both readable by
humans and to be parsed by DLT Viewer plugins. The information source for the
dlt-kpi tool is the /proc file system.

## Message format

*DLT KPI* logs its messages as human readable ASCII messages, divided in
multiple arguments. The tool will log messages in user defined intervals, which
can be set in the configuration file dlt-kpi.conf.

## Identifiers and their datasets

The logged messages always start with a three character long identifier as first
argument. After this identifier, they can contain multiple datasets separated in
the remaining arguments. The datasets contain information separated by
semicolons. The order and meaning of those information chunks is defined below.

The following will explain the meaning to each three-character-identifier and
each information chunk of the datasets associated with this identifier. The
example messages all contain only one dataset - in real use, many messages will
contain multiple datasets (one per argument).

*NOTE*: Arguments are delimited by spaces when shown in ASCII, but dlt-viewer
plugins can easily access each argument separately by certain methods, which
makes arguments useful for parsing.

### NEW
 This identifies a message that contains datasets describing newly created
processes.

The datasets in these messages have the following form:

`[PID];[Parent PID];[Commandline]`

Example message:

`NEW 21226;1;/usr/libexec/nm-dispatcher`

### STP
This identifies a message that contains datasets describing processes
that have ended since the last interval.

The datasets in these messages have the following form:

`[PID]`

Example message:

`STP 20541`

### ACT
This identifies a message that contains datasets describing active
processes. These are processes that have consumed CPU time since the last
interval.

The datasets in these messages have the following form:

`[PID];[CPU time in milliseconds per second];[RSS bytes];[CTX-switches since last interval];[I/O bytes];[I/O wait time in milliseconds per second]`

Example message:

`ACT 20503;10;389;3;1886649;0`

*NOTE:* The *CPU time* value is the active time of a process in milliseconds,
divided by the number of CPU cores. So this value should never get greater than
1000ms, which would mean 100% CPU usage.

### CHK
This identifies a message that is logged for each process in a certain
interval. These messages can be used to get a list of currently existing processes and to keep a plugin, that tracks running processes, up to date if messages were lost or if the commandlines have changed.

The datasets in these messages have the following form:

`[PID];[Commandline]`

Example message:

`CHK 660;/sbin/audispd`

### IRQ
This identifies a message that contains datasets describing the numbers of interrupts that occurred on each CPU.

The datasets in these messages have the following form:

`[IRQ name];cpu[CPU number];[Number of total interrupts];`

Example message:

`IRQ 0;cpu0:133;cpu1:0; 1;cpu0:76827;cpu1:0;`

## Synchronization messages

Because the messages can get too long for logging and segmented network messages
don't allow for individually set arguments, the datasets can be splitted into
multiple messages of the same type (i.e. they have the same identifier). This
can make it difficult for an observer (human or machine) to keep track of
currently valid information. For example, one can't be sure if a process is
part of the list of currently active processes or not, or if this message was
part of an older interval that simply arrived too late. So, to correctly
associate these messages to each other, each group of potentially "segmented"
messages is surrounded by two synchronization messages which start with the same
identifier, followed by the codes _BEG_ (for the opening sync message) or _END_
(for the closing sync message). Synchronization messages do not contain datasets.

Example (Messages have been shortened for simplicity):

```c
ACT BEG
ACT 21768;10;417;3;672075;0 19284;20;15857;303654;22932174;0 1826;20;39781;4404293;154392870;0
ACT 1635;10;10696;8412557;375710810;0 990;10;22027;1176631;0;0
ACT END
```

Only processes that are part of this group are active at this moment. *ACT*
messages that came before this message-group are invalid now.

It can also happen that, between a *BEG* and an *END* sync message, there are
messages of other types. So, plugins should not expect these message groups to
always be a "solid block", but react on each message individually and
dynamically, and store the logged information until the closing *END* message arrives.

## AUTHOR

Sven Hassler <Sven_Hassler (at) mentor (dot) com>

## COPYRIGHT

Copyright (C) 2015 BMW AG. License MPL-2.0: Mozilla Public License version 2.0 <http://mozilla.org/MPL/2.0/>.
