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
 * \file dlt-kpi-process-list.h
 */

#ifndef SRC_KPI_DLT_KPI_PROCESS_LIST_H_
#define SRC_KPI_DLT_KPI_PROCESS_LIST_H_

#include "dlt-kpi-process.h"
#include "dlt-kpi-common.h"

typedef struct
{
    struct DltKpiProcess *start, *cursor;
} DltKpiProcessList;

DltKpiProcessList *dlt_kpi_create_process_list();
DltReturnValue dlt_kpi_free_process_list_soft(DltKpiProcessList *list);
DltReturnValue dlt_kpi_free_process_list(DltKpiProcessList *list);
DltKpiProcess *dlt_kpi_get_process_at_cursor(DltKpiProcessList *list);
DltReturnValue dlt_kpi_increment_cursor(DltKpiProcessList *list);
DltReturnValue dlt_kpi_decrement_cursor(DltKpiProcessList *list);
DltReturnValue dlt_kpi_reset_cursor(DltKpiProcessList *list);
DltReturnValue dlt_kpi_add_process_at_start(DltKpiProcessList *list, DltKpiProcess *process);
DltReturnValue dlt_kpi_add_process_before_cursor(DltKpiProcessList *list, DltKpiProcess *process);
DltReturnValue dlt_kpi_add_process_after_cursor(DltKpiProcessList *list, DltKpiProcess *process);
DltReturnValue dlt_kpi_remove_process_at_cursor_soft(DltKpiProcessList *list);
DltReturnValue dlt_kpi_remove_process_at_cursor(DltKpiProcessList *list);

/* DltReturnValue dlt_kpi_remove_process_after_cursor(DltKpiProcessList *list); */
/* DltReturnValue dlt_kpi_remove_first_process(DltKpiProcessList *list); */

#endif /* SRC_KPI_DLT_KPI_PROCESS_LIST_H_ */
