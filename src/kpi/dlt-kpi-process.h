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
 * \author Sven Hassler <sven_hassler@mentor.com>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-kpi-process.h
 */

#ifndef SRC_KPI_DLT_KPI_PROCESS_H_
#define SRC_KPI_DLT_KPI_PROCESS_H_

#include "dlt.h"
#include <stdlib.h>
#include "dlt-kpi-common.h"

typedef struct DltKpiEventWatch DltKpiEventWatch; /* forward declaration */

typedef struct DltKpiProcess
{
    pid_t pid, ppid;
    char *command_line;
    unsigned long int cpu_time, last_cpu_time, io_wait, last_io_wait, io_bytes;
    long int rss, ctx_switches;

    struct DltKpiProcess *next, *prev;
} DltKpiProcess;

DltKpiProcess *dlt_kpi_create_process();
DltKpiProcess *dlt_kpi_clone_process(DltKpiProcess *original);
DltReturnValue dlt_kpi_free_process(DltKpiProcess *process);
DltReturnValue dlt_kpi_print_process(DltKpiProcess *process);
DltReturnValue dlt_kpi_update_process(DltKpiProcess *process, unsigned long int time_dif_ms);
DltReturnValue dlt_kpi_get_msg_process_new(DltKpiProcess *process, char *buffer, int maxlen);
DltReturnValue dlt_kpi_get_msg_process_stop(DltKpiProcess *process, char *buffer, int maxlen);
DltReturnValue dlt_kpi_get_msg_process_update(DltKpiProcess *process, char *buffer, int maxlen);
DltReturnValue dlt_kpi_get_msg_process_commandline(DltKpiProcess *process, char *buffer, int maxlen);

#endif /* SRC_KPI_DLT_KPI_PROCESS_H_ */
