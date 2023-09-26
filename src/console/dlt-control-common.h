/**
 * Copyright (C) 2015  Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * This file is part of COVESA Project Dlt - Diagnostic Log and Trace console apps.
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
 * For further information see http://www.covesa.org/.
 */

#ifndef _DLT_CONTROL_COMMON_H_
#define _DLT_CONTROL_COMMON_H_

#include <stdio.h>

#include "dlt_common.h"

#define DLT_CTRL_TIMEOUT 10

#define DLT_CTRL_ECUID_LEN 10
#define DLT_DAEMON_FLAG_MAX 256

#define JSON_FILTER_NAME_SIZE 16 /* Size of buffer for the filter names in json filter files */
#define JSON_FILTER_SIZE 200     /* Size in bytes, that the definition of one filter with all parameters needs */

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

#define DLT_DAEMON_DEFAULT_CTRL_SOCK_PATH "/tmp/dlt-ctrl.sock"

#define NANOSEC_PER_MILLISEC 1000000
#define NANOSEC_PER_SEC 1000000000

/* To be used as Dlt Message body when sending to DLT daemon */
typedef struct
{
    void *data; /**< data to be send to DLT Daemon */
    uint32_t size;   /**< size of that data */
} DltControlMsgBody;

/* As verbosity, ecuid, timeout, send_serial_header, resync_serial_header are
 * needed during the communication, defining getter and setters here.
 * Then there is no need to define them in the control's user application.
 */
int get_verbosity(void);
void set_verbosity(int);

char *get_ecuid(void);
void set_ecuid(char *);

int get_timeout(void);
void set_timeout(int);

void set_send_serial_header(const int value);
void set_resync_serial_header(const int value);

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

#ifdef EXTENDED_FILTERING
/**
 * Load json filter from file.
 * @param filter pointer to structure of organising DLT filter
 * @param filename filename to load filters from
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
DltReturnValue dlt_json_filter_load(DltFilter *filter, const char *filename, int verbose);
/**
 * Save filter in json format to file.
 * @param filter pointer to structure of organising DLT filter
 * @param filename filename to safe filters into
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
DltReturnValue dlt_json_filter_save(DltFilter *filter, const char *filename, int verbose);
#endif
#endif
