% DLT-TEST-QNX-SLOGGER(1)

# NAME

**dlt-test-qnx-slogger** - Console based test application for sending messages
to QNX slogger

# SYNOPSIS

**dlt-test-qnx-slogger** \[**-h**\] \[**-n** count\] \[**-d** delay\] \[**-l** length\]


# DESCRIPTION

The binary writes specific amount of messages to slogger2. This can be used to
test `dlt-qnx-system` ([dlt-qnx-system.md](dlt_qnx_system.md)).

## OPTIONS

-h

:   Display a short help text.

-n

:   Number of messages to be generated (Default: 10).

-d

:   Milliseconds to wait between sending messages (Default: 500).

-l

:   Messages length (Default: 100 bytes).

# Examples

Send 100 messages every 1 second:

    dlt-test-qnx-slogger -n 100 -d 1000

# EXIT STATUS

Non zero value is returned in case of failure.

# AUTHOR

Saya Sugiura (ssugiura@jp.adit-jv.com)

# COPYRIGHT

Copyright (C) 2021 ADIT GmbH. License MPL-2.0: Mozilla Public License version 2.0 <http://mozilla.org/MPL/2.0/>.

# BUGS

See Github issue: <https://github.com/COVESA/dlt-daemon/issues>

# SEE ALSO

**dlt-qnx-system.md(1)**
