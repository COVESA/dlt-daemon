/**
 * @licence app begin@
 * Copyright (C) 2015  Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * This file is part of GENIVI Project Dlt - Diagnostic Log and Trace console apps.
 *
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Christoph Lipka <clipka@jp.adit-jv.com> ADIT 2015
 * \author Frederic Berat <fberat@de.adit-jv.com> ADIT 2015
 *
 * \file dlt-control-common.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef _DLT_CONTROL_COMMON_H_
#define _DLT_CONTROL_COMMON_H_

#include <stdio.h>

#include "dlt_common.h"

#define DLT_CTRL_TIMEOUT 10

#define DLT_CTRL_ECUID_LEN 10
#define DLT_DAEMON_FLAG_MAX 256

#ifndef pr_fmt
#   define pr_fmt(fmt) fmt
#endif

#ifndef USE_STDOUT
#   define PRINT_OUT stderr
#else
#   define PRINT_OUT stdout
#endif

#define pr_error(fmt, ...) \
    ({ fprintf(PRINT_OUT, pr_fmt(fmt), ## __VA_ARGS__); fflush(PRINT_OUT); })
#define pr_verbose(fmt, ...) \
    ({ if (get_verbosity()) { fprintf(PRINT_OUT, pr_fmt(fmt), ## __VA_ARGS__); fflush(PRINT_OUT); } })

#define DLT_CTRL_DEFAULT_ECUID "ECU1"

/* To be used as Dlt Message body when sending to DLT daemon */
typedef struct
{
    void *data; /**< data to be send to DLT Daemon */
    int size;   /**< size of that data */
} DltControlMsgBody;

/* As verbosity, ecuid and timeout are needed during the communication,
 * defining getter and setters here.
 * Then there is no need to define them in the control's user application.
 */
int get_verbosity(void);
void set_verbosity(int);

char *get_ecuid(void);
void set_ecuid(char *);

long get_timeout(void);
void set_timeout(long);

/* Parse dlt.conf file and return the value of requested configuration */
int dlt_parse_config_param(char *config_id, char **config_data);

/* Initialize the connection to the daemon */
int dlt_control_init(int (*response_analyser)(char *, void *, int),
                     char *ecuid,
                     int verbosity);

/* Send a message to the daemon. The call is not thread safe. */
int dlt_control_send_message(DltControlMsgBody *, int);

/* Destroys the connection to the daemon */
int dlt_control_deinit(void);
#endif
