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
* \file dlt_buffer_dump.c
*/

#include "dlt_buffer_dump.h"
#include "dlt-daemon_cfg.h"
#include <syslog.h>
#include <stdlib.h>
#include <string.h>

DltReturnValue dlt_ringbuffer_copy(const DltBuffer *src, DltBuffer *dst) {
    if (src == NULL || dst == NULL) {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    dst->size = src->size;
    dst->min_size = src->min_size;
    dst->max_size = src->max_size;
    dst->step_size = src->step_size;

    int length = dst->size + sizeof(DltBufferHead);
    dst->shm = malloc(length);
    memcpy(dst->shm, src->shm, length);
    dst->mem = (unsigned char *) (dst->shm + sizeof(DltBufferHead));

    return DLT_RETURN_OK;
}

DltReturnValue dlt_buffer_dump(const DltDaemon *daemon, const DltBuffer *ring_buffer, const char *dump_file_path) {
    uint8_t data[DLT_DAEMON_RCVBUFSIZE] = {0};
    int length;
    DltBuffer buffer = {0};

    DltReturnValue ret = dlt_ringbuffer_copy(ring_buffer, &buffer);
    if (ret != DLT_RETURN_OK) {
        return ret;
    }

    FILE *file = fopen(dump_file_path, "w");
    if (file == NULL) {
        dlt_vlog(LOG_ERR, "Could not open dump file:%s\n", dump_file_path);
        dlt_buffer_free_dynamic(&buffer);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    DltStorageHeader storage_header = {0};
    dlt_set_storageheader(&storage_header, daemon->ecuid);

    while ((length = dlt_buffer_copy(&buffer, data, sizeof(data))) > 0) {
        fwrite(&storage_header, sizeof(DltStorageHeader), 1, file);
        fwrite(&data, length, 1, file);
        dlt_buffer_remove(&buffer);
    }

    fclose(file);
    dlt_buffer_free_dynamic(&buffer);

    return ret;
}