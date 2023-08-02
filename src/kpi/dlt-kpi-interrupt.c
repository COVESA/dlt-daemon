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
 * \file dlt-kpi-interrupt.c
 */

#include "dlt-kpi-interrupt.h"

DltReturnValue dlt_kpi_log_interrupts(DltContext *ctx, DltLogLevelType log_level)
{
    if (ctx == NULL) {
        fprintf(stderr, "%s: Nullpointer parameter (NULL) !\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    char buffer[BUFFER_SIZE];
    *buffer = '\0';

    char file_buffer[BUFFER_SIZE];
    char *token, *delim = " \t", *delim2 = " \t\n", *check;
    int head_line = 1, first_row = 1, cpu_count = 0, column = 0, buffer_offset = 0;
    DltReturnValue ret;

    if ((ret = dlt_kpi_read_file("/proc/interrupts", file_buffer, BUFFER_SIZE)) < DLT_RETURN_OK) return ret;

    token = strtok(file_buffer, delim);

    while (token != NULL) {
        if (head_line) {
            if ((strlen(token) > 3) && (token[0] == 'C') && (token[1] == 'P') && (token[2] == 'U')) {
                cpu_count++;
            }
            else if (cpu_count <= 0)
            {
                fprintf(stderr, "%s: Could not parse CPU count !\n", __func__);
                return DLT_RETURN_ERROR;
            }
            else if (strcmp(token, "\n") == 0)
            {
                head_line = 0;
            }

            token = strtok(NULL, delim);
        }
        else {
            int tokenlen = strlen(token);

            if (token[tokenlen - 1] == ':') {
                column = 0;

                if (first_row)
                    first_row = 0;
                else
                    buffer_offset += snprintf(buffer + buffer_offset, BUFFER_SIZE - buffer_offset, "\n");
            }

            if (column == 0) { /* IRQ number */
                buffer_offset += snprintf(buffer + buffer_offset,
                                          BUFFER_SIZE - buffer_offset,
                                          "%.*s;",
                                          tokenlen - 1,
                                          token);
            }
            else if (column <= cpu_count)
            {
                long int interrupt_count = strtol(token, &check, 10);

                if (*check != '\0') {
                    fprintf(stderr, "%s: Could not parse interrupt count for CPU !\n", __func__);
                    return DLT_RETURN_ERROR;
                }

                buffer_offset += snprintf(buffer + buffer_offset,
                                          BUFFER_SIZE - buffer_offset,
                                          "cpu%d:%ld;",
                                          column - 1,
                                          interrupt_count);
            }

            column++;

            token = strtok(NULL, delim2);
        }
    }

    /* synchronization message */
    DLT_LOG(*ctx, log_level, DLT_STRING("IRQ"), DLT_STRING("BEG"));

    DltContextData ctx_data;

    if ((ret = dlt_user_log_write_start(ctx, &ctx_data, log_level)) < DLT_RETURN_OK) {
        fprintf(stderr, "%s: dlt_user_log_write_start() returned error\n", __func__);
        return ret;
    }

    if ((ret = dlt_user_log_write_string(&ctx_data, "IRQ")) < DLT_RETURN_OK) {
        fprintf(stderr, "%s: dlt_user_log_write_string() returned error\n", __func__);
        return ret;
    }

    token = strtok(buffer, "\n");

    while (token != NULL) {
        if (dlt_user_log_write_string(&ctx_data, token) < DLT_RETURN_OK) {
            /* message buffer full, start new one */
            if ((ret = dlt_user_log_write_finish(&ctx_data)) < DLT_RETURN_OK) {
                fprintf(stderr, "%s: dlt_user_log_write_finish() returned error\n", __func__);
                return ret;
            }

            if ((ret = dlt_user_log_write_start(ctx, &ctx_data, log_level)) < DLT_RETURN_OK) {
                fprintf(stderr, "%s: dlt_user_log_write_start() returned error\n", __func__);
                return ret;
            }

            if ((ret = dlt_user_log_write_string(&ctx_data, "IRQ")) < DLT_RETURN_OK) {
                fprintf(stderr, "%s: dlt_user_log_write_string() returned error\n", __func__);
                return ret;
            }
        }
        else {
            token = strtok(NULL, "\n");
        }
    }

    if ((ret = dlt_user_log_write_finish(&ctx_data)) < DLT_RETURN_OK) {
        fprintf(stderr, "%s: dlt_user_log_write_finish() returned error\n", __func__);
        return ret;
    }

    /* synchronization message */
    DLT_LOG(*ctx, log_level, DLT_STRING("IRQ"), DLT_STRING("END"));

    return DLT_RETURN_OK;
}
