# DLT Protocol Version 2

DLT Protocol version 2 is an upgrade of DLT Protocol version 1 to follow
AUTOSAR_PRS_LogAndTraceProtocol_R22_11 standards, while version 1 was following
AUTOSAR_PRS_LogAndTraceProtocol_R19_11 standards. Major differences are as follows:

1. Header type is of 4 bytes including information about message type (verbose,
non-verbose or control), version and optional parameters.
2. Timestamp is now a conditional parameter along with Message information,
Number of arguments and Message ID.
3. Timestamp has 4 bytes for nanoseconds and 5 bytes for seconds and should store epoch time.
4. Message ID is part of header conditional parameter instead of payload in non-verbose mode.
5. Optional parameters could include ECU Id, Application Id, Context Id, Session Id,
Filename, Line number, Privacy level, Tags and Segmentation information.
6. Version 2 supports dynamic size for ECU Id, Application Id, Context Id, Filename and
Tagname, maximum length being 255 (max size in uint8). Also a any number of tags can be added,
maximum being 255 (max size in uint8).
7. No Endianness bit in header, data should be in Big Endian format.

To support the DLT Protocol v2 requirements following major changes are included.

## New features

1. Send and receive logs in v2 protocol in Verbose mode
2. Logs in all payload formats like string, int, raw, etc. which were supported by v1
3. UTC timestamp
4. Logging to local file
5. Set log level
6. Get log info
7. Get software version
8. Sending optional parameters filename, line number, tags and privacy level with logs

## Pending features [WIP]

The current implementation has yet adapted for v2 for following features:

1. Non-Verbose logs
2. Trace load control
3. Network trace
4. Message segmentation
5. Offline log storage
6. Daemon Gateway
7. File transfer
8. System log

## Utilities support

### *dlt-example-user-v2.c*

Same as *dlt-example-user.c* including usage, except using v2 logging macros.

### *dlt-example-user-func-v2.c*

Same as *dlt-example-user-func.c* including usage, except using v2 logging APIs.

### *dlt-receive-v2.c*

Same as *dlt-receive.c* including usage, reads messages sent in v2 format.

### *dlt-control-v2.c*

Same as *dlt-control.c* including usage, sends control messages and receives response in v2 format.

## Macros support

For version 2 logging, following macros can be used:
| DLTv2 Logging Macros |
| ---- |
| DLT_LOG_V2(CONTEXT, LOGLEVEL, ...) |
| DLT_LOG_STRING_V2(CONTEXT, LOGLEVEL, TEXT) |
| DLT_LOG_STRING_INT_V2(CONTEXT, LOGLEVEL, TEXT, INT_VAR) |
| DLT_LOG_STRING_UINT_V2(CONTEXT, LOGLEVEL, TEXT, UINT_VAR) |
| DLT_LOG_UINT_V2(CONTEXT, LOGLEVEL, UINT_VAR) |
| DLT_LOG_INT_V2(CONTEXT, LOGLEVEL, INT_VAR) |
| DLT_LOG_RAW_V2(CONTEXT, LOGLEVEL, BUF, LEN) |
| DLT_REGISTER_APP_V2(APPID, DESCRIPTION) |
| DLT_UNREGISTER_APP_V2() |
| DLT_UNREGISTER_APP_FLUSH_BUFFERED_LOGS_V2() |
| DLT_GET_APPID_V2(APPID) |
| DLT_REGISTER_CONTEXT_V2(CONTEXT, CONTEXTID, DESCRIPTION) |
| DLT_REGISTER_CONTEXT_LL_TS_V2(CONTEXT, CONTEXTID, DESCRIPTION, LOGLEVEL, TRACESTATUS) |
| DLT_REGISTER_CONTEXT_LLCCB_V2(CONTEXT, CONTEXTID, DESCRIPTION, CBK) |
| DLT_UNREGISTER_CONTEXT_V2(CONTEXT) |
| DLT_REGISTER_LOG_LEVEL_CHANGED_CALLBACK_V2(CONTEXT, CALLBACK) |


| Macros for optional filename, line number, tags and privacy level |
| --- |
| DLT_WITH_FILENAME_LINENUMBER(FILENAME, LINR) |
| DLT_WITH_TAGS(TAG, ...) |
| DLT_WITH_PRIVACYLEVEL(PRLV) |

