# DLT MultiNode

Back to [README.md](../README.md)

## Overview

MultiNode allows to connect DLT Daemons running on different operating systems,
e.g. in a virtualized environment. The central component is the Gateway DLT
Daemon which connects external DLT Clients, like the DLT Viewer running on a
host computer with Passive DLT Daemons running on nodes without a physical
connection to external DLT clients. All communication between passive nodes and
DLT Viewer has to be sent via the Gateway node. The Gateway node forwards log
messages coming from passive nodes to all connected DLT clients. The Gateway DLT
Daemon also forwards command and control requests coming from DLT clients to the
corresponding passive node.

![alt text](images/dlt-multinode.png "DLT MultiNode")

## Precondition

The dlt.conf configuration file which is read by each DLT Daemon on start-up
contains an entry to specify the ECU identifier (node identifier). It has to be
ensured, that **each DLT Daemon in the System has a unique ECU** identifier
specified. The ECU identifier is included in every DLT Message and is used to
distinguish if a DLT message has to be forwarded to a passive node or handled by
the Gateway DLT Daemon itself.

## Configuration

The dlt.conf configuration file provides an option to enable the Gateway
functionality of a DLT Daemon. The default setting is 0 (Off), which means the
Gateway functionality is not available.

```
# Enable Gateway mode (Default: 0)
GatewayMode = 1
```

### Gateway Configuration File

The MultiNode configuration file has to be loaded by the Gateway DLT Daemon
during startup.

```
[PassiveNode1]
; IP Address. (Mandatory)
IPaddress = 192.168.2.32
; TCP port. Default 3490 is used if no port is specified
Port = 3495
; Passive node ECU identifier. (Mandatory)
EcuID = ECU2
; Connection to passive node only on demand. Default ‘OnStartup’ if not specified
Connect = OnDemand
; timeout in seconds
Timeout = 10
; Send following control messages after connection is established
SendControl=0x03, 0x13
; Send SerialHeader with control messages. Value in dlt.conf is used
; as default if not specified
SendSerialHeader=1
```

The configuration file is written in an INI file format and contains information
about different connected passive nodes. Each passive node’s connection
parameters are specified in a unique numbered separate section
([PassiveNode{1,2, …N}]). Because TCP is the only supported communication
channel, the IPaddress and Port of the Passive DLT Daemon has to be specified.

With the Connect property it is possible to specify when the Gateway DLT Daemon
shall connect to the passive node. The following values are allowed:
- OnStartup - The Gateway DLT Daemon tries to connect to the Passive DLT Daemon
  immediately after the Gateway DLT Daemon is started.
- OnDemand - The Gateway DLT Daemon tries to connect to the Passive DLT Daemon
  when it receives a connection request.

The Timeout property specifies the time after which the Gateway DLT Daemon stops
connecting attempts to a Passive DLT Daemon. If the connection is not
established within the specified time, the Gateway DLT Daemon gives up
connecting attempts and writes an error messages to its internal log. The
following control messages are supported to be send to a passive node
automatically after connection is established:
- 0x03: Get Log Info
- 0x13: Get Software Version

## Using DLT MultiNode

```
Usage: dlt-passive-node-ctrl [options]
Send a trigger to DLT daemon to (dis)connect a passive node or get current passive node status.

Options:
  -c         Connection status (1 - connect, 0 - disconnect)
  -h         Usage
  -n         passive Node identifier (e.g. ECU2)
  -s         Show passive node(s) connection status
  -t         Specify connection timeout (Default: 10s)
  -v         Set verbose flag (Default:0)
```
