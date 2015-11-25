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
 * \file dlt-kpi-common.c
 */

#include "dlt-kpi-common.h"

DltReturnValue dlt_kpi_read_file_compact(char *filename, char **target)
{
    char buffer[BUFFER_SIZE];
    int ret = dlt_kpi_read_file(filename, buffer, BUFFER_SIZE);
    if(ret < DLT_RETURN_OK)
        return ret;

    if((*target = malloc(strlen(buffer) + 1)) == NULL)
    {
        fprintf(stderr, "Out of memory!\n");
        return DLT_RETURN_ERROR;
    }

    memcpy(*target, buffer, strlen(buffer) + 1);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_read_file(char* filename, char* buffer, uint maxLength)
{
    if(filename == NULL || buffer == NULL)
    {
        fprintf(stderr, "Nullpointer parameter!\n");
        return DLT_RETURN_WRONG_PARAMETER;
    }

    FILE* file = fopen(filename, "r");
    if(file == NULL)
    {
        // fprintf(stderr, "Could not read file %s\n", filename);
        return DLT_RETURN_ERROR;
    }

    int buflen = fread(buffer, 1, maxLength-1, file);
    buffer[buflen] = '\0';

    fclose(file);

    return DLT_RETURN_OK;
}
