% DLT-SYSTEM(1)

# NAME

**dlt-system** - DLT system logging process

# SYNOPSIS

**dlt-system** \[**-h**\] \[**-d**\] \[**-c** filename\]

#DESCRIPTION

The DLT system logging process is the central application, which logs system information from the platform. It provides the features
filetransfer, syslog adapater, logging of any kind of files and
procfilesystem logger. The individual features can be enabled and
disabled in the configuration file.

dlt-system loads by default the configuration file  /etc/dlt-system.conf.

## OPTIONS

-h

:   Display a short help text.

-d

:   Daemonize, needed in System V init systems.

-c

:   Load an alternative configuration file. By default the configuration file /etc/dlt.conf is loaded.

# EXAMPLES

Start DLT system with custom configuration:
    **dlt-system -c ~/my-configuration.cfg**

# EXIT STATUS

Non zero is returned in case of failure.

# AUTHOR

Alexander Wenzel (alexander.aw.wenzel (at) bmw (dot) de)

# COPYRIGHT

Copyright (C) 2015 BMW AG. License MPL-2.0: Mozilla Public License version 2.0 <http://mozilla.org/MPL/2.0/>.

# BUGS

See Github issue: <https://github.com/COVESA/dlt-daemon/issues>

# SEE ALSO

**dlt-system.conf(5)**, **dlt-daemon(1)**
