/**
 * Copyright (C) 2020 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * DLT QNX system functionality source file.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Nguyen Dinh Thi <Thi.NguyenDinh@vn.bosch.com>
 *
 * \file: dlt-qnx-system.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */


#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <err.h>

#include "dlt.h"
#include "dlt-qnx-system.h"

DLT_DECLARE_CONTEXT(dltQnxSystem)

/* Global variables */
volatile DltQnxSystemThreads g_threads;

/* Function prototype */
static void daemonize();
static void start_threads(DltQnxSystemConfiguration *config);
static void join_threads();
static int read_configuration_file(DltQnxSystemConfiguration *config,
                            const char *file_name);
static int read_command_line(DltQnxSystemCliOptions *options, int argc, char *argv[]);

int main(int argc, char* argv[])
{
    DltQnxSystemCliOptions options;
    DltQnxSystemConfiguration config;
    int sigNo = 0;
    int ret = 0;
    sigset_t mask;
    int i;

    if (read_command_line(&options, argc, argv) < 0)
    {
        fprintf(stderr, "Failed to read command line!\n");
        return -1;
    }

    if (read_configuration_file(&config, options.configurationFileName) < 0)
    {
        fprintf(stderr, "Failed to read configuration file!\n");
        return -1;
    }

    if (options.daemonize > 0)
    {
        daemonize();
    }

    DLT_REGISTER_APP(config.applicationId, "DLT QNX System");
    DLT_REGISTER_CONTEXT(dltQnxSystem, config.applicationContextId,
            "Context of main dlt qnx system manager");

    DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG,
            DLT_STRING("Setting signals wait for abnormal exit"));

    g_threads.mainThread = pthread_self();

    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGALRM);
    if (pthread_sigmask(SIG_BLOCK, &mask, NULL) == -1) {
        DLT_LOG(dltQnxSystem, DLT_LOG_WARN,
                DLT_STRING("Failed to block signals!"));
        DLT_UNREGISTER_APP();
        return -1;
    }

    DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG, DLT_STRING("Launching threads."));
    start_threads(&config);

    ret = sigwait(&mask, &sigNo);

    for (i = 0; i < MAX_THREADS; i++) {
        pthread_cancel(g_threads.threads[i]);
    }
    join_threads();

    if (ret != 0) {
        DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG,
                DLT_STRING("sigwait failed with error: "),
                DLT_INT(ret));
        DLT_UNREGISTER_APP();
        return -1;
    }

    DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG,
            DLT_STRING("Received signal: "),
            DLT_STRING(strsignal(sigNo)));

    DLT_UNREGISTER_APP_FLUSH_BUFFERED_LOGS();
    return 0;

}

/**
 * Print information how to use this program.
 */
static void usage(char *prog_name)
{
    char version[255];
    dlt_get_version(version, 255);

    printf("Usage: %s [options]\n", prog_name);
    printf("Application to manage QNX system, such as:\n");
    printf("    - forward slogger2 messages from QNX to DLT) .\n");
    printf("%s\n", version);
    printf("Options:\n");
    printf(" -d           Daemonize. Detach from terminal and run in background.\n");
    printf(" -c filename  Use configuration file. \n");
    printf("              Default: %s\n", DEFAULT_CONF_FILE);
    printf(" -h           This help message.\n");
}

/**
 * Initialize command line options with default values.
 */
static void init_cli_options(DltQnxSystemCliOptions *options)
{
    options->configurationFileName     = DEFAULT_CONF_FILE;
    options->daemonize                 = 0;
}

/**
 * Read command line options and set the values in provided structure
 */
static int read_command_line(DltQnxSystemCliOptions *options, int argc, char *argv[])
{
    init_cli_options(options);
    int opt;

    while ((opt = getopt(argc, argv, "c:hd")) != -1)
    {
        switch (opt) {
            case 'd':
            {
                options->daemonize = 1;
                break;
            }
            case 'c':
            {
                options->configurationFileName = (char *)malloc(strlen(optarg)+1);
                MALLOC_ASSERT(options->configurationFileName);
                /**
                 * strcpy unritical here, because size matches exactly the size
                 * to be copied
                 */
                strcpy(options->configurationFileName, optarg);
                break;
            }
            case 'h':
            {
                usage(argv[0]);
                exit(0);
                return -1;
            }
            default:
            {
                fprintf(stderr, "Unknown option '%c'\n", optopt);
                usage(argv[0]);
                return -1;
            }
        }
    }
    return 0;
}

/**
 * Initialize configuration to default values.
 */
static void init_configuration(DltQnxSystemConfiguration *config)
{
    /* Common */
    config->applicationId          = "QSYM";
    config->applicationContextId   = "QSYC";

    /* Slogger2 */
    config->qnxslogger2.enable     = 0;
    config->qnxslogger2.contextId  = "QSLA";
    config->qnxslogger2.useOriginalTimestamp = 1;
}

/**
 * Read options from the configuration file
 */
