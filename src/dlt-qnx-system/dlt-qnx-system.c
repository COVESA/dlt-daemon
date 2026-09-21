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
 * \author Nguyen Dinh Thi <Thi.NguyenDinh@vn.bosch.com>
 *
 * \file: dlt-qnx-system.c
 * For further information see http://www.covesa.org/.
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <err.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <unistd.h>

#include "dlt.h"
#include "dlt-qnx-system.h"

DLT_DECLARE_CONTEXT(dltQnxSystem)

/* Global variables */
extern _Atomic bool g_inj_disable_slog2_cb;
extern _Atomic bool g_slog2_thread_alive;
volatile DltQnxSystemThreads g_threads;

#define INJECTION_SLOG2_ADAPTER     4096
#define MAX_THREAD_START_RETRIES    3

#define DATA_DISABLED   "00"
#define DATA_ENABLED    "01"

/* Function prototypes */
static void daemonize(void);
static void start_thread(void);
static void join_thread(void);
static int read_configuration_file(const char *file_name);
static int read_command_line(DltQnxSystemCliOptions *options, int argc, char *argv[]);
static int dlt_injection_cb(uint32_t service_id, void *data, uint32_t length);
static void init_configuration(void);
static void clean_up(void);

static DltQnxSystemConfiguration *g_dlt_qnx_conf;

int main(int argc, char* argv[])
{
    DltQnxSystemCliOptions options;
    int sigNo = 0;
    int ret = 0;
    sigset_t mask;

    if (read_command_line(&options, argc, argv) < 0)
    {
        fprintf(stderr, "Failed to read command line!\n");
        return -1;
    }

    if (read_configuration_file(options.configurationFileName) < 0)
    {
        fprintf(stderr, "Failed to read configuration file!\n");
        return -1;
    }

    if (options.daemonize > 0)
    {
        daemonize();
    }

    /* Point 2: Synchronized Signal Mask Setup before starting worker threads */
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGALRM);

    if (pthread_sigmask(SIG_BLOCK, &mask, NULL) != 0) {
        fprintf(stderr, "Failed to set pthread_sigmask!\n");
        return -1;
    }

    DLT_REGISTER_APP(g_dlt_qnx_conf->applicationId, "DLT QNX System");
    DLT_REGISTER_CONTEXT(dltQnxSystem, g_dlt_qnx_conf->applicationContextId,
            "Context of main dlt qnx system manager");
    dlt_register_injection_callback(&dltQnxSystem,
            INJECTION_SLOG2_ADAPTER, dlt_injection_cb);

    g_threads.main_thread = pthread_self();

    DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG, DLT_STRING("Launching logging thread."));
    
    int retry_count = 0;
    while (!g_slog2_thread_alive && retry_count < MAX_THREAD_START_RETRIES) {
        start_thread();
        if (!g_slog2_thread_alive) {
            retry_count++;
            DLT_LOG(dltQnxSystem, DLT_LOG_WARN,
                    DLT_STRING("Failed to start logging thread. Retrying..."),
                    DLT_INT(retry_count));
            sleep(1);
        }
    }

    if (!g_slog2_thread_alive) {
        DLT_LOG(dltQnxSystem, DLT_LOG_ERROR,
                DLT_STRING("Failed to start logging thread after maximum retries - "),
                DLT_INT(MAX_THREAD_START_RETRIES));
        clean_up();
        DLT_UNREGISTER_APP();
        return -1;
    }

    /* Synchronously wait for incoming signals (SIGINT, SIGTERM, etc.) */
    ret = sigwait(&mask, &sigNo);

    DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG,
            DLT_STRING("Received shutdown signal: "),
            DLT_INT(sigNo));

    /* Point 2 & 4: Stop worker lifecycle gracefully */
    g_slog2_thread_alive = false;
    join_thread();

    DLT_UNREGISTER_APP_FLUSH_BUFFERED_LOGS();
    clean_up();

    return (ret == 0) ? 0 : -1;
}

static void usage(char *prog_name)
{
    char version[255];
    dlt_get_version(version, 255);

    printf("Usage: %s [options]\n", prog_name);
    printf("Application to manage QNX system, such as:\n");
    printf("    - forward slogger2 messages from QNX to DLT .\n");
    printf("%s\n", version);
    printf("Options:\n");
    printf(" -d            Daemonize. Detach from terminal and run in background.\n");
    printf(" -c filename   Use configuration file. \n");
    printf("               Default: %s\n", DEFAULT_CONF_FILE);
    printf(" -h            This help message.\n");
}

static void init_cli_options(DltQnxSystemCliOptions *options)
{
    options->configurationFileName     = DEFAULT_CONF_FILE;
    options->daemonize                 = 0;
}

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

static void init_configuration(void)
{
    g_dlt_qnx_conf = calloc(1, sizeof(DltQnxSystemConfiguration));
    g_dlt_qnx_conf->applicationId          = strdup("QSYM");
    g_dlt_qnx_conf->applicationContextId   = strdup("QSYC");

    g_dlt_qnx_conf->qnxslogger2.enable     = 0;
    g_dlt_qnx_conf->qnxslogger2.contextId  = strdup("QSLA");
    g_dlt_qnx_conf->qnxslogger2.useOriginalTimestamp = 1;
}

