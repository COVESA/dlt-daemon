# DLT Offline Logstorage Design Specification

Back to [README.md](../../README.md)

## Executive Summary

This document describes DLT Offline Logstorage feature. It includes software
architecture and design. DLT Offline Logstorage offers the possibility to store
application logs on different storage devices by just plug-in the storage device
into the target. The configuration of what log messages will be stored is read
from a special configuration file which has to be installed on the storage
device before inserting into the target.

Usage is available in [dlt_offline_logstorage.md](../dlt_offline_logstorage.md).

## Scope

This document is written to understand design and implementation, and should
help to use DLT Logstorage feature in the correct way.

## Requirements

## Design Considerations

The following points are considered for the design of Offline Logstorage:

**Reusability**

The existing implementation of the DLT shall be reused with minor modification
and adaptation of the existing DLT design.

**Portability**

It shall be ensured that while updating the DLT source code care is taken not to
alter the platform dependencies of the existing DLT; thereby maintaining the
portability design in DLT.

**API consistency**

The interface between DLT Daemon, DLT Client and DLT applications shall not be
changed.

## Architecture

This chapter gives an overview of Offline Logstorage architecture inside DLT.
More details of design itself are given in later session.

### Use Case View

#### Configuration

Use Case  | Configuration
----- | ----
Responsibilities | *DLT Daemon*: Apply log level filter, Change Offline Logstorage configuration, Store log message to device, *System Monitoring Instance*: Informs DLT Daemon about storage device
Trigger | Storage device is plugged into the system
Pre-Condition | Storage device contains a configuration file
Result | DLT Daemon is configured to store logs of configured applications into files on storage device
Exceptions | -
Variants | -

#### Log to storage device

Use Case  | Log to storage device
----- | ----
Responsibilities | *Application*: Log message, *DLT Daemon*: Apply log level filter, Store log message to device
Trigger | Application starts logging message through DLT interface
Pre-Condition | Storage device inserted and Logstorage configuration done
Result | Log message is stored in file on storage device
Exceptions | -
Variants | -

#### Log to FLASH storage device

Use Case  | Log to FLASH storage device
----- | ----
Responsibilities | *Application*: Log message, *DLT Daemon*: Apply log level filter, Store log messages in RAM, Store log message to device on a specific event
Trigger | Application starts logging message through DLT interface
Pre-Condition | DLT Daemon is configured to use a folder on the FLASH storage as Offline Logstorage device
Result | Log message is stored in file on storage device
Exceptions | -
Variants | -

#### Update Log Level in Passive Node

Use Case  | Update Log Level in Passive Node
----- | ----
Responsibilities | *User* Creates DLT Logstorage Configuration file, Stores DLT Logstorage Configuration file on a storage device,  Notifies DLT Daemon to attach DLT Logstorage device (e.g. via HMI), *DLT Daemon*: Validates DLT Logstorage Configuration File, Sends Log Level change request to Application via connected passive Node DLT Daemon Application (DLT Library), Activates the received Log Level
Trigger | Notification send to DLT Daemon (e.g. via HMI) to activate a DLT Logstorage device
Pre-Condition | Storage device contains a configuration file, Configured storage location is writeable by DLT Daemon
Result | Application has updated the Log Level
Exceptions | DLT Logstorage Configuration file is missing or invalid, Configured storage location is not writable by DLT Daemon, The configured Application is not running, The configured node (ECU Identifier) is not connected
Variants | -

#### Store Non-Verbose Log message

Use Case  | Store Non-Verbose Log Message
----- | ----
Responsibilities | *Application*: Sends DLT Non-Verbose Log Message, *DLT Daemon*: Receives DLT Non-Verbose Log Message, Filters DLT Non-Verbose Log Message based on ECU identifier, Writes DLT Non-Verbose Log Message into configured log file
Trigger | Application reaches a state that results in sending a Log Message
Pre-Condition | Log Level is configured, Storage location is writable by DLT Daemon
Result | DLT Non-Verbose Log Message stored in log file
Exceptions | Received Log Message is filtered (no valid filter configuration found)
Variants | -

