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
 * \file dlt-procfs-process-list.h
 */

#ifndef SRC_PROCFS_DLT_PROCFS_PROCESS_LIST_H_
#define SRC_PROCFS_DLT_PROCFS_PROCESS_LIST_H_

#include "dlt-procfs-common.h"
#include "dlt-procfs-process.h"

typedef struct
{
    struct DltProcfsProcess *start, *cursor;
} DltProcfsProcessList;

DltProcfsProcessList *dlt_procfs_create_process_list();
DltReturnValue dlt_procfs_free_process_list_soft(DltProcfsProcessList *list);
DltReturnValue dlt_procfs_free_process_list(DltProcfsProcessList *list);
DltProcfsProcess *dlt_procfs_get_process_at_cursor(DltProcfsProcessList *list);
DltReturnValue dlt_procfs_increment_cursor(DltProcfsProcessList *list);
DltReturnValue dlt_procfs_decrement_cursor(DltProcfsProcessList *list);
DltReturnValue dlt_procfs_reset_cursor(DltProcfsProcessList *list);
DltReturnValue dlt_procfs_add_process_at_start(DltProcfsProcessList *list, DltProcfsProcess *process);
DltReturnValue dlt_procfs_add_process_before_cursor(DltProcfsProcessList *list, DltProcfsProcess *process);
DltReturnValue dlt_procfs_add_process_after_cursor(DltProcfsProcessList *list, DltProcfsProcess *process);
DltReturnValue dlt_procfs_remove_process_at_cursor_soft(DltProcfsProcessList *list);
DltReturnValue dlt_procfs_remove_process_at_cursor(DltProcfsProcessList *list);

// DltReturnValue dlt_procfs_remove_process_after_cursor(DltProcfsProcessList *list);
// DltReturnValue dlt_procfs_remove_first_process(DltProcfsProcessList *list);

#endif /* SRC_PROCFS_DLT_PROCFS_PROCESS_LIST_H_ */
