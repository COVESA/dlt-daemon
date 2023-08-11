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
 * \author Lassi Marttala <lassi.lm.marttala@partner.bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-system-options.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-system-options.c                                          **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Lassi Marttala <lassi.lm.marttala@partner.bmw.de>             **
**              Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
**                                                                            **
**  PURPOSE   :                                                               **
**                                                                            **
**  REMARKS   :                                                               **
**                                                                            **
**  PLATFORM DEPENDANT [yes/no]: yes                                          **
**                                                                            **
**  TO BE CHANGED BY USER [yes/no]: no                                        **
**                                                                            **
*******************************************************************************/

/*******************************************************************************
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  lm          Lassi Marttala             BMW                                **
**  aw          Alexander Wenzel           BMW                                **
*******************************************************************************/

#include "dlt-system.h"

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
    printf("Application to forward syslog messages to DLT, transfer system information, logs and files.\n");
    printf("%s\n", version);
    printf("Options:\n");
    printf(" -d             Daemonize. Detach from terminal and run in background.\n");
    printf(" -c filename    Use configuration file. \n");
    printf("                Default: %s\n", DEFAULT_CONF_FILE);
    printf(" -h             This help message.\n");
}

/**
 * Initialize command line options with default values.
 */
void init_cli_options(DltSystemCliOptions *options)
{
    options->ConfigurationFileName = strdup(DEFAULT_CONF_FILE);
    options->freeConfigFileName = 0;
    options->Daemonize = 0;
}

/**
 * Read command line options and set the values in provided structure
 */
int read_command_line(DltSystemCliOptions *options, int argc, char *argv[])
{
    init_cli_options(options);
    int opt;

    while ((opt = getopt(argc, argv, "c:hd")) != -1)
        switch (opt) {
        case 'd':
        {
            options->Daemonize = 1;
            break;
        }
        case 'c':
        {
            options->ConfigurationFileName = malloc(strlen(optarg) + 1);
            options->freeConfigFileName = 1;
            MALLOC_ASSERT(options->ConfigurationFileName);
            strcpy(options->ConfigurationFileName, optarg);     /* strcpy unritical here, because size matches exactly the size to be copied */
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
void init_configuration(DltSystemConfiguration *config)
{
    int i = 0;

    /* Common */
    strncpy(config->ApplicationId, "SYS", DLT_ID_SIZE);

    /* Shell */
    config->Shell.Enable = 0;

    /* Syslog */
    config->Syslog.Enable = 0;
    strncpy(config->Syslog.ContextId, "SYSL", DLT_ID_SIZE);
    config->Syslog.Port = 47111;

    /* Journal */
    config->Journal.Enable = 0;
    strncpy(config->Journal.ContextId, "JOUR", DLT_ID_SIZE);
    config->Journal.CurrentBoot = 1;
    config->Journal.Follow = 0;
    config->Journal.MapLogLevels = 1;
    config->Journal.UseOriginalTimestamp = 1;

    /* File transfer */
    config->Filetransfer.Enable = 0;
    strncpy(config->Filetransfer.ContextId, "FILE", DLT_ID_SIZE);
    config->Filetransfer.TimeStartup = 30;
    config->Filetransfer.TimeoutBetweenLogs = 10;
    config->Filetransfer.Count = 0;

    for (i = 0; i < DLT_SYSTEM_LOG_DIRS_MAX; i++) {
        config->Filetransfer.Directory[i] = NULL;
        config->Filetransfer.Compression[i] = 0;
        config->Filetransfer.CompressionLevel[i] = 5;
    }

    /* Log file */
    config->LogFile.Enable = 0;
    config->LogFile.Count = 0;

    for (i = 0; i < DLT_SYSTEM_LOG_FILE_MAX; i++) {
        strncpy(config->LogFile.ContextId[i], "\0", DLT_ID_SIZE);
        config->LogFile.Filename[i] = NULL;
        config->LogFile.Mode[i] = 0;
        config->LogFile.TimeDelay[i] = 0;
    }

    /* Log process */
    config->LogProcesses.Enable = 0;
    strncpy(config->LogProcesses.ContextId, "PROC", DLT_ID_SIZE);
    config->LogProcesses.Count = 0;

    for (i = 0; i < DLT_SYSTEM_LOG_PROCESSES_MAX; i++) {
        config->LogProcesses.Name[i] = NULL;
        config->LogProcesses.Filename[i] = NULL;
        config->LogProcesses.Mode[i] = 0;
        config->LogProcesses.TimeDelay[i] = 0;
    }
}

/**
 * Read options from the configuration file
 */
int read_configuration_file(DltSystemConfiguration *config, char *file_name)
{
    FILE *file;
    char *line, *token, *value, *pch;
    int ret = 0;

    init_configuration(config);

    file = fopen(file_name, "r");

    if (file == NULL) {
        fprintf(stderr, "dlt-system-options, could not open configuration file.\n");
        return -1;
    }

    line = malloc(MAX_LINE);
    token = malloc(MAX_LINE);
    value = malloc(MAX_LINE);

    MALLOC_ASSERT(line);
    MALLOC_ASSERT(token);
    MALLOC_ASSERT(value);

    while (fgets(line, MAX_LINE, file) != NULL) {
        token[0] = 0;
        value[0] = 0;

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
                strncpy(config->ApplicationId, value, DLT_ID_SIZE);
            }

            /* Shell */
            else if (strcmp(token, "ShellEnable") == 0)
            {
                config->Shell.Enable = atoi(value);
            }

            /* Syslog */
            else if (strcmp(token, "SyslogEnable") == 0)
            {
                config->Syslog.Enable = atoi(value);
            }
            else if (strcmp(token, "SyslogContextId") == 0)
            {
                strncpy(config->Syslog.ContextId, value, DLT_ID_SIZE);
            }
            else if (strcmp(token, "SyslogPort") == 0)
            {
                config->Syslog.Port = atoi(value);
            }

            /* Journal */
            else if (strcmp(token, "JournalEnable") == 0)
            {
                config->Journal.Enable = atoi(value);
            }
            else if (strcmp(token, "JournalContextId") == 0)
            {
                strncpy(config->Journal.ContextId, value, DLT_ID_SIZE);
            }
            else if (strcmp(token, "JournalCurrentBoot") == 0)
            {
                config->Journal.CurrentBoot = atoi(value);
            }
            else if (strcmp(token, "JournalFollow") == 0)
            {
                config->Journal.Follow = atoi(value);
            }
            else if (strcmp(token, "JournalMapLogLevels") == 0)
            {
                config->Journal.MapLogLevels = atoi(value);
            }
            else if (strcmp(token, "JournalUseOriginalTimestamp") == 0)
            {
                config->Journal.UseOriginalTimestamp = atoi(value);
            }

            /* File transfer */
            else if (strcmp(token, "FiletransferEnable") == 0)
            {
                config->Filetransfer.Enable = atoi(value);
            }
            else if (strcmp(token, "FiletransferContextId") == 0)
            {
                strncpy(config->Filetransfer.ContextId, value, DLT_ID_SIZE);
            }
            else if (strcmp(token, "FiletransferTimeStartup") == 0)
            {
                config->Filetransfer.TimeStartup = atoi(value);
            }
            else if (strcmp(token, "FiletransferTimeoutBetweenLogs") == 0)
            {
                config->Filetransfer.TimeoutBetweenLogs = atoi(value);
            }
            else if (strcmp(token, "FiletransferDirectory") == 0)
            {
                config->Filetransfer.Directory[config->Filetransfer.Count] = malloc(strlen(value) + 1);
                MALLOC_ASSERT(config->Filetransfer.Directory[config->Filetransfer.Count]);
                strcpy(config->Filetransfer.Directory[config->Filetransfer.Count], value); /* strcpy unritical here, because size matches exactly the size to be copied */
            }
            else if (strcmp(token, "FiletransferCompression") == 0)
            {
                config->Filetransfer.Compression[config->Filetransfer.Count] = atoi(value);
            }
            else if (strcmp(token, "FiletransferCompressionLevel") == 0)
            {
                config->Filetransfer.CompressionLevel[config->Filetransfer.Count] = atoi(value);

                if (config->Filetransfer.Count < (DLT_SYSTEM_LOG_DIRS_MAX - 1)) {
                    config->Filetransfer.Count++;
                }
                else {
                    fprintf(stderr,
                            "Too many file transfer directories configured. Maximum: %d\n",
                            DLT_SYSTEM_LOG_DIRS_MAX);
                    ret = -1;
                    break;
                }
            }

            /* Log files */
            else if (strcmp(token, "LogFileEnable") == 0)
            {
                config->LogFile.Enable = atoi(value);
            }
            else if (strcmp(token, "LogFileFilename") == 0)
            {
                config->LogFile.Filename[config->LogFile.Count] = malloc(strlen(value) + 1);
                MALLOC_ASSERT(config->LogFile.Filename[config->LogFile.Count]);
                strcpy(config->LogFile.Filename[config->LogFile.Count], value); /* strcpy unritical here, because size matches exactly the size to be copied */
            }
            else if (strcmp(token, "LogFileMode") == 0)
            {
                config->LogFile.Mode[config->LogFile.Count] = atoi(value);
            }
            else if (strcmp(token, "LogFileTimeDelay") == 0)
            {
                config->LogFile.TimeDelay[config->LogFile.Count] = atoi(value);
            }
            else if (strcmp(token, "LogFileContextId") == 0)
            {
                strncpy(config->LogFile.ContextId[config->LogFile.Count], value, DLT_ID_SIZE);

                if (config->LogFile.Count < (DLT_SYSTEM_LOG_FILE_MAX - 1)) {
                    config->LogFile.Count++;
                }
                else {
                    fprintf(stderr,
                            "Too many log files configured. Maximum: %d\n",
                            DLT_SYSTEM_LOG_FILE_MAX);
                    ret = -1;
                    break;
                }
            }

            /* Log Processes */
            else if (strcmp(token, "LogProcessesEnable") == 0)
            {
                config->LogProcesses.Enable = atoi(value);
            }
            else if (strcmp(token, "LogProcessesContextId") == 0)
            {
                strncpy(config->LogProcesses.ContextId, value, DLT_ID_SIZE);
            }
            else if (strcmp(token, "LogProcessName") == 0)
            {
                config->LogProcesses.Name[config->LogProcesses.Count] = malloc(strlen(value) + 1);
                MALLOC_ASSERT(config->LogProcesses.Name[config->LogProcesses.Count]);
                strcpy(config->LogProcesses.Name[config->LogProcesses.Count], value); /* strcpy unritical here, because size matches exactly the size to be copied */
            }
            else if (strcmp(token, "LogProcessFilename") == 0)
            {
                config->LogProcesses.Filename[config->LogProcesses.Count] = malloc(strlen(value) + 1);
                MALLOC_ASSERT(config->LogProcesses.Filename[config->LogProcesses.Count]);
                strcpy(config->LogProcesses.Filename[config->LogProcesses.Count], value); /* strcpy unritical here, because size matches exactly the size to be copied */
            }
            else if (strcmp(token, "LogProcessMode") == 0)
            {
                config->LogProcesses.Mode[config->LogProcesses.Count] = atoi(value);
            }
            else if (strcmp(token, "LogProcessTimeDelay") == 0)
            {
                config->LogProcesses.TimeDelay[config->LogProcesses.Count] = atoi(value);

                if (config->LogProcesses.Count < (DLT_SYSTEM_LOG_PROCESSES_MAX - 1)) {
                    config->LogProcesses.Count++;
                }
                else {
                    fprintf(stderr,
                            "Too many processes to log configured. Maximum: %d\n",
                            DLT_SYSTEM_LOG_PROCESSES_MAX);
                    ret = -1;
                    break;
                }
            }
        }
    }

    fclose(file);
    free(value);
    free(token);
    free(line);
    return ret;
}

void cleanup_config(DltSystemConfiguration *config, DltSystemCliOptions *options)
{
    /* command line options */
    if ((options->ConfigurationFileName) != NULL && options->freeConfigFileName)
    {
        free(options->ConfigurationFileName);
        options->ConfigurationFileName = NULL;
    }

    /* File transfer */
    for(int i = 0 ; i < DLT_SYSTEM_LOG_DIRS_MAX ; i++)
    {
        if ((config->Filetransfer.Directory[i]) != NULL)
        {
            free(config->Filetransfer.Directory[i]);
            config->Filetransfer.Directory[i] = NULL;
        }
    }

    /* Log files */
    for(int i = 0 ; i < DLT_SYSTEM_LOG_FILE_MAX ; i++)
    {
        if ((config->LogFile.Filename[i]) != NULL)
        {
            free(config->LogFile.Filename[i]);
            config->LogFile.Filename[i] = NULL;
        }
    }

    /* Log Processes */
    for(int i = 0 ; i < DLT_SYSTEM_LOG_PROCESSES_MAX ; i++)
    {
        if ((config->LogProcesses.Filename[i]) != NULL)
        {
            free(config->LogProcesses.Filename[i]);
            config->LogProcesses.Filename[i] = NULL;
        }
        if ((config->LogProcesses.Name[i]) != NULL)
        {
            free(config->LogProcesses.Name[i]);
            config->LogProcesses.Name[i] = NULL;
        }
    }
}
