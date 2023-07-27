/**
 * Copyright (C) 2020 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * DLT QNX system functionality header file.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Nguyen Dinh Thi <Thi.NguyenDinh@vn.bosch.com> ADIT 2020
 *
 * \file: dlt-qnx-system.h
 * For further information see http://www.covesa.org/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_qnx-system.h                                              **
**                                                                            **
**  TARGET    : QNX                                                           **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Thi.NguyenDinh@vn.bosch.com                                   **
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

/*******************************************************************************
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
** ndt          Nguyen Dinh Thi            ADIT                               **
**                                                                            **
*******************************************************************************/

#ifndef DLT_QNX_SYSTEM_H_
#define DLT_QNX_SYSTEM_H_

#include "dlt.h"
#include "dlt_user_macros.h"

/* Constants */
#define DEFAULT_CONF_FILE ( CONFIGURATION_FILES_DIR "/dlt-qnx-system.conf")

#define MAX_LINE 1024
#define MAX_THREADS 8

/* Macros */
#define MALLOC_ASSERT(x)\
    do\
    {\
        if(x == NULL) {\
            fprintf(stderr, "%s - %d: Out of memory\n",  __func__, __LINE__);\
            abort();\
        }\
    }\
    while (0)

#ifdef __cplusplus
extern "C" {
#endif

/* Command line options */
typedef struct {
    char    *configurationFileName;
    int     daemonize;
} DltQnxSystemCliOptions;

/* Configuration slogger2 options */
typedef struct {
    int      enable;
    char     *contextId;
    int      useOriginalTimestamp;
} Qnxslogger2Options;

typedef struct {
    char                 *applicationId;
    char                 *applicationContextId;
    Qnxslogger2Options   qnxslogger2;
} DltQnxSystemConfiguration;

typedef struct {
    pthread_t threads[MAX_THREADS];
    pthread_t mainThread;
    int count;
    int shutdown;
} DltQnxSystemThreads;

void start_qnx_slogger2(DltQnxSystemConfiguration *conf);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* DLT_QNX_SYSTEM_H_ */
