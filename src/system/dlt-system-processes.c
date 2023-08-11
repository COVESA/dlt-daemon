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
 * \file dlt-system-processes.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-system-processes.c                                                  **
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
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dlt-system.h"

/* Modes of sending */
#define SEND_MODE_OFF  0
#define SEND_MODE_ONCE 1
#define SEND_MODE_ON   2

int process_delays[DLT_SYSTEM_LOG_PROCESSES_MAX];

DLT_IMPORT_CONTEXT(dltsystem)
DLT_DECLARE_CONTEXT(procContext)

void send_process(LogProcessOptions const *popts, int n)
{
    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-processes, send process info."));
    FILE *pFile;
    struct dirent *dp;
    char filename[PATH_MAX];
    char buffer[1024];
    int bytes;
    int found = 0;

    /* go through all process files in directory */
    DIR *dir = opendir("/proc");

    if (dir != NULL) {
        while ((dp = readdir(dir)) != NULL)
            if (isdigit(dp->d_name[0])) {
                buffer[0] = 0;
                snprintf(filename, PATH_MAX, "/proc/%s/cmdline", dp->d_name);
                pFile = fopen(filename, "r");

                if (pFile != NULL) {
                    bytes = fread(buffer, 1, sizeof(buffer) - 1, pFile);
                    fclose(pFile);
                }

                if ((strcmp((*popts).Name[n], "*") == 0) ||
                    (strcmp(buffer, (*popts).Name[n]) == 0)) {
                    found = 1;
                    snprintf(filename, PATH_MAX, "/proc/%s/%s", dp->d_name, (*popts).Filename[n]);
                    pFile = fopen(filename, "r");

                    if (pFile != NULL) {
                        bytes = fread(buffer, 1, sizeof(buffer) - 1, pFile);
                        fclose(pFile);

                        if (bytes > 0) {
                            buffer[bytes] = 0;
                            DLT_LOG(procContext, DLT_LOG_INFO, DLT_INT(atoi(dp->d_name)),
                                    DLT_STRING((*popts).Filename[n]), DLT_STRING(buffer));
                        }
                    }

                    if (strcmp((*popts).Name[n], "*") != 0)
                        break;
                }
            }

        closedir(dir);
    }
    else {
        DLT_LOG(dltsystem, DLT_LOG_ERROR,
                DLT_STRING("dlt-system-processes, failed to open /proc."));
    }

    if (!found)
        DLT_LOG(procContext, DLT_LOG_INFO, DLT_STRING("Process"), DLT_STRING((*popts).Name[n]),
                DLT_STRING("not running!"));
}

void logprocess_init(void *v_conf)
{
    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-processes, in thread."));

    DltSystemConfiguration *conf = (DltSystemConfiguration *)v_conf;
    DLT_REGISTER_CONTEXT(procContext, conf->LogProcesses.ContextId, "Log Processes");

    for (int i = 0; i < conf->LogProcesses.Count; i++)
        process_delays[i] = conf->LogProcesses.TimeDelay[i];
}

void logprocess_fd_handler(void *v_conf)
{
    DltSystemConfiguration *conf = (DltSystemConfiguration *)v_conf;
    for (int i = 0; i < conf->LogProcesses.Count; i++) {
        if (conf->LogProcesses.Mode[i] == SEND_MODE_OFF)
            continue;

        if (process_delays[i] <= 0) {
            send_process(&(conf->LogProcesses), i);
            process_delays[i] = conf->LogProcesses.TimeDelay[i];

            if (conf->LogProcesses.Mode[i] == SEND_MODE_ONCE)
                conf->LogProcesses.Mode[i] = SEND_MODE_OFF;
        }
        else {
            process_delays[i]--;
        }
    }
}