### External View
DLT consists of the DLT Daemon which acts as central component of DLT. It is
responsible for receiving logs from applications and forwarding them to
connected DLT clients. In case of Logstorage, the logs are filtered and stored
directly into files on storage devices depending on given configuration.

![External view](images/logstorage/dlt_logstorage_external.png "External view")

The DLT Daemon furthermore provides an interface to send control messages to the
DLT Daemon  or connected applications. This interface is used by DLT Offline
Logstorage. When the DLT Offline Logstorage Control Application is triggered due
to an attached storage device or executed by another application, it informs the
DLT Daemon about attached storage devices. The DLT Daemon will then read the
configuration file and start storing log messages to the device. The system has
to make sure that the device is mounted as read-write; otherwise no logs will be
stored onto the device. When a used device is removed, the Logstorage Control
application is triggered again and informs the DLT Daemon about the removal. The
DLT Daemon will stop storing logs onto the removed device. The extended
functionality is that the DLT Logstorage Configuration file might contain a
specific Log Level configuration for DLT Applications running on a Passive Node.
In this case, the Gateway DLT Daemon has to send a Log Level Change Control
message to the connected Passive DLT Daemon. The Passive DLT Daemon afterwards
forwards this request to the DLT Application itself. The DLT Application finally
configures its Log Level. Log messages sent by applications running on the
Passive Node are forwarded to the Gateway DLT Daemon via Passive DLT Daemon and
finally stored inside a DLT Logstorage Storage Device.

## Interface Description

There are no changes in the interface to DLT applications, but the description
of configuration file for Logstorage logging is given here. Internal design is
explained in later session.

### Logstorage configuration file

The Logstorage configuration file will provide the user with configurable
options to setup device settings, log filters and storage locations of the log
messages. To easily implement applications for creating project specific
configuration files, the used file format is INI. An example configuration file
describing all configuration parameters can be found below. Mandatory name of
the configuration file is: ***dlt_logstorage.conf***.

```
[FILTER1]
LogAppName=APP1
ContextName=CON1,CON2
LogLevel=DLT_LOG_INFO
File=App
FileSize=10000
NOFiles=10

[FILTER2]
LogAppName=TEST
ContextName=.*
LogLevel=DLT_LOG_ERROR
File=Test
FileSize=250000
NOFiles=5
EcuID=ECU2
SyncBehavior=ON_SPECIFIC_SIZE
SpecificSize=5000

[FILTER3]
LogAppName=TEST
ContextName=.*
LogLevel=DLT_LOG_ERROR
File=Test
FileSize=250000
NOFiles=5
SyncBehavior=ON_FILE_SIZE,ON_DEMAND
EcuID=ECU1
```

As seen in example configuration file, a filter configuration consists of
- A **unique name** of the filter (for a certain storage device)
- **LogAppName**: Name of application to store logs from. Multiple applications
  can be separated by ",". ".*" denotes all applications
- **ContextName**: Name or names of contexts to store logs from. Multiple
  contexts can be separated by ",". ".*" denotes all contexts of the application
- **LogLevel**: Define log level
- **File**: Base name of the created files that containing the logs.
- **FileSize**: File size in bytes
- **NOFiles**: Number of created files before oldest is deleted and a new one is
  created. The numbering will continue. E.g. If 10 files have to be created and
  the first is deleted, the next one is *<File name>_011_<timestamp>.dlt*
- **EcuID (optional)**: Only handle messages coming from the specified ECU.
- **SyncBehavior (optional)**: Sync behavior of the Filter configuration. It is
  possible to add a list of Sync strategies.
- **SpecificSize (optional)**: Size in bytes when messages are synced to files
  when SyncBehavior is set to ON\_SPECIFIC\_SIZE

**Note**: Not allowed is the combination of wildcards for LogAppName and
ContextName if EcuID is not specified.

```
[NON-VERBOSE-STORAGE-FILTER1]
EcuID=PASV
File=scc
FileSize=50000
NOFiles=5
[NON-VERBOSE-LOGLEVEL-CTRL1]
LogAppName=LOG
ContextName=TEST
LogLevel=DLT_LOG_DEBUG
EcuID=PASV
[NON-VERBOSE-LOGLEVEL-CTRL2]
LogAppName=.*
ContextName=.*
LogLevel=DLT_LOG_WARN
EcuID=PASV
```

