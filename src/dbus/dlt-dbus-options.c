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
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-dbus-options.c
 */



#include "dlt-dbus.h"

#include <stdlib.h>
#include <string.h>

/**
 * Print information how to use this program.
 */
void usage(char *prog_name)
{
    char version[255];
    dlt_get_version(version, 255);

    printf("Usage: %s [options]\n", prog_name);
    printf("Application to forward dbus messages to DLT.\n");
    printf("%s\n", version);
    printf("Options:\n");
    printf(" -d             Daemonize. Detach from terminal and run in background.\n");
    printf(" -c filename    Use configuration file. \n");
    printf(" -a apid        Used application id. \n");
    printf("                Default: %s\n", DEFAULT_CONF_FILE);
    printf(" -b type        Used bus type. \n");
    printf("                Session = 0, System = 1.\n");
    printf(" -h             This help message.\n");
}

/**
 * Initialize command line options with default values.
 */
void init_cli_options(DltDBusCliOptions *options)
{
    options->ConfigurationFileName = DEFAULT_CONF_FILE;
    options->ApplicationId = 0;
    options->BusType = 0;
    options->Daemonize = 0;
}

/**
 * Read command line options and set the values in provided structure
 */
int read_command_line(DltDBusCliOptions *options, int argc, char *argv[])
{
    init_cli_options(options);
    int opt;

    while ((opt = getopt(argc, argv, "c:b:a:hd")) != -1)
        switch (opt) {
        case 'd':
        {
            options->Daemonize = 1;
            break;
        }
        case 'b':
        {
            options->BusType = malloc(strlen(optarg) + 1);
            MALLOC_ASSERT(options->BusType);
            strcpy(options->BusType, optarg); /* strcpy uncritical here, because size matches exactly the size to be copied */
            break;
        }
        case 'a':
        {
            options->ApplicationId = malloc(strlen(optarg) + 1);
            MALLOC_ASSERT(options->ApplicationId);
            strcpy(options->ApplicationId, optarg); /* strcpy uncritical here, because size matches exactly the size to be copied */
            break;
        }
        case 'c':
        {
            options->ConfigurationFileName = malloc(strlen(optarg) + 1);
            MALLOC_ASSERT(options->ConfigurationFileName);
            strcpy(options->ConfigurationFileName, optarg); /* strcpy uncritical here, because size matches exactly the size to be copied */
            break;
        }
        case 'h':
        {
            usage(argv[0]);
            exit(0);
            return -1;                    /*for parasoft */
        }
        default:
        {
            fprintf(stderr, "Unknown option '%c'\n", optopt);
            usage(argv[0]);
            return -1;
        }
        }

    return 0;
}

/**
 * Initialize configuration to default values.
 */
void init_configuration(DltDBusConfiguration *config)
{
    /* Common */
    config->ApplicationId = "IPC0";

    /* DBus */
    config->DBus.ContextId = "ALL";
    config->DBus.BusType = 0;
    config->DBus.FilterCount = 0;

}

/**
 * Read options from the configuration file
 */
int read_configuration_file(DltDBusConfiguration *config, char *file_name)
{
    FILE *file;
    char *line, *token, *value, *filter, *pch;
    int ret = 0;
    char *filterBegin, *filterEnd;

    init_configuration(config);

    file = fopen(file_name, "r");

    if (file == NULL) {
        fprintf(stderr, "dlt-dbus-options, could not open configuration file.\n");
        return -1;
    }

    line = malloc(MAX_LINE);
    token = malloc(MAX_LINE);
    value = malloc(MAX_LINE);
    filter = malloc(MAX_LINE);

    MALLOC_ASSERT(line);
    MALLOC_ASSERT(token);
    MALLOC_ASSERT(value);
    MALLOC_ASSERT(filter);

    while (fgets(line, MAX_LINE, file) != NULL) {
        token[0] = 0;
        value[0] = 0;
        filter[0] = 0;

        filterBegin = strchr(line, '=');
        filterEnd = strpbrk (line, "\r\n");

        if (filterBegin) {
            if (filterEnd && (filterEnd > filterBegin)) {
                strncpy(filter, filterBegin + 1, filterEnd - filterBegin - 1);
                filter[filterEnd - filterBegin - 1] = 0;
            }
            else {
                strcpy(filter, filterBegin + 1);
            }
        }

        pch = strtok (line, " =\r\n");

        while (pch != NULL) {
            if (pch[0] == '#')
                break;

            if (token[0] == 0) {
                strncpy(token, pch, MAX_LINE - 1);
                token[MAX_LINE - 1] = 0;
            }
            else {
                strncpy(value, pch, MAX_LINE);
                value[MAX_LINE - 1] = 0;
                break;
            }

            pch = strtok (NULL, " =\r\n");
        }

        if (token[0] && value[0]) {
            /* Common */
            if (strcmp(token, "ApplicationId") == 0) {
                config->ApplicationId = malloc(strlen(value) + 1);
                MALLOC_ASSERT(config->ApplicationId);
                strcpy(config->ApplicationId, value); /* strcpy unritical here, because size matches exactly the size to be copied */
            }
            /* ContextId */
            else if (strcmp(token, "ContextId") == 0)
            {
                config->DBus.ContextId = malloc(strlen(value) + 1);
                MALLOC_ASSERT(config->DBus.ContextId);
                strcpy(config->DBus.ContextId, value); /* strcpy unritical here, because size matches exactly the size to be copied */
            }
            /* BusType */
            else if (strcmp(token, "BusType") == 0)
            {
                config->DBus.BusType = malloc(strlen(value) + 1);
                MALLOC_ASSERT(config->DBus.BusType);
                strcpy(config->DBus.BusType, value); /* strcpy unritical here, because size matches exactly the size to be copied */
            }
            /* BusType */
            else if (strcmp(token, "FilterMatch") == 0)
            {
                if (config->DBus.FilterCount < DLT_DBUS_FILTER_MAX) {
                    config->DBus.FilterMatch[config->DBus.FilterCount] = malloc(strlen(filter) + 1);
                    MALLOC_ASSERT(config->DBus.FilterMatch[config->DBus.FilterCount]);
                    strcpy(config->DBus.FilterMatch[config->DBus.FilterCount], filter);
                    config->DBus.FilterCount++;
                }
            }
        }
    }

    fclose(file);
    free(value);
    free(token);
    free(filter);
    free(line);
    return ret;
}
