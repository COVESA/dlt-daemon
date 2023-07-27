% DLT-DAEMON(1)

# NAME

**dlt-daemon** - Diagnostic Log and Trace daemon

# SYNOPSIS

**dlt-daemon** \[**-h**\] \[**-d**\] \[**-c** filename\] \[**-t** directory\] \[**-p** port\] \[**-a** filename\]

# DESCRIPTION

The DLT daemon is the central place where logs and traces are gathered
from different applications, stored temporarily or permanently and
transferred to a DLT client application, which can run directly on the
COVESA system or more likely on a external tester device.

## OPTIONS

-h

:   Display a short help text.

-d

:   Daemonize, needed in System V init systems.

-c

:   Load an alternative configuration file. By default the configuration file /etc/dlt.conf is loaded.

-t

:   Directory for local fifo and user-pipes (Default: /tmp).
    Applications wanting to connect to a daemon using a custom directory need to be started with the environment variable DLT_PIPE_DIR set appropriately.

-p

:   Port to monitor for incoming requests (Default: 3490)
    Applications wanting to connect to a daemon using a custom
    port need to be started with the environment variable
    DLT_DAEMON_TCP_PORT set appropriately.

-a

:   Load an alternative configuration for app id log level defaults.
    By default, the configuration file /etc/dlt-log-levels.conf is loaded.


# EXAMPLES

Start DLT daemon in background mode:
    **dlt-daemon -d**

Start DLT daemon with own configuration:
    **dlt-daemon -c ~/my-dlt-configuration.cfg**

Start DLT daemon with custom pipes directory:
    **dlt-daemon -t ~/dlt_pipes**

Start DLT daemon listening on custom port 3500:
    **dlt-daemon -p 3500**

# EXIT STATUS

Non zero is returned in case of failure.

# AUTHOR

Alexander Wenzel (alexander.aw.wenzel (at) bmw (dot) de)

# COPYRIGHT

Copyright (C) 2016 BMW AG. License MPL-2.0: Mozilla Public License version 2.0 <http://mozilla.org/MPL/2.0/>.

# BUGS

See Github issue: <https://github.com/COVESA/dlt-daemon/issues>

# SEE ALSO

**dlt.conf(5)**, **dlt-system(1)**