In case of Non-Verbose Mode, two different configuration sections exist.
*[NON-VERBOSE-STORAGE-FILTER1]* describes where log messages of a Non-Verbose
passive Node (here: PASV) are stored. Furthermore, a configuration section to
control the log level of Non-Verbose applications exists. Either a log level for
a specific application/context can be defined (*[NON-VERBOSE=LOGLEVEL-CTRL1]*)
or the log level of all registered applications can be changed
(*[NON-VERBOSE=LOGLEVEL-CTRL2]*).

### Offline Logstorage Control messages

The DLT Daemon can be controlled with a standard set of control messages. To
control Offline Logstorage, the following control messages are added.

- **DLT_SERVICE_ID_OFFLINE_LOGSTORAGE**
  Send request to DLT Daemon to either enable or disable a Logstorage device
  defined by its mount path. Conntection Types are: 0-Disconnect, 1-Connect, and
  2-Sync caches
    - Parameter:

      ```
      typedef struct
      {
          uint32_t service_id;                  /**< service ID */
          char mount_point[DLT_MOUNT_PATH_MAX]; /**< storage device mount point */
          uint8_t connection_type;              /**< connection status of the connected device connected/disconnected */
          char comid[DLT_ID_SIZE];              /**< communication interface */
      } DLT_PACKED DltServiceOfflineLogstorage;
      ```

## Design

It is not intended to describe the design of DLT itself before explaining the
DLT Offline Logstorage feature. Therefore it is necessary for the reader to have
good understanding of DLT's internal design to understand the design of DLT
Offline Logstorage.

### Structural View

![DLT Offline Logstorage inside DLT Daemon](images/logstorage/dlt_logstorage_structural.png "DLT Offline Logstorage inside DLT Daemon")

DLT Daemon consists mainly of two parts: the LogProducer and a set of
LogConsumers. The LogProducer inside DLT acts as backend to libdlt.so which can
be seen as the implementation of the LogProducer frontend. Log messages are
filtered inside the LogProducer frontend based on the configured log level of
the context. The LogProducer backend inside DLT Daemon maintains tables of
connected applications and contexts to be able to send messages to the connected
DLT applications. This is used by DLT Offline Storage to change the log levels
of applications. A LogConsumer inside DLT Daemon forwards log messages to
connected DLT clients. When a DLT client connects to the Daemon, the DLT Daemon
opens the corresponding interface. In case of a connected DLT Viewer, a
TcpLogConsumer (socket connection over TCP/IP) or a SerialLogConsumer is used.
Each LogConsumer maintains its own ConsumerInfoTable. In case of DLT Offline
Logstorage, no external DLT Client is connected. Then DLT Daemon acts as DLT
Client by itself. This is done by implementing DLT Offline Logstorage as another
LogConsumer. For each LogConsumer (Storage Device) an
OfflineLogstorageConsumerInfoTable with following content is maintained:
- **Log storage file path**: Path to file system location for storing sets of
  log files for that configuration.
- **File size**: The maximum size in bytes which each file in the set must reach
  before a new file is created.
- **Total number of log files**: The total number of files to be created for the
  set of files. If this number is reached, the first file is deleted and a new
  file is created.
- **Log level**: The log level which a message needs to have to be stored
- **Application and Context Identifier**: Application and Context names to know
  from which applications and contexts logs shall be stored.

#### Application and context information

The DLT Daemon maintains a list of applications and a list of contexts that are
running on the same ECU. To be able to set and reset log levels of applications
running on passive nodes, the (Gateway) DLT Daemon also has to store information
about applications and contexts running on passive nodes. This is currently not
possible.

![Application and Context Information Handling](images/logstorage/dlt_logstorage_ap_ct_hdl.png "Application and Context Information Handling")

The figure above depicts the updated handling of registered application and
context information. The Dlt Daemon will have a reference to a
DltDaemonApplicationList and DltDaemonContextList, instead of handling the data
itself (pointer to DltDaemonApplication, number of registered applications and
ECU identifier the registered users running on). Furthermore, the APIs to add,
delete, find, etc. applications are updated in a way that the dependency to
DltDaemon structure is removed. As a result, it is possible to instantiate the
application list multiple times (one time for each ECU in the system) without
changing the internal implementation of the list handling.