## DLT-Daemon

The DLT Daemon currently supports processing both v1 and v2 dlt messages, but it
cannot handle a mix of both message versions concurrently within the same session.
To enable mixed-version usage, the Daemon requires access to the message version
information from the connection object at the time an event occurs. Therefore,
the current implementation uses the **DLT_PROTOCOL_VERSION** environment variable
to specify the active version ("1" or "2") the Daemon will process. If this variable
is not set, the Daemon defaults to the value defined in the **DLT_PROTOCOL_VERSION**
macro within *dlt-daemon.h*.

Additionally, all Daemon functions that interact with the DLT Message structure
 have been updated to support v2 message processing.

## DLT Library

The user library has been updated with new functions to support the handling of v2 messages.

- The function ```dlt_user_log_write_start``` retains its existing signature and
functionality, with internal modifications to include necessary v2 member initialization.

- A new function, ```dlt_user_log_write_finish_v2```, has been introduced. This function
handles the final packing and transmission of the v2 message.

- All data structures and functions that utilize identifiers (ECU ID, Application ID,
or Context ID) have been updated to support dynamic length variables, including relevant callback functions.

## DLT Client

The DLT client now includes dedicated APIs for handling v2 protocol messages, enabling
both message parsing and control message transmission to the daemon.

**Message Parsing:** Use the function ```dlt_message_read_v2``` to parse v2 messages
received from the buffer.

**Control Message Transmission:** Use the function ```dlt_client_send_ctrl_msg_v2```
to transmit control messages to the daemon using the dlt v2 protocol.

All individual v2 control message APIs have been updated internally to utilize
```dlt_client_send_ctrl_msg_v2``` and accept the appropriate v2 parameters.

## Packing v2 format message

Payload is packed in v2 format using following function:
```
DltReturnValue dlt_user_log_send_log_v2(DltContextData *log, const int mtype,
DltHtyp2ContentType msgcontent, int *const sent_size)
```
This process requires the message type (Verbose, Non-verbose, or Control message)
as input, and subsequently attaches the DLT Header v2 to the message payload.

## Parsing v2 format message

DLT v2 Message can be parsed using read message function.
```
int dlt_message_read_v2(DltMessageV2 *msg, uint8_t *buffer, unsigned int length,
int resync, int verbose);
```
**Important**
**Storage Header Exclusion:** It is important to note that the populated header
buffer and header size do not include the storage header as its size would be
dependent on ECU ID length. The storage header information is expected to be
managed and updated by the callback functions, the storage header size returned will be $0$.

## DLT Message v2 Structure

```
typedef struct DltMessageV2
{
    /* flags */
    int8_t found_serialheader;
    int32_t resync_offset;
    int32_t headersizev2;
    int32_t datasize;
    uint8_t *headerbufferv2;
    uint8_t *databuffer;
    int32_t databuffersize;
    uint32_t storageheadersizev2;
    uint32_t baseheadersizev2;
    uint32_t baseheaderextrasizev2;
    uint32_t extendedheadersizev2;
    DltStorageHeaderV2 storageheaderv2;
    DltBaseHeaderV2 *baseheaderv2;
    DltBaseHeaderExtraV2 headerextrav2;
    DltExtendedHeaderV2 extendedheaderv2;
} DLT_PACKED DltMessageV2;
```
- Header buffer is a pointer as the size may vary at run time based on length of different Ids.
- Additional members for size of each part of header like storage, base, conditional and
extended are added to calculate header size as per need.
- Storage header and Extended header are non pointer as now we don't directly assign
them to the pointers inside header buffer.

## Documentation

Detailed information on the new and updated structures, along with the Protocol v2 APIs,
can be found in the generated documentation. To generate the documentation, enable the
WITH_DOC build option.

## Test Plans

The test coverage for v2 functionalities has been implemented by adapting the existing v1 test suite.

- **Integration Tests:** New files, ported from the original v1 tests, are located in the
*dlt-daemon/src/tests* directory to validate v2 features. v1-specific features not supported
in current v2 are not covered in this new suite.

- **Unit Tests:** The Gtest unit test cases within the *dlt-daemon/tests* folder have been
updated to invoke and verify the new v2 APIs as per feature availability.