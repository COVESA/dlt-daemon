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
 * \file dlt-kpi-process-list.c
 */

#include "dlt-kpi-process-list.h"

DltKpiProcessList *dlt_kpi_create_process_list()
{
    DltKpiProcessList *new_list = malloc(sizeof(DltKpiProcessList));

    if (new_list == NULL) {
        fprintf(stderr, "%s: Cannot create process list, out of memory\n", __func__);
        return NULL;
    }

    memset(new_list, 0, sizeof(DltKpiProcessList));
    new_list->start = new_list->cursor = NULL;

    return new_list;
}

DltReturnValue dlt_kpi_free_process_list_soft(DltKpiProcessList *list)
{
    if (list == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    free(list);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_free_process_list(DltKpiProcessList *list)
{
    if (list == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    DltKpiProcess *tmp;

    list->cursor = list->start;

    while (list->cursor != NULL) {
        tmp = list->cursor->next;
        dlt_kpi_free_process(list->cursor);
        list->cursor = tmp;
    }

    return dlt_kpi_free_process_list_soft(list);
}

DltKpiProcess *dlt_kpi_get_process_at_cursor(DltKpiProcessList *list)
{
    if (list == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return NULL;
    }

    return list->cursor;
}

DltReturnValue dlt_kpi_reset_cursor(DltKpiProcessList *list)
{
    if (list == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    list->cursor = list->start;
    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_set_cursor_at_end(DltKpiProcessList *list)
{
    if (list == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    list->cursor = list->start;

    if (list->cursor == NULL)
        return DLT_RETURN_OK;

    while (list->cursor->next != NULL)
        dlt_kpi_increment_cursor(list);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_increment_cursor(DltKpiProcessList *list)
{
    if (list == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (list->cursor == NULL)
        return DLT_RETURN_ERROR;

    list->cursor = list->cursor->next;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_decrement_cursor(DltKpiProcessList *list)
{
    if (list == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (list->cursor == NULL)
        return DLT_RETURN_ERROR;

    list->cursor = list->cursor->prev;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_add_process_at_start(DltKpiProcessList *list, DltKpiProcess *process)
{
    if ((list == NULL) || (process == NULL)) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (list->start != NULL)
        list->start->prev = process;

    process->next = list->start;
    list->start = process;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_add_process_before_cursor(DltKpiProcessList *list, DltKpiProcess *process)
{
    if ((list == NULL) || (process == NULL)) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (list->start == NULL) { /* Empty list? */
        DltReturnValue ret = dlt_kpi_add_process_at_start(list, process);
        list->cursor = NULL;
        return ret;
    }
    else if (list->cursor == NULL)
    {
        dlt_kpi_set_cursor_at_end(list);
        DltReturnValue ret = dlt_kpi_add_process_after_cursor(list, process);
        list->cursor = NULL;
        return ret;
    }

    if (list->cursor->prev != NULL)
        list->cursor->prev->next = process;
    else
        list->start = process;

    process->next = list->cursor;
    process->prev = list->cursor->prev;
    list->cursor->prev = process;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_add_process_after_cursor(DltKpiProcessList *list, DltKpiProcess *process)
{
    if ((list == NULL) || (process == NULL)) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_ERROR;
    }

    if (list->cursor == NULL)
        return dlt_kpi_add_process_at_start(list, process);

    if (list->cursor->next != NULL)
        list->cursor->next->prev = process;

    process->next = list->cursor->next;
    process->prev = list->cursor;
    list->cursor->next = process;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_remove_process_at_cursor_soft(DltKpiProcessList *list)
{
    if (list == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (list->cursor == NULL) {
        fprintf(stderr, "%s: Cursor is Invalid (NULL)\n", __func__);
        return DLT_RETURN_ERROR;
    }

    DltKpiProcess *tmp = list->cursor;

    if (tmp->prev != NULL) {
        if (tmp->next != NULL) {
            tmp->prev->next = tmp->next;
            tmp->next->prev = tmp->prev;
        }
        else {
            tmp->prev->next = NULL;
        }
    }
    else {
        if (tmp->next != NULL) {
            tmp->next->prev = NULL;
            list->start = tmp->next;
        }
        else {
            list->start = NULL;
        }
    }

    list->cursor = tmp->next; /* becomes NULL if list is at end */

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_remove_process_at_cursor(DltKpiProcessList *list)
{
    if (list == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (list->cursor == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_ERROR;
    }

    DltKpiProcess *tmp = list->cursor;
    DltReturnValue ret = dlt_kpi_remove_process_at_cursor_soft(list);

    if (ret < DLT_RETURN_OK)
        return ret;

    dlt_kpi_free_process(tmp);

    return DLT_RETURN_OK;
}