### Dynamic View

#### External components

##### Interaction of Different Modules for Storing Log Messages

##### Start Storing Log Messages

![Interaction of different modules to start storing messages](images/logstorage/dlt_logstorage_start_str_msg.png "Interaction of different modules to start storing messages")

The above diagram shows the interaction of different modules for triggering
storing of log messages in DLT. The user (tester in sequence diagram) inserts
the external device with a configuration file present in it. On plug-in of that
device, the udev Daemon mounts the device to specific DLT Offline Logstorage
location (/tmp/dltlogs/dltlogsdev\<devnumber\>) and subsequently executes DLT
Logstorage Control Application with device number and connection status (in this
case *CONNECTED*) to trigger the DLT Daemon to start storing log messages to the
mounted device. The trigger is sent to the DLT Daemon from the DLT Logstorage
Control Application as a Service Request with the service ID of the
*DLT\_LOG\_STORAGE*. The DLT Daemon reads the configuration file from the device
and updates its OfflineLogstorageConsumerInfoTable entries. Then it calculates
the actual log level to be set at the DLT Application context by calculating the
union of log levels for that context from DLT Clients and all Log Storage
OfflineLogstorageConsumerInfoTables. This means, the lowest needed log level
will be set in the Application. E.g. there are two filter configuration set
storing the same log messages, but one of the configurations has DLT\_LOG\_FATAL,
the other DLT\_LOG\_WARN set. To ensure, that the second filter receives log
messages, DLT\_LOG\_WARN needs to be set in the application as log level. Next,
the DLT Daemon sends a DLT\_USER\_Command to all corresponding Applications to
update the log levels of their contexts. Upon receiving the set log level
command the DLT Application re-adjusts its log level and continues to log
messages with newly set log level. When Log messages are now received at the DLT
Daemon from DLT Applications, the DLT forwards the log messages to Offline
Logstorage and it filters the received messages based on the configuration in
its OfflineLogstorageConsumerInfoTable. Finally, the messages are written into
log files on the mounted device.

##### Stop Storing of Log Messages

![Interaction of different modules to stop storing log messages](images/logstorage/dlt_logstorage_stop_str_msg.png "Interaction of different modules to stop storing log messages")

The above diagram shows the interaction of different modules to stop storing of
log messages in DLT using DLT Offline Logstorage feature. The user unplugs the
external device with a configuration file present in it. On unplug of such
device Udev Daemon executes the DLT Logstorage Control Application with device
number and connection status (in this case *DISCONNECTED*) to stop DLT Offline
Logstorage at DLT Daemon. The DLT Logstorage Control Application then sends a
Service Request to the DLT Daemon to trigger the DLT Daemon to stop logging to
the device to be unplugged. The DLT Daemon updates the
OfflineLogstorageConsumerInfoTable entries and also calculates a new log level
of all Application contexts which are affected by this change. Afterwards, the
DLT Daemon sends a command to all affected applications to update the log level
of their respective contexts. Upon receiving the set log level command, the DLT
application re-adjusts its log level and continues to log messages with newly
set log level. Finally, the device is un-mounted by the Udev Daemon.

#### MultiNode handling

##### Retrieve information about running application on passive node

![Request application status](images/logstorage/dlt_logstorage_get_log_info.png "Request application status")

The Gateway DLT Daemon needs actual information about current status of DLT
applications running on Passive Node. It might be that applications started or
stopped during system run time, so the GET\_LOG\_INFO request needs to be sent
periodically, because new applications/contexts might be registered in the
meanwhile.

##### DLT Gateway Configuration

To control the frequency of Passive Node control requests, another entry is
added to a Passive Node configuration.

```
[PassiveNode1]
...
; Send following control messages periodically (<control:interval[in seconds]>)
SendPeriodicControl=0x03:5,0x05:10
```
By using *SendPeriodicControl* the user is able to control what control messages
shall be sent in a specific interval. If multiple control messages shall be sent
periodically, a comma separated list of commands can be specified.

##### Store information about passive node applications

