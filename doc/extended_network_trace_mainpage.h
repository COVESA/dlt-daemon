/**
 * @licence app begin@
 * Copyright (C) 2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt - Diagnostic Log and Trace console apps.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Lassi Marttala <lassi.lm.marttala@partner.bmw.de> BMW 2012
 *
 * For further information see http://www.genivi.org/.
 * @licence end@
 */
 
/** \mainpage 

\image html genivilogo.png

\par More information
can be found at https://collab.genivi.org/wiki/display/genivi/GENIVI+Home \n

\par About DLT
The DLT is a Deamon that enables diagnostic log and trace in a GENIVI headunit and is based on AUTOSAR 4.0.

DLT Extended Network Trace Main Page

\section Introduction Introduction
The extended network trace functionality allows now for large network messages to be sent or truncated.

\section Protocol Protocol
When truncation of messages is allowed, the truncated messages will be wrapped into a special message which indicates
that a network trace message was truncated and what was the original size of the message.

Segmented messages are sent in multiple packages. The package stream is prepended with a a start message indicating
which contain a unique handle for this stream, size of data to follow, count of segments to follow and segment size.
Each segment contains the stream handle, segment sequence number, the data and data length.
Finally after sending all the data segments, one more packet is sent to indicate the end of the stream.

\subsection Truncated Truncated package
Truncated message can be sent using the following function:
\code
int dlt_user_trace_network_truncated(DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len, void *payload, int allow_truncate)
\endcode
This will send a packet in the following format:
\code
,----------------------------------------------------.
|                      NWTR                          | Package identifier. STRING
|----------------------------------------------------|
|                     header                         | nw_trace header and it's length. RAW
|----------------------------------------------------|
|                      size                          | Original size of the message. UINT
|----------------------------------------------------|
|                    payload                         | The truncated nw_trace payload. RAW
`----------------------------------------------------'
\endcode

\subsection Segmented Segmented messages
User can send a segmented network trace message asynchronously using:
\code
void dlt_user_trace_network_segmented(DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len, void *payload)
\endcode
This will start a background thread and return immediately. User can also send all the required packages one by one using:
\code
int dlt_user_trace_network_segmented_start(unsigned int *id, DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len)
int dlt_user_trace_network_segmented_segment(int id, DltContext *handle, DltNetworkTraceType nw_trace_type, int sequence, uint16_t payload_len, void *payload)
int dlt_user_trace_network_segmented_end(int id, DltContext *handle, DltNetworkTraceType nw_trace_type)
\endcode
It is not recommended to use these functions unless you really have to.

\subsection Start Segmented start packet
The first packet in the stream is the header:
\code
,----------------------------------------------------.
|                      NWST                          | Package identifier. STRING
|----------------------------------------------------|
|                  streamhandle                      | Unique identifier for all packages in the stream. UINT
|----------------------------------------------------|
|                     header                         | nw_trace header and it's length. RAW
|----------------------------------------------------|
|                   payloadsize                      | Size of the complete payload in this stream. UINT
|----------------------------------------------------|
|                   segmentcount                     | Number of segments to follow.
|----------------------------------------------------|
|                    segmentlen                      | Size of one segment
`----------------------------------------------------'
\endcode

\subsection Data Data segment
After the header, follows a stream of data segments.
\code
,----------------------------------------------------.
|                      NWCH                          | Package identifier. STRING
|----------------------------------------------------|
|                  streamhandle                      | Unique identifier for all packages in the stream. UINT
|----------------------------------------------------|
|                    sequence                        | Sequence number of this segment. UINT
|----------------------------------------------------| 
|                     data                           | One segment of the original nw_trace. RAW
`----------------------------------------------------'
\endcode

\subsection End End packet
After all the segments have been sent, an End identifier is sent.
\code
,----------------------------------------------------.
|                      NWEN                          | Package identifier. STRING
|----------------------------------------------------|
|                  streamhandle                      | Unique identifier for all packages in the stream. UINT
`----------------------------------------------------'
\endcode


\section Requirements Requirements
 \code
 automotive-dlt
 \endcode
 <hr>

\section Licence Licence
Copyright 2012 - BMW AG, Lassi Marttala <lassi.lm.marttala@partner.bmw.de>

* */
