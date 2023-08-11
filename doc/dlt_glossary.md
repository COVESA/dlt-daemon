# Glossary

Throughout the documentation specific terms are used. Those are defined below.

Back to [README.md](../README.md)

Term  | Definition
----- | ----
Application ID | Application Identifier is a unique identifier for an application registered at the DLT Daemon. It is defined as 8bit ASCII character. e.g. "APP1". Each Application can have several sub-components each having a unique context ID
Context ID	| This is a user defined ID to group log and trace messages produced by an application. Each Application ID can own several Context IDs and the Context IDs shall be unique within an Application ID. The identification of the source of a log and trace message is done with a pair of Application ID and Context ID. It is composed by four 8 bit ASCII characters.
Control Message	| A control message is send by a connected client application (e.g. Log Viewer) to the DLT Daemon that includes an action request. E.g. change the Log level of a certain application).
DLT Daemon | The DLT Daemon is the central component which receives all logs and traces from the DLT user applications. The DLT Daemon forwards all logs and traces to a connected DLT client (e.g. Log Viewer) or stores them optionally in a file on the target.
DLT Library | Provides applications (esp. DLT users) with an API to produce DLT messages and to handle DLT Control Messages accordingly.
DLT User | A DLT User is a type of application that produces log messages. It typically uses the DLT library to produce the messages and resembles an ECU.
DLT Viewer | The DLT Viewer is the COVESA Log Viewer implementation. It is a Qt-based desktop application able to run on Windows and Linux operating systems. Further information and source code can be found here: https://github.com/COVESA/dlt-viewer
Gateway DLT Daemon | In a Multi-Node system, the DLT Daemon running on the Node directly connected to a Log Viewer is called Gateway DLT Daemon (if configured as Gateway). It forwards log messages from Passive DLT Daemons to Log Viewers and command/control messages from Log Viewer(s) to Passive DLT Daemon(s).
Injection Message | An injection message is a control message for a specific DLT application.
Log Consumer | A log consumer is an application connected to the DLT Daemon (DLT Client) that receives log messages and stores (e.g. Logstorage) or displays them (e.g. Log Viewer). A log consumer can run on the same operating system or on a remote host pc.
Log Level | A log level defines a classification for the severity grade of a log message.
Log Viewer | A client application or tool used to view log information on a host pc. DLT-viewer is the supported Log Viewers.
Multi-Node System | A system with more than one node.
Node | A node represents an operating system, e.g. Linux, AUTOSAR or a container having its own DLT Daemon instance. Only one DLT Daemon should run on a node. In the context of DLT, Node and ECU describe the same thing.
Node Identifier	| Unique Identifier of a node (node ID). In DLT, the node ID is defined by __EcuID__ set in _dlt.conf_ configuration file.
Passive DLT Daemon | A passive DLT Daemon runs on a node without direct connection to a Log Viewer. All communication between a passive DLT Daemon and a Log Viewer has to be send via the Gateway DLT Daemon.
Trace Status | The trace status provides information if a trace message should be send. Supported States are ON or OFF
Verbose / Non-Verbose Mode | The DLT supports Verbose and Non-Verbose Mode. In _Verbose_ mode, all logging data including a type description is provided within the payload. Furthermore, Application ID and Context ID are transferred as part of the message header. In _Non-Verbose_ mode, description about the sender (Application ID, Context ID) as well as static strings and data description is not part of the payload. Instead, this information is stored in a file that needs to be parsed by a Log Viewer. The log message contains a unique message ID instead which allows a mapping between received log message and information stored in the file.