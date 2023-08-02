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
 * \file dlt-kpi-options.c
 */

#include "dlt-kpi.h"

/**
 * Print information how to use this program.
 */
void usage(char *prog_name)
{
    char version[255];
    dlt_get_version(version, 255);

    printf("Usage: %s [options]\n", prog_name);
    printf("Application to forward information from the /proc/ file system to DLT.\n");
    printf("%s\n", version);
    printf("Options:\n");
    /*printf(" -d             Daemonize. Detach from terminal and run in background.\n"); */
    printf(" -c filename    Use configuration file. \n");
    printf("                Default: %s\n", DEFAULT_CONF_FILE);
    printf(" -h             This help message.\n");
}

/**
 * Initialize command line options with default values.
 */
void dlt_kpi_init_cli_options(DltKpiOptions *options)
{
    options->configurationFileName = DEFAULT_CONF_FILE;
    options->customConfigFile = 0;
}

void dlt_kpi_free_cli_options(DltKpiOptions *options)
{
    if (options->customConfigFile)
        free(options->configurationFileName);
}

DltReturnValue dlt_kpi_read_command_line(DltKpiOptions *options, int argc, char **argv)
{
    if (options == NULL) {
        fprintf(stderr, "%s: Nullpointer parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    dlt_kpi_init_cli_options(options);
    int opt;

    while ((opt = getopt(argc, argv, "c:h")) != -1)
        switch (opt) {
        case 'c':
        {
            if ((options->configurationFileName = malloc(strlen(optarg) + 1)) == 0) {
                fprintf(stderr, "Out of memory!\n");
                return DLT_RETURN_ERROR;
            }

            strcpy(options->configurationFileName, optarg);     /* strcpy unritical here, because size matches exactly the size to be copied */
            options->customConfigFile = 1;
            break;
        }
        case 'h':
        {
            usage(argv[0]);
            exit(0);
            return -1;    /*for parasoft */
        }
        default:
        {
            fprintf(stderr, "Unknown option: %c\n", optopt);
            usage(argv[0]);
            return DLT_RETURN_ERROR;
        }
        }

    return DLT_RETURN_OK;
}

/**
 * Initialize configuration to default values.
 */
void dlt_kpi_init_configuration(DltKpiConfig *config)
{
    config->process_log_interval = 1000;
    config->irq_log_interval = 1000;
    config->log_level = DLT_LOG_DEFAULT;
}

/**
 * Read options from the configuration file
 */
DltReturnValue dlt_kpi_read_configuration_file(DltKpiConfig *config, char *file_name)
{
    FILE *file;
    char *line = NULL;
    char *token = NULL;
    char *value = NULL;
    char *pch, *strchk;
    int tmp;

    if ((config == NULL) || (file_name == NULL)) {
        fprintf(stderr, "%s: Nullpointer parameter!\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    dlt_kpi_init_configuration(config);

    file = fopen(file_name, "r");

    if (file == NULL) {
        fprintf(stderr, "%s: Could not open configuration file!\n", __func__);
        return DLT_RETURN_ERROR;
    }

    if (((line = malloc(COMMAND_LINE_SIZE)) == 0) ||
        ((token = malloc(COMMAND_LINE_SIZE)) == 0) ||
        ((value = malloc(COMMAND_LINE_SIZE)) == 0)) {
        fclose(file);
        free(line);
        free(token);
        free(value);

        fprintf(stderr, "%s: Out of memory!\n", __func__);
        return DLT_RETURN_ERROR;
    }

    while (fgets(line, COMMAND_LINE_SIZE, file) != NULL) {
        token[0] = '\0';
        value[0] = '\0';

        pch = strtok (line, " =\r\n");

        while (pch != NULL) {
            if (pch[0] == '#')
                break;

            if (token[0] == '\0') {
                strncpy(token, pch, COMMAND_LINE_SIZE - 1);
                token[COMMAND_LINE_SIZE - 1] = '\0';
            }
            else {
                strncpy(value, pch, COMMAND_LINE_SIZE - 1);
                value[COMMAND_LINE_SIZE - 1] = '\0';
                break;
            }

            pch = strtok(NULL, " =\r\n");
        }

        if ((token[0] != '\0') && (value[0] != '\0')) {
            if (strcmp(token, "process_interval") == '\0') {
                tmp = strtol(value, &strchk, 10);

                if ((strchk[0] == '\0') && (tmp > 0))
                    config->process_log_interval = tmp;
                else
                    fprintf(stderr, "Error reading configuration file: %s is not a valid value for %s\n", value, token);
            }
            else if (strcmp(token, "irq_interval") == '\0')
            {
                tmp = strtol(value, &strchk, 10);

                if ((strchk[0] == '\0') && (tmp > 0))
                    config->irq_log_interval = tmp;
                else
                    fprintf(stderr, "Error reading configuration file: %s is not a valid value for %s\n", value, token);
            }
            else if (strcmp(token, "check_interval") == '\0')
            {
                tmp = strtol(value, &strchk, 10);

                if ((strchk[0] == '\0') && (tmp > 0))
                    config->check_log_interval = tmp;
                else
                    fprintf(stderr, "Error reading configuration file: %s is not a valid value for %s\n", value, token);
            }
            else if (strcmp(token, "log_level") == '\0')
            {
                tmp = strtol(value, &strchk, 10);

                if ((strchk[0] == '\0') && (tmp >= -1) && (tmp <= 6))
                    config->log_level = tmp;
                else
                    fprintf(stderr, "Error reading configuration file: %s is not a valid value for %s\n", value, token);
            }
        }
    }

    fclose(file);
    free(value);
    free(token);
    free(line);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_init(int argc, char **argv, DltKpiConfig *config)
{
    DltKpiOptions options;

    DltReturnValue ret;

    if (config == NULL) {
        fprintf(stderr, "%s: Invalid Parameter!", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if ((ret = dlt_kpi_read_command_line(&options, argc, argv)) < DLT_RETURN_OK) {
        fprintf(stderr, "Failed to read command line!");
        return ret;
    }

    if ((ret = dlt_kpi_read_configuration_file(config, options.configurationFileName)) < DLT_RETURN_OK) {
        fprintf(stderr, "Failed to read configuration file!");
        return ret;
    }

    dlt_kpi_free_cli_options(&options);

    return DLT_RETURN_OK;
}
