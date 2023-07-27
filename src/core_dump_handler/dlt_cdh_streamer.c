/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
 *
 * This file is part of COVESA Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.covesa.org/.
 */

/*!
 * \author Magneti Marelli http://www.magnetimarelli.com
 * \author Lutz Helwing <lutz_helwing@mentor.com>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_cdh_streamer.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include "dlt_cdh_streamer.h"

#define Z_CHUNK_SZ      1024 * 128
#define Z_MODE_STR      "wb1"

cdh_status_t stream_init(file_streamer_t *p_fs, const char *p_src_fname, const char *p_dst_fname)
{
    if (p_fs == NULL) {
        syslog(LOG_ERR, "Internal pointer error in 'stream_init'");
        return CDH_NOK;
    }

    memset(p_fs, 0, sizeof(file_streamer_t));

    /* Allow to not save the coredump */
    if (p_dst_fname == NULL) {
        p_fs->gz_dst_file = 0;
    }
    else {
        /* Create output file */
        p_fs->gz_dst_file = gzopen(p_dst_fname, Z_MODE_STR);

        if (p_fs->gz_dst_file == Z_NULL) {
            /*return CDH_NOK; */
            syslog(LOG_ERR, "Cannot open output filename <%s>. %s", p_dst_fname, strerror(errno));
            p_fs->gz_dst_file = 0;

        }
    }

    if (p_fs->gz_dst_file == Z_NULL)
        syslog(LOG_WARNING, "The coredump will be processed, but not written");

    /* Open input file */
    if (p_src_fname == NULL) {
        p_fs->stream = stdin;
    }
    else if ((p_fs->stream = fopen(p_src_fname, "rb")) == NULL) {
        syslog(LOG_ERR, "Cannot open filename <%s>. %s", p_src_fname, strerror(errno));
        return CDH_NOK;
    }

    /* Allocate read buffer */
    if ((p_fs->read_buf = (unsigned char *)malloc(Z_CHUNK_SZ)) == NULL) {
        syslog(LOG_ERR, "Cannot allocate %d bytes for read buffer. %s", Z_CHUNK_SZ, strerror(errno));
        return CDH_NOK;
    }

    return CDH_OK;
}

cdh_status_t stream_close(file_streamer_t *p_fs)
{
    if (p_fs == NULL) {
        syslog(LOG_ERR, "Internal pointer error in 'stream_close'");
        return CDH_NOK;
    }

    if (p_fs->gz_dst_file != NULL) {
        gzflush(p_fs->gz_dst_file, Z_FINISH);
        gzclose(p_fs->gz_dst_file);
        p_fs->gz_dst_file = NULL;
    }

    if (p_fs->stream != NULL) {
        fclose(p_fs->stream);
        p_fs->stream = NULL;
    }

    if (p_fs->read_buf != NULL) {
        free(p_fs->read_buf);
        p_fs->read_buf = NULL;
    }

    return CDH_OK;
}

cdh_status_t stream_read(file_streamer_t *p_fs, void *p_buf, unsigned int p_size)
{
    unsigned int byte_read = 0;

    if (p_fs == NULL) {
        syslog(LOG_ERR, "Internal pointer error in 'stream_read'");
        return CDH_NOK;
    }

    if (p_buf == NULL) {
        syslog(LOG_ERR, "Internal buffer pointer error in 'stream_read'");
        return CDH_NOK;
    }

    if ((byte_read = fread(p_buf, 1, p_size, p_fs->stream)) != p_size) {
        syslog(LOG_WARNING, "Cannot read %d bytes from src. %s", p_size, strerror(errno));
        return CDH_NOK;
    }

    p_fs->offset += byte_read;

    if (p_fs->gz_dst_file != NULL)
        gzwrite(p_fs->gz_dst_file, p_buf, byte_read);

    return CDH_OK;
}

int stream_finish(file_streamer_t *p_fs)
{
    if ((p_fs == NULL) || (p_fs->stream == NULL)) {
        syslog(LOG_ERR, "Internal pointer error in 'stream_move_ahead'");
        return CDH_NOK;
    }

    while (!feof(p_fs->stream)) {
        size_t read_bytes = fread(p_fs->read_buf, 1, Z_CHUNK_SZ, p_fs->stream);

        if (p_fs->gz_dst_file != NULL)
            gzwrite(p_fs->gz_dst_file, p_fs->read_buf, read_bytes);

        p_fs->offset += read_bytes;

        if (ferror(p_fs->stream)) {
            syslog(LOG_WARNING, "Error reading from the src stream: %s", strerror(errno));
            return CDH_NOK;
        }
    }

    return CDH_OK;
}

int stream_move_to_offest(file_streamer_t *p_fs, unsigned int p_offset)
{
    int bytes_to_read = 0;

    if (p_fs == NULL) {
        syslog(LOG_ERR, "Internal pointer error in 'stream_move_to_offest'");
        return CDH_NOK;
    }

    bytes_to_read = p_offset - p_fs->offset;

    return stream_move_ahead(p_fs, bytes_to_read);
}

int stream_move_ahead(file_streamer_t *p_fs, unsigned int p_nbbytes)
{
    int bytes_to_read = p_nbbytes;

    if (p_fs == NULL) {
        syslog(LOG_ERR, "Internal pointer error in 'stream_move_ahead'");
        return CDH_NOK;
    }

    while (bytes_to_read > 0) {
        size_t chunk_size = bytes_to_read > Z_CHUNK_SZ ? Z_CHUNK_SZ : bytes_to_read;
        size_t read_bytes = fread(p_fs->read_buf, 1, chunk_size, p_fs->stream);

        if (read_bytes != chunk_size) {
            syslog(LOG_WARNING, "Cannot move ahead by %d bytes from src. Read %lu bytes", p_nbbytes, read_bytes);
            return CDH_NOK;
        }

        if (p_fs->gz_dst_file != 0)
            gzwrite(p_fs->gz_dst_file, p_fs->read_buf, chunk_size);

        bytes_to_read -= chunk_size;
    }

    p_fs->offset += p_nbbytes;

    return CDH_OK;
}

unsigned int stream_get_offset(file_streamer_t *p_fs)
{
    if (p_fs == NULL) {
        syslog(LOG_ERR, "Internal pointer error in 'stream_get_offset'");
        return CDH_NOK;
    }

    return p_fs->offset;
}
