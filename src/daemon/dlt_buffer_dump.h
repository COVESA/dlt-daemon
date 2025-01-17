/*
* SPDX license identifier: MPL-2.0
*
* Copyright (C) 2022, Daimler TSS GmbH
*
* This file is part of COVESA Project DLT - Diagnostic Log and Trace.
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License (MPL), v. 2.0.
* If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* For further information see https://www.covesa.global/.
*
* \file dlt_buffer_dump.h
 */

#ifndef AUTOMOTIVE_DLT_SRC_DAEMON_DLT_BUFFER_DUMP_H_
#define AUTOMOTIVE_DLT_SRC_DAEMON_DLT_BUFFER_DUMP_H_

#include "dlt_client.h"
#include "dlt_common.h"
#include "dlt_daemon_common.h"

/**
 * Copy the ring buffer from src to dst
 * @param src the ringbuffer source
 * @param dst the ringbuffer dest
 * @return DLT_RETURN OK if it is successful
 */
DltReturnValue dlt_ringbuffer_copy(const DltBuffer *src, DltBuffer *dst);

/**
 * dump the ringbuffer to disk
 * @param daemon  the DltDaemon
 * @param ring_buffer the ringbuffer
 * @param dump_file_path the dump file path
 * @return DLT_RETURN OK if it is successful
 */
DltReturnValue dlt_buffer_dump(const DltDaemon *daemon, const DltBuffer *ring_buffer, const char *dump_file_path);

#endif //AUTOMOTIVE_DLT_SRC_DAEMON_DLT_BUFFER_DUMP_H_
