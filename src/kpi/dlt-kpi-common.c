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
 * \file dlt-kpi-common.c
 */

#include "dlt-kpi-common.h"

static int dlt_kpi_cpu_count = -1;

DltReturnValue dlt_kpi_read_file_compact(char *filename, char **target)
{
    if ((filename == NULL) || (target == NULL)) {
        fprintf(stderr, "%s: Nullpointer parameter!\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    char buffer[BUFFER_SIZE];
    int ret = dlt_kpi_read_file(filename, buffer, BUFFER_SIZE);

    if (ret < DLT_RETURN_OK)
        return ret;

    if ((*target = malloc(strlen(buffer) + 1)) == NULL) {
        fprintf(stderr, "Out of memory!\n");
        return DLT_RETURN_ERROR;
    }

    memcpy(*target, buffer, strlen(buffer) + 1);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_read_file(char *filename, char *buffer, uint maxLength)
{
    if ((filename == NULL) || (buffer == NULL)) {
        fprintf(stderr, "%s: Nullpointer parameter!\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    FILE *file = fopen(filename, "r");

    if (file == NULL)
        /* fprintf(stderr, "Could not read file %s\n", filename); */
        return DLT_RETURN_ERROR;

    int buflen = fread(buffer, 1, maxLength - 1, file);
    buffer[buflen] = '\0';

    fclose(file);

    return DLT_RETURN_OK;
}

int dlt_kpi_read_cpu_count()
{
    char buffer[BUFFER_SIZE];
    int ret = dlt_kpi_read_file("/proc/cpuinfo", buffer, sizeof(buffer));

    if (ret != 0) {
        fprintf(stderr, "dlt_kpi_get_cpu_count(): Could not read /proc/cpuinfo\n");
        return -1;
    }

    char *delim = "[] \t\n";
    char *tok = strtok(buffer, delim);

    if (tok == NULL) {
        fprintf(stderr, "dlt_kpi_get_cpu_count(): Could not extract token\n");
        return -1;
    }

    int num = 0;

    do {
        if (strcmp(tok, "processor") == 0)
            num++;

        tok = strtok(NULL, delim);
    } while (tok != NULL);

    return num;
}

int dlt_kpi_get_cpu_count()
{
    if (dlt_kpi_cpu_count <= 0) {
        dlt_kpi_cpu_count = dlt_kpi_read_cpu_count();

        if (dlt_kpi_cpu_count <= 0) {
            fprintf(stderr, "Could not get CPU count\n");
            dlt_kpi_cpu_count = -1;
        }
    }

    return dlt_kpi_cpu_count;
}