Below diagram depicts update of registered applications and contexts of a
passive node DLT Daemon. Triggered by a GET\_LOG\_INFO request send by the
Gateway DLT Daemon, the passive node DLT Daemon sends a GET\_LOG\_INFO response.
The GET\_LOG\_INFO response does not contain any information about stopped
applications. Because of that, to avoid too much maintenance on Gateway DLT
Daemon, the passive node application and context list is cleared at first. While
parsing the GET\_LOG\_INFO response, applications and contexts are added to the
list again. As a result, the Gateway DLT Daemon always holds up-to-date
information about running DLT users on the passive node.

![Internal update of passive node users](images/logstorage/dlt_logstorage_update_passive.png "Internal update of passive node users")

##### Update Log Level of application running on Passive Node

![Update log level](images/logstorage/dlt_logstorage_update_log_level.png "Update log level")

Triggered by the DLT Logstorage Control application, the Gateway DLT Daemon
configures a new Logstorage device. When a Log Level change of a DLT Application
running on a passive node is requested, the Gateway DLT Daemon sends a control
request to the Passive DLT Daemon containing the information given in the DLT
Logstorage Configuration file. Finally, the request is forwarded to the actual
DLT Application that updates its Log Level.

##### Store DLT Non-Verbose message in a DLT Logstorage device

![Store DLT Non-Verbose message](images/logstorage/dlt_logstorage_str_non-verbose_msg.png "Store DLT Non-Verbose message")

A DLT message is sent by a DLT Application running on a Passive Node to the
Passive DLT Daemon running on the same node. Because the Gateway DLT Daemon is
connected as DLT Client to the Passive DLT Daemon, the Passive DLT Daemon
forwards the log message to the Gateway DLT Daemon as usual. The Gateway DLT
Daemon itself filters the incoming DLT Non-Verbose message by ECU Identifier
(EcuID)  and finally stores the message in a log file on the file system.

#### Internal components

![DLT Offline Logstorage -Sequence Diagram](images/logstorage/dlt_logstorage_sequence.png "DLT Offline Logstorage -Sequence Diagram")

The above depicted sequence diagram shows the different stages involved in DLT
Offline Logstorage. The detailed design is explained through following sections
below.

##### Configuring DLT and Setup Log Levels of Applications

![DLT Offline Logstorage - Startup and Configure](images/logstorage/dlt_logstorage_startup_conf.png "DLT Offline Logstorage - Startup and Configure")

This task involves reading of DLT Offline Logstorage configuration file and
updating the OfflineLogstorageConsumerInfoTable (context information table) with
configuration settings. The configuration files are located at preconfigured
locations or on external storage devices. The DLT Daemon reads the configuration
file from that location, adds entries into the
OfflineLogstorageConsumerInfoTable based on the settings provided in the
configuration file. As shown in the above sequence diagram the OfflineLogstorage
Consumer reads configuration (read\_configuration) from Storage Device and
updates the OfflineLogstorageConsumerInfoTable (add\_update\_table\_entry).
For each OfflineLogstorage Consumer (set of log files or device), an
OfflineLogstorageConsumerInfoTable is maintained. When a DLT application context
is registered, the LogProducer obtains the log level of individual application
context from each OfflineLogstorageConsumerInfoTable, makes a union of it and
sets this level as the log level for the context in the ContextTable. This log
level is in turn sent to the DLT application to ensure that the DLT application
sends all logs to OfflineLogstorage. Otherwise relevant log messages might be
filtered in DLT user library. OfflineLogstorage Consumer will obtain all the
stored configuration settings of each application context from
OfflineLogstorageConsumerInfoTable during logging of message from application.
This set log level will be used for filtering at the LogProducer (inside
libdlt.so).

**Store filter configuration**

This section describes more in detail what happens during
*add_update_table_entry()*. Internally, a hash table is set up using the
information from the parsed *dlt_logstorage.conf* file. This file contains
different kinds of data:
- Message information
    - Application ID (LogAppName)
    - Context ID (ContextName)
    - Log level (LogLevel)
    - Ecu ID (EcuID)
- Storage information
    - Base file name (File)
    - File size (FileSize)
    - Number of files (NOFiles)
    - Synchronization behavior (SyncBehavior)

