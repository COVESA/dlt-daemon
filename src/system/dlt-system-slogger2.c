/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2020, Volvo Car Corporation
 *
 * This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 */

/*!
 * \author Yaroslav Yatsenko <yaroslav.yatsenko@volvocars.com>
 *
 * \copyright Copyright © 2020 Volvo Car Corporation. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-system-slogger2.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-system-slogger2.c                                         **
**                                                                            **
**  TARGET    : QNX                                                           **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Yaroslav Yatsenko <yaroslav.yatsenko@volvocars.com>           **
**                                                                            **
**  PURPOSE   : Redirecting of slogger2 output to dlt                         **
**                                                                            **
**  REMARKS   :                                                               **
**                                                                            **
**  PLATFORM DEPENDANT [yes/no]: yes                                          **
**                                                                            **
**  TO BE CHANGED BY USER [yes/no]: no                                        **
**                                                                            **
*******************************************************************************/

#if defined(DLT_SLOGGER2_ENABLE)

#include <pthread.h>
#include <sys/slog2.h>
#include <slog2_parse.h>

#include "dlt-system.h"

extern DltSystemThreads threads;

DLT_IMPORT_CONTEXT(dltsystem)
DLT_DECLARE_CONTEXT(slogger2Context)

int on_packet_callback(slog2_packet_info_t *info, void *payload, void *param)
{
    (void)param;
    DltLogLevelType logLevel = DLT_LOG_DEFAULT;

    switch (info->severity)
    {
        case SLOG2_CRITICAL:
            logLevel = DLT_LOG_FATAL;
            break;
        case SLOG2_ERROR:
            logLevel = DLT_LOG_ERROR;
            break;
        case SLOG2_WARNING:
            logLevel = DLT_LOG_WARN;
            break;
        case SLOG2_NOTICE:
            logLevel = DLT_LOG_INFO;
            break;
        case SLOG2_INFO:
            logLevel = DLT_LOG_INFO;
            break;
        case SLOG2_DEBUG1:
            logLevel = DLT_LOG_DEBUG;
            break;
        case SLOG2_DEBUG2:
            logLevel = DLT_LOG_DEBUG;
            break;
        }

        DLT_LOG(slogger2Context, logLevel, DLT_STRING((char *)payload));

        return 0;
}

void slogger2_thread(void *v_conf)
{
    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-slogger2, in thread."));

    DltSystemConfiguration *conf = (DltSystemConfiguration *)v_conf;
    DLT_REGISTER_CONTEXT(slogger2Context, conf->Slogger2.ContextId, "SLOGGER2 Adapter");

    slog2_packet_info_t packet_info = SLOG2_PACKET_INFO_INIT;

    /* Streaming of all merged buffers. */
    if(slog2_parse_all(SLOG2_PARSE_FLAGS_DYNAMIC,
                        NULL,
                        NULL,
                        &packet_info,
                        on_packet_callback,
                        NULL ) == -1) {
                            DLT_LOG(slogger2Context, DLT_LOG_FATAL,
                                DLT_STRING("slog2_parse_all(SLOG2_PARSE_FLAGS_DYNAMIC) failed."));
                            return;
                     }

    /* Parsing incoming messages from the current moment.*/
    while (!threads.shutdown) {
        if(slog2_parse_all(SLOG2_PARSE_FLAGS_CURRENT,
                        conf->Slogger2.LogsPath,
                        NULL,
                        &packet_info,
                        on_packet_callback,
                        NULL ) == -1) {
                            DLT_LOG(slogger2Context, DLT_LOG_FATAL,
                                DLT_STRING("slog2_parse_all(SLOG2_PARSE_FLAGS_CURRENT) failed."));
                            return;
                     }
    }
}
#endif
