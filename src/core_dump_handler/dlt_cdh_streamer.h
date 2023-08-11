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
 * \file dlt_cdh_streamer.h
 */

#ifndef DLT_CDH_STREAMER_H
#define DLT_CDH_STREAMER_H

#include <stdio.h>
#include <zlib.h>

#include "dlt_cdh_definitions.h"

typedef struct
{
    FILE *stream;
    unsigned int offset;
    gzFile gz_dst_file;
    unsigned char *read_buf;

} file_streamer_t;

cdh_status_t stream_init(file_streamer_t *p_fs, const char *p_src_fname, const char *p_dst_fname);
cdh_status_t stream_close(file_streamer_t *p_fs);
cdh_status_t stream_read(file_streamer_t *p_fs, void *p_buf, unsigned int p_size);
cdh_status_t stream_finish(file_streamer_t *p_fs);
cdh_status_t stream_move_to_offest(file_streamer_t *p_fs, unsigned int p_offset);
cdh_status_t stream_move_ahead(file_streamer_t *p_fs, unsigned int p_nbbytes);
unsigned int stream_get_offset(file_streamer_t *p_fs);

#endif /* #ifndef DLT_CDH_STREAMER_H */