The basic idea is to describe a mapping between Message information (used as key
and filter criteria) and storage information (used as data). Furthermore, the
message information is used as filter criteria to decide if a certain message
has to be stored or not.

##### Setup Application Context Log Level

![Setup Application Context Log Level](images/logstorage/dlt_logstorage_setup_ap_ct_log_level.png "Setup Application Context Log Level")

When an application using DLT is started, it registers itself at the DLT daemon.
Afterwards, it registers all its contexts. In case of enabled DLT Offline
Logstorage, the DLT Daemon checks if an entry of the registered Application in
one or multiple of the OfflineLogstorageConsumerInfoTables exits. If one or more
entries for the context are found, a union of all configured log levels is
created. This means, the DLT Daemon chooses the lowest configured log level.
Finally the DLT Daemon updates the log level of the registered context to this
log level by calling *set_user_log_level()*. This ensures that logs of a
configured log level are not filtered within the LogProducer (libdlt.so) and
forwarded to the OfflineLogstorage Consumer.

##### Storing Log Messages to Devices

![Store Log Information](images/logstorage/dlt_logstorage_str_msg.png "Store Log Information")

Storing of log messages to devices involves following tasks:
- **Filtering of Log Information**: When DLT applications log information, the
  log information is filtered at the LogProducer (inside libdlt.so) based on the
  log level the application is configured to use. This is then forwarded by the
  LogProducer (receiver inside DLT Daemon) to the OfflineLogstorage Consumer. On
  receiving the log information, the OfflineLogstorage Consumer gets the
  configured, application name, context name and log level from its
  OfflineLogstorageConsumerInfoTable and filters the log information before
  storing into the sets of files on the Storage Device. If the configured log
  level obtained from OfflineLogstorageConsumerInfoTable is lesser than the log
  level of message and application or context name are not present in the
  OfflineLogstorageConsumerInfoTable, the OfflineLogstorage Consumer discards
  the log message. *Note*: In case multiple devices, the OfflineLogstorage
  Consumer obtains the log level from all OfflineLogstorageConsumerInfoTables.
  More details on the implementation can be obtained below.
- **Storing Log Information**: After filtering the relevant log messages, the
  OfflineLogstorage Consumer will store the log message in a file on the Storage
  Device.

**Filtering of incoming log messages**

To filter a log message based on information given in the filter configuration,
a linked list is used. A member *key_list* consists of a combination of
application ID and context ID. Since DLT Offline Logstorage supports wildcards
and lists of IDs, the following cases have to be considered:

If Ecu Identifier is not specified, Filter configuration contains
1. One application ID (App1) and one context ID (Ctx1): Key = ":App1:Ctx1"
2. One application ID (App1), wildcard for context ID: Key = ":App1:"
3. Wildcard of application ID, One context ID (Ctx1): Key = "::Ctx1"
4. Wildcard of application ID, list of context IDs (Ctx1,Ctx2): Keys="Ctx1","
  Ctx2" and the other way around
5. List of application (App1, App2) and context IDs (Ctx1, Ctx2): all
  combinations of application ID and context ID are possible.
  Keys = ":App1:Ctx1", ":App1:Ctx2", ":App2:Ctx1", ":App2,Ctx2"
6. Both application ID and context ID is not allowed to be set to wildcard.

Data of a key value pair contains of all information given in
OfflineLogstorageConsumerInfoTable - apart from application ID and context ID.
When a message arrives at the filter, three checks have to be done. It has to be
checked if:
1. The combination of application and context ID is a valid key
2. The application ID is a valid key
3. The context ID is a valid key

If one or more keys are valid the message is stored in the file of the
corresponding filter. If not, the incoming message will be ignored. In case of a
configured Ecu Identifier (ECU1), the following Keys are generated:
1. One application ID (App1) and one context ID (Ctx1): Key = "ECU1:App1:Ctx1"
2. One application ID (App1), wildcard for context ID: Key = "ECU1:App1:"
3. Wildcard of application ID, One context ID (Ctx1): Key = "ECU1::Ctx1"
4. Wildcard of application ID, list of context IDs (Ctx1,Ctx2):
  Keys=" ECU1::Ctx1"," ECU1::Ctx2" and the other way around
