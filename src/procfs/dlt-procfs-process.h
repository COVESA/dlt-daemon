/*
 * @licence app begin@
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
 *
 * This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*!
 * \author Sven Hassler <sven_hassler@mentor.com>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-procfs-process.h
 */

#ifndef SRC_PROCFS_DLT_PROCFS_PROCESS_H_
#define SRC_PROCFS_DLT_PROCFS_PROCESS_H_

#include "dlt.h"
#include "dlt-procfs-common.h"
#include <stdlib.h>

typedef struct DltProcfsEventWatch DltProcfsEventWatch; // forward declaration

typedef struct DltProcfsProcess
{
    pid_t pid, ppid;
    char *command_line;
    unsigned long int cpu_time, last_cpu_time, io_wait, last_io_wait, io_bytes;
    long int rss, ctx_switches;

    struct DltProcfsProcess *next, *prev;
} DltProcfsProcess;

DltProcfsProcess *dlt_procfs_create_process();
DltProcfsProcess *dlt_procfs_clone_process(DltProcfsProcess *original);
DltReturnValue dlt_procfs_free_process(DltProcfsProcess *process);
DltReturnValue dlt_procfs_print_process(DltProcfsProcess *process);
DltReturnValue dlt_procfs_update_process(DltProcfsProcess *process, unsigned long int time_dif_ms);
DltReturnValue dlt_procfs_get_msg_process_new(DltProcfsProcess *process, char *buffer, int maxlen);
DltReturnValue dlt_procfs_get_msg_process_stop(DltProcfsProcess *process, char *buffer, int maxlen);
DltReturnValue dlt_procfs_get_msg_process_update(DltProcfsProcess *process, char *buffer, int maxlen);
DltReturnValue dlt_procfs_get_msg_process_commandline(DltProcfsProcess *process, char *buffer, int maxlen);

#endif /* SRC_PROCFS_DLT_PROCFS_PROCESS_H_ */