static int read_configuration_file(DltQnxSystemConfiguration *config,
                             const char *file_name)
{
    FILE *file;
    char *line;
    char *token;
    char *value;
    char *pch;
    int ret = 0;

    init_configuration(config);

    file = fopen(file_name, "r");

    if (file == NULL)
    {
        fprintf(stderr,
                "dlt-qnx-system, could not open configuration file.\n");
        return -1;
    }

    line = malloc(MAX_LINE);
    token = malloc(MAX_LINE);
    value = malloc(MAX_LINE);

    MALLOC_ASSERT(line);
    MALLOC_ASSERT(token);
    MALLOC_ASSERT(value);

    while (fgets(line, MAX_LINE, file) != NULL)
    {
        token[0] = 0;
        value[0] = 0;

        pch = strtok(line, " =\r\n");
        while (pch != NULL)
        {
            if (pch[0] == '#')
            {
                break;
            }

            if (token[0] == 0)
            {
                strncpy(token, pch, MAX_LINE-1);
                token[MAX_LINE-1] = 0;
            }
            else
            {
                strncpy(value, pch, MAX_LINE);
                value[MAX_LINE-1] = 0;
                break;
            }

            pch = strtok(NULL, " =\r\n");
        }

        if (token[0] && value[0])
        {
            /* Common */
            if (strcmp(token, "ApplicationId") == 0)
            {
                config->applicationId = (char *)malloc(strlen(value) + 1);
                MALLOC_ASSERT(config->applicationId);
                /**
                 * strcpy unritical here, because size matches exactly the
                 * size to be copied
                 */
                strcpy(config->applicationId, value);
            }
            else if (strcmp(token, "ApplicationContextID") == 0)
            {
                config->applicationContextId = (char *)malloc(strlen(value) + 1);
                MALLOC_ASSERT(config->applicationContextId);
                /**
                * strcpy unritical here, because size matches exactly
                * the size to be copied
                */
                strcpy(config->applicationContextId, value);
            }
            /* Slogger2 */
            else if (strcmp(token, "QnxSlogger2Enable") == 0)
            {
                config->qnxslogger2.enable = atoi(value);
            }
            else if (strcmp(token, "QnxSlogger2ContextId") == 0)
            {
                config->qnxslogger2.contextId = (char *)malloc(strlen(value) + 1);
                MALLOC_ASSERT(config->qnxslogger2.contextId);
                /**
                 * strcpy unritical here, because size matches exactly
                 * the size to be copied
                 */
                strcpy(config->qnxslogger2.contextId, value);
            }
            else if (strcmp(token, "QnxSlogger2UseOriginalTimestamp") == 0)
            {
                config->qnxslogger2.useOriginalTimestamp = atoi(value);
            }
            else
            {
                /* Do nothing */
            }
        }
    }

    fclose(file);
    file = NULL;

    free(value);
    value = NULL;

    free(token);
    token = NULL;

    free(line);
    line = NULL;

    return ret;
}

static void daemonize()
{
    pid_t pid = fork();

    if (pid == -1) {
        err(-1, "%s failed on fork()", __func__);
    }

    if (pid > 0) { /* parent process*/
        exit(0);
    }

    /* Create a new process group */
    if (setsid() == -1) {
        err(-1, "%s failed on setsid()", __func__);
    }

    /* Point std(in,out,err) to /dev/null */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int fd = open("/dev/null", O_RDWR);
    if (fd == -1) {
        err(-1, "%s failed on open() /dev/null", __func__);
    }

    if ((dup2(fd, STDIN_FILENO) == -1) ||
        (dup2(fd, STDOUT_FILENO) == -1 ) ||
        (dup2(fd, STDERR_FILENO) == -1 )) {
        err(-1, "%s failed on dup2()", __func__);
    }

    /**
     * Ignore signals related to child processes and
     * terminal handling.
     */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
}

static void start_threads(DltQnxSystemConfiguration *config)
{
    int i = 0;

    /* Check parameter */
    if (!config)
    {
        return;
    }

    DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-qnx-system, start threads"));

    g_threads.count = 0;
    g_threads.shutdown = 0;

    for (i = 0; i < MAX_THREADS; i++)
    {
        g_threads.threads[i] = 0;
    }

    if (config->qnxslogger2.enable)
    {
        start_qnx_slogger2(config);
    }
}

/**
 * Wait for threads to exit.
 */
static void join_threads()
{
    int i = 0;
    DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG,
        DLT_STRING("dlt-qnx-system, waiting for threads to exit"));

    if (g_threads.count < 1)
    {
        DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG,
                DLT_STRING("dlt-qnx-system, no threads, waiting for signal."));
    }
    else
    {
        DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG,
                DLT_STRING("dlt-qnx-system, thread count: "),
                DLT_INT(g_threads.count));

        for (i = 0; i < g_threads.count; i++)
        {
            pthread_join(g_threads.threads[i], NULL);
            DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG,
                    DLT_STRING("dlt-qnx-system, thread exit: "),
                    DLT_INT(g_threads.threads[i]));
        }
    }

    DLT_UNREGISTER_CONTEXT(dltQnxSystem);
}