static int read_configuration_file(const char *file_name)
{
    FILE *file;
    char *line;
    char *token;
    char *value;
    char *pch;
    int ret = 0;

    init_configuration();

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
            if (strcmp(token, "ApplicationId") == 0)
            {
                if (g_dlt_qnx_conf->applicationId)
                    free(g_dlt_qnx_conf->applicationId);

                g_dlt_qnx_conf->applicationId = (char *)malloc(DLT_ID_SIZE + 1);
                MALLOC_ASSERT(g_dlt_qnx_conf->applicationId);
                strncpy(g_dlt_qnx_conf->applicationId, value, DLT_ID_SIZE);
            }
            else if (strcmp(token, "ApplicationContextID") == 0)
            {
                if (g_dlt_qnx_conf->applicationContextId)
                    free(g_dlt_qnx_conf->applicationContextId);

                g_dlt_qnx_conf->applicationContextId = (char *)malloc(DLT_ID_SIZE + 1);
                MALLOC_ASSERT(g_dlt_qnx_conf->applicationContextId);
                strncpy(g_dlt_qnx_conf->applicationContextId, value, DLT_ID_SIZE);
            }
            else if (strcmp(token, "QnxSlogger2Enable") == 0)
            {
                g_dlt_qnx_conf->qnxslogger2.enable = atoi(value);
            }
            else if (strcmp(token, "QnxSlogger2ContextId") == 0)
            {
                if (g_dlt_qnx_conf->qnxslogger2.contextId)
                    free(g_dlt_qnx_conf->qnxslogger2.contextId);

                g_dlt_qnx_conf->qnxslogger2.contextId = (char *)malloc(DLT_ID_SIZE + 1);
                MALLOC_ASSERT(g_dlt_qnx_conf->qnxslogger2.contextId);
                strncpy(g_dlt_qnx_conf->qnxslogger2.contextId, value, DLT_ID_SIZE);
            }
            else if (strcmp(token, "QnxSlogger2UseOriginalTimestamp") == 0)
            {
                g_dlt_qnx_conf->qnxslogger2.useOriginalTimestamp = atoi(value);
            }
        }
    }

    fclose(file);
    free(value);
    free(token);
    free(line);

    return ret;
}

static void daemonize(void)
{
    pid_t pid = fork();

    if (pid == -1) {
        err(-1, "%s failed on fork()", __func__);
    }

    if (pid > 0) {
        exit(0);
    }

    if (setsid() == -1) {
        err(-1, "%s failed on setsid()", __func__);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int fd = open("/dev/null", O_RDWR);
    if (fd == -1) {
        err(-1, "%s failed on open() /dev/null", __func__);
    }

    if ((dup2(fd, STDIN_FILENO) == -1) ||
        (dup2(fd, STDOUT_FILENO) == -1) ||
        (dup2(fd, STDERR_FILENO) == -1)) {
        err(-1, "%s failed on dup2()", __func__);
    }

    signal(SIGCHLD, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
}

static void start_thread(void)
{
    if (g_dlt_qnx_conf->qnxslogger2.enable) {
        printf("Enable dlt_slogger2_adapter, start logging thread.\n");
        start_qnx_slogger2(g_dlt_qnx_conf);
    } else {
        printf("Disable dlt_slogger2_adapter, cannot start logging thread.\n");
    }
}

/* Point 4: Correct Joining & Context Unregister Sequence */
static void join_thread(void)
{
    if (g_threads.slog2_thread != 0) {
        (void) pthread_join(g_threads.slog2_thread, NULL);
        g_threads.slog2_thread = 0;
    }
    DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-qnx-system, thread exit complete."));
    DLT_UNREGISTER_CONTEXT(dltQnxSystem);
}

static int dlt_injection_cb(uint32_t service_id, void *data, uint32_t length)
{
    (void) length;
    DLT_LOG(dltQnxSystem, DLT_LOG_INFO,
            DLT_STRING("Injection received:"),
            DLT_INT32(service_id));

    if (service_id != INJECTION_SLOG2_ADAPTER)
        return -1;

    if (0 == strncmp((char*) data, DATA_DISABLED, sizeof(DATA_DISABLED)-1)) {
        g_inj_disable_slog2_cb = true;
        DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG,
            DLT_STRING("Disabling slog2 callback because of injection request."));
    }

    if (0 == strncmp((char*) data, DATA_ENABLED, sizeof(DATA_ENABLED)-1)) {
        g_inj_disable_slog2_cb = false;
        DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG,
            DLT_STRING("Enabling slog2 callback because of injection request."));
    }

    return 0;
}

static void clean_up(void)
{
    clean_qnx_slogger2();

    if (g_dlt_qnx_conf) {
        if (g_dlt_qnx_conf->applicationId)
            free(g_dlt_qnx_conf->applicationId);
        if (g_dlt_qnx_conf->applicationContextId)
            free(g_dlt_qnx_conf->applicationContextId);
        if (g_dlt_qnx_conf->qnxslogger2.contextId)
            free(g_dlt_qnx_conf->qnxslogger2.contextId);
        free(g_dlt_qnx_conf);
        g_dlt_qnx_conf = NULL;
    }
}