5. List of application (App1, App2) and context IDs (Ctx1, Ctx2): all
  combinations of application ID and context ID are possible.
  Keys = "ECU1:App1:Ctx1", "ECU1:App1:Ctx2", "ECU1:App2:Ctx1", "ECU1:App2:Ctx2"
6. If application ID and context ID is set to wildcard, the following key is
  generated: "ECU1::"
7. If application ID and context ID is not present in configuration file,
  the following key is generated: "ECU1::"

The diagram below shows the steps to be done to find files to store an incoming log message.

![Steps Log Message Filtering](images/logstorage/dlt_logstorage_msg_filter.png "Steps Log Message Filtering")

When an incoming message is received, it needs to be checked first if the
message is sent in Verbose or Non-Verbose mode. Depending on the mode, different
keys are created to search for valid filter configurations. In case of
Non-Verbose mode, only the following key can be created; "EcuID::", because a
Non-Verbose message usually does not contain any information about Application
and Context ID. In case of Verbose mode, all valid combinations of EcuID,
Application ID and Context ID are generated. The generated keys are used as
input to find valid configuration that contain information about log level, file
location etc. Using the filter information found in the configuration, the log
message gets stored in a file.

### DLT Offline Logstorage Cache

Usually, the DLT Offline Logstorage is used to store log information on an
attached external device as described above. Another use case is to store logs
to an internal storage device as described in Use Case View. In this case,
writing every log message to the storage device must be avoided since FLASH
storage devices have a limited number of write cycles. Therefore, the log
messages have to be buffered and synced to disk based on a certain strategy.

The following strategies exist:
1. **Write every log message (ON_MSG)**: Each log message is persistently
  written to the storage device.
2. **Write on close (ON_DAEMON_EXIT)**: Write data to disk before the DLT Daemon
  is stopped.
3. **Write on demand (ON_DEMAND)**: Write data on disk whenever the write is
  triggered from an external application.
4. **Write on file size reached (ON_FILE_SIZE)**: The log file size for a
  specific filter is specified in the DLT Logstorage configuration file. A
  memory region with the same size will be allocated and filled with data.
  Whenever the buffer is filled with data, this data is written into a log file.
5. **Write after specific size (ON_SPECIFIC_SIZE)**: This is basically the same
  as (4), but writes data into a log file after a the incoming data size reached
  the specified "write after" data size.
6. **A combination of the above scenarios**: Combinations with ON\_MSG or
   combination of ON\_FILE\_SIZE and ON\_SPECIFIC\_SIZE are not allowed.

Write every log message is the standard DLT Offline Logstorage strategy which
has been discussed in detail above. All other strategies base on having a log
message cache available.

#### Write on close strategy

In this scenario, all log messages are stored in a ring buffer with the size
specified in *FileSize*. When the DLT Daemon is terminated, all logs that belong
to filter configurations having the *ON_DAEMON_EXIT* sync strategy enabled are
synced to disk. This is shown in diagram below. The daemon receives a
termination signal which is handled in the installed signal handler. The main
event loop is stopped and the DLT Daemon cleans up all internal data structures.
It calls the *logstorage_cleanup()* function which itself cleans up all active
Logstorage devices. In this stage, every filter configurations configured with
the *ON_DAEMON_EXIT* flag will sync its cached log messages to disk in a new
log file.

![Sync on daemon exit](images/logstorage/dlt_logstorage_sync_on_daemon_exit.png "Sync on daemon exit")

#### Write on demand strategy

In this scenario, all log messages are stored in a ring buffer with the size
specified in *FileSize*. When the DLT Logstorage Control Application sends a
trigger, all logs that belong to filter configurations having the *ON\_DEMAND*
sync strategy enabled are synced to disk. This is shown in diagram below. The
DLT Logstorage Control Application will send a DLT Daemon control command that
is forwarded by the DLT Daemon to the Offline Logstorage. Two different exist:
If the command is sent without a mount point as parameter, all Logstorage caches
configured with the on demand sync strategy are synced to disk. If the command
is sent with a mount point, only the Logstorage caches of the corresponding
Logstorage device are synced to disk. In case the mount point is not valid, the
DLT Daemon returns an error response to the DLT Logstorage Control application.

