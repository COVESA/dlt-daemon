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
 * \author Lassi Marttala <lassi.lm.marttala@partner.bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-system-logfile.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-system-logfile.c                                                  **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Lassi Marttala <lassi.lm.marttala@partner.bmw.de>             **
**              Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
**                                                                            **
**  PURPOSE   :                                                               **
**                                                                            **
**  REMARKS   :                                                               **
**                                                                            **
**  PLATFORM DEPENDANT [yes/no]: yes                                          **
**                                                                            **
**  TO BE CHANGED BY USER [yes/no]: no                                        **
**                                                                            **
*******************************************************************************/


#include <unistd.h>
#include "dlt-system.h"

/* Modes of sending */
#define SEND_MODE_OFF  0
#define SEND_MODE_ONCE 1
#define SEND_MODE_ON   2

DLT_IMPORT_CONTEXT(dltsystem)

DltContext logfileContext[DLT_SYSTEM_LOG_FILE_MAX];
int logfile_delays[DLT_SYSTEM_LOG_FILE_MAX];

void send_file(LogFileOptions const *fileopt, int n)
{
    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-logfile, sending file."));
    FILE *pFile;
    DltContext context = logfileContext[n];
    char buffer[1024];
    int bytes;
    int seq = 1;

    pFile = fopen((*fileopt).Filename[n], "r");

    if (pFile != NULL) {
        while (!feof(pFile)) {
            bytes = fread(buffer, 1, sizeof(buffer) - 1, pFile);

            if (bytes >= 0)
                buffer[bytes] = 0;
            else
                buffer[0] = 0;

            if (feof(pFile)) {
                DLT_LOG(context, DLT_LOG_INFO, DLT_INT(seq * -1), DLT_STRING(buffer));
                break;
            }
            else {
                DLT_LOG(context, DLT_LOG_INFO, DLT_INT(seq++), DLT_STRING(buffer));
            }
        }

        fclose(pFile);
    }
    else {
        DLT_LOG(dltsystem, DLT_LOG_ERROR,
                DLT_STRING("dlt-system-logfile, failed to open file."),
                DLT_STRING((*fileopt).Filename[n]));
    }
}

void register_contexts(LogFileOptions const *fileopts)
{
    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-logfile, registering file contexts."));
    int i;

    for (i = 0; i < (*fileopts).Count; i++)
        DLT_REGISTER_CONTEXT(logfileContext[i], (*fileopts).ContextId[i],
                             (*fileopts).Filename[i]);
}

void logfile_init(void *v_conf)
{
    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-logfile, in thread."));
    DltSystemConfiguration *conf = (DltSystemConfiguration *)v_conf;

    register_contexts(&(conf->LogFile));

    for (int i = 0; i < conf->LogFile.Count; i++)
        logfile_delays[i] = conf->LogFile.TimeDelay[i];
}

void logfile_fd_handler(void *v_conf)
{
    DltSystemConfiguration *conf = (DltSystemConfiguration *)v_conf;
    for (int i = 0; i < conf->LogFile.Count; i++) {
        if (conf->LogFile.Mode[i] == SEND_MODE_OFF)
            continue;

        if (logfile_delays[i] <= 0) {
            send_file(&(conf->LogFile), i);
            logfile_delays[i] = conf->LogFile.TimeDelay[i];

            if (conf->LogFile.Mode[i] == SEND_MODE_ONCE)
                conf->LogFile.Mode[i] = SEND_MODE_OFF;
        }
        else {
            logfile_delays[i]--;
        }
    }
}

