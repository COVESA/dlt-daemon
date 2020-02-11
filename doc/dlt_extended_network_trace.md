# Extended Network Trace

Back to [README.md](../README.md)

## Introduction

The extended network trace allows the user to send or truncate network trace
messages that are larger than the normal maximum size of a DLT message. This
interface will be enabled if following calls are supported on the target:
- mq\_open
- mq\_close
- mq\_unlink
- mq\_send
- mq\_receive

## Protocol

When truncation of messages is allowed, the truncated messages will be wrapped
into a special message which indicates that a network trace message was
truncated and what was the original size of the message.

Segmented messages are sent in multiple packages. The package stream is
prepended with a start message indicating which contains a unique handle for
this stream, size of data to follow, count of segments to follow and segment
size.

Each segment contains the stream handle, segment sequence number, the data and
data length.

Finally after sending all the data segments, one more packet is sent to indicate
the end of the stream.

## Truncated package

Truncated message can be sent using the following function:

` int dlt_user_trace_network_truncated(DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len, void *payload, int allow_truncate) `

This will send a packet in the following format:

Value | Description | Type
:--- | :--- | :---
NWTR | Package identifier | STRING
header | nw_trace header and it's length | RAW
size | Original size of the message | UINT
payload | The truncated nw_trace payload | RAW

## Segmented messages

User can send a segmented network trace message asynchronously using:

` void dlt_user_trace_network_segmented(DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len, void *payload) `

This will start a background thread and return immediately.

User can also send all the required packages one by one using:

` int dlt_user_trace_network_segmented_start(unsigned int *id, DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len) `

` int dlt_user_trace_network_segmented_segment(int id, DltContext *handle, DltNetworkTraceType nw_trace_type, int sequence, uint16_t payload_len, void *payload) `

` int dlt_user_trace_network_segmented_end(int id, DltContext *handle, DltNetworkTraceType nw_trace_type) `

*NOTE*: It is not recommended to use these functions unless you really have to.

## Segmented start packet

The first packet in the stream is the header:

Value | Description | Type
:--- | :--- | :---
NWST | Package identifier | STRING
streamhandle | Unique identifier for all packages in the stream | UINT
header | nw_trace header and it's length | RAW
payloadsize | Size of the complete payload in this stream | UINT
segmentcount | Number of segments to follow | UINT
segmentlen | Size of one segment | UINT

## Data segment

After the header, follows a stream of data segments.

Value | Description | Type
:--- | :--- | :---
NWCH | Package identifier | STRING
streamhandle | Unique identifier for all packages in the stream | UINT
sequence | Sequence number of this segment | UINT
data | One segment of the original nw_trace | RAW

## End packet

After all the segments have been sent, an End identifier is sent.

Value | Description | Type
:--- | :--- | :---
NWEN | Package identifier | STRING
streamhandle | Unique identifier for all packages in the stream | UINT

## Author

Lassi Marttala <Lassi.LM.Marttala (at) partner (dot) bmw (dot) de>

## COPYRIGHT

Copyright (C) 2011 - 2015 BMW AG. License MPL-2.0: Mozilla Public License version 2.0 <http://mozilla.org/MPL/2.0/>.
