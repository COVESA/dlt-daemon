% DLT_GATEWAY.CONF(5)

# NAME

**dlt_gateway.conf** - DLT configuration file for gateway

# DESCRIPTION

The configuration file dlt_gateway.conf allows to configure the different runtime behaviour of gateway in Multinode feature.

The configuration file is written in an INI file format and contains information about different connected passive nodes.

If Multinode feature is enabled, dlt-daemon loads by default the configuration file /etc/dlt_gateway.conf.

An alternative configuration file can be loaded by changing `GatewayConfigFile` in dlt.conf(5).

# GENERAL SECTION

### Interval

Time interval for reconnection to passive Node in second.

    Default: 1

# PASSIVENODE SECTION

Each passive node’s connection parameters are specified in a unique numbered separate section

([PassiveNode{1,2, …N}]).

    Example: [PassiveNode1]

### IPaddress

Because TCP is the only supported communication channel,

the IPaddress and Port of the Passive DLT Daemon has to be specified.

IP Address of passive node. Mandatory

### Port

TCP port. Default 3490 is used if no port is specified.

    Default: 3490

### EcuID

ECU identifier of passive node. Mandatory.

### Connect

With the Connect property it is possible to specify when the Gateway DLT Daemon
shall connect to the passive node.

    Default: OnStartUp

 The following values are allowed:

    OnStartup   The Gateway DLT Daemon tries to connect to the Passive DLT Daemon
                immediately after the Gateway DLT Daemon is started.
    OnDemand    The Gateway DLT Daemon tries to connect to the Passive DLT Daemon
                when it receives a connection request.

### Timeout

Stop connecting to passive node, if not successful after 10 retries.

After <Timeout> of retries, the connection to passive Node is marked as DISABLED.

It means there is no any retry anymore.

Set to 0 for endless retry.

    Default: 10

### SendControl

Send following control messages after connection is established. Optional.

    Default: disabled

Supported Control messages:

    DLT_SERVICE_ID_GET_LOG_INFO                    0x03
    DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL           0x04
    DLT_SERVICE_ID_GET_SOFTWARE_VERSION            0x13

### SendSerialHeader

Send Serial Header with control messages. Value in dlt.conf(5) is used as default if not specified.

    Default: disabled

### SendPeriodicControl

Send following control messages periodically.

    Default: disabled

Format:

    control:interval[in seconds]

# AUTHOR

Thanh Bui Nguyen Quoc (thanh.buinguyenquoc (at) vn (dot) bosch (dot) vn)

# COPYRIGHT

Copyright (C) 2020 Advanced Driver Information Technology, Bosch and DENSO. License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.

# BUGS

See Github issue: <https://github.com/COVESA/dlt-daemon/issues>

# SEE ALSO

**dlt.conf(5)**, **dlt-daemon(1)**