![Sync on demand](images/logstorage/dlt_logstorage_sync_on_demand.png "Sync on demand")

#### Offline Logstorage Ring buffer cache

Every Offline Logstorage filter configuration will have its own buffer with a
specific size. If the buffer is full and the content was not synced to disk
before, older log messages will be overwritten from the beginning as shown in
diagram below.

![DLT Offline Logstorage Ring buffer](images/logstorage/dlt_logstorage_ring_buffer.png "DLT Offline Logstorage Ring buffer")

#### Strategy initialization

The user can specify which strategies he wants to add to a filter configuration.
Here it must be ensured that *ON_MSG* cannot be combined with any strategy
based on a Logstorage cache.

For each filter configuration, depend on the value `File`, a list of newest file
which is keeping the data of newest file name and wrap-around id for corresponding
`File`.

```
struct DltNewestFileName
{
    char *file_name;    /* The unique name of file in whole a dlt_logstorage.conf */
    char *newest_file;  /* The real newest name of file which is associated with filename.*/
    unsigned int wrap_id;   /* Identifier of wrap around happened for this file_name */
    DltNewestFileName *next; /* Pointer to next */
};
```

#### Storage preparation based on strategy

For each Logstorage configuration, the storage preparation step is executed
once.

In case of *ON\_MSG" strategy, the file on the storage device is created.
The newest file info must be updated to the working file info after preparation.


In case of cache based strategies, the file on the storage device is created and
the Logstorage cache (ring buffer) is initialized.

Following diagram shows the
difference between caching strategy initialization and non-caching *ON_MSG*
strategy.

![Storage preparation](images/logstorage/dlt_logstorage_preparation.png "Storage preparation")

#### Storing log messages based on strategy

The default behavior of Offline Logstorage writes every log message directly
into a file (*ON_MSG*). In case of caching strategies, the log message is
stored into a ring buffer (refer
[Offline Logstorage Ring buffer cache](#Offline-Logstorage-Ring-buffer-cache)).
Following diagram shows the difference between caching strategy initialization
and non-caching *ON_MSG* strategy.

![Write log message](images/logstorage/dlt_logstorage_wrt_msg.png "Write log message")

#### Synchronization based on strategy

The default behavior of Offline Logstorage syncs the file directly after a log
message is written into the file (*ON_MSG*). Before synchronization to file,
the working log file is compared against the log files to ensure that
the log data is written to the newest file. If they are different each other,
the working log file is closed and try to open the newest one for writing.
fsync() is called before fclose() instead of on every message because invoking
it per message basis has too much impact on performance.

In case of other log strategies,
the ring buffer is synced to a file depending on the configured sync strategies.
The sync function is called from different places in the Logstorage code.
Therefore, the current situation has to be given as parameter. If the filter
configuration is configured with a specific strategy the ring buffer will be
flushed to a file. If the certain strategy is not configured, the function
returns without actually syncing the ring buffer. The action of update log files
is always performed to ensure that the newest file is used to synchronize the data in ring buffer.
The newest file info must be updated to the working file info after sync.

![Sync messages](images/logstorage/dlt_logstorage_sync_msg.png "Sync messages")

Syncing the ring buffer includes additional internal steps as shown in diagram
below.

![Sync ring buffer](images/logstorage/dlt_logstorage_sync_ring_buffer.png "Sync ring buffer")

## Security

The current implementation of DLT Offline Logstorage raises the following
security related problems.
1. DLT Offline Logstorage is designed in a way that it is enough to attach an
  external storage device to the target containing a valid configuration file to
  receive logs from applications. If corresponding udev rules are installed on
  the target the storing of log messages starts immediately. Another possibility
  to start storing of log messages is to send a trigger through
  *dlt-logstorage-ctrl* application to the DLT Daemon.
2. Furthermore, DLT Offline Logstorage chances the configured log level of
  applications, which is needed to fulfill its requirements, but generates
  security problems. It in a real production system, it may be not allowed to
  receive INFO or VERBOSE messages, but the current implementation does not have
  any possibility to avoid certain log levels.

## Known Limitations

No.  | Description
----- | ----
1 | A combination of wildcards for Application ID ***and*** Context ID is only possible when EcuID is specified.
