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
 * For further information see http://www.covesa.org/.
 * @licence end@
 */
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <err.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include "dlt.h"
#include "dlt-qnx-system.h"

DLT_DECLARE_CONTEXT(dltQnxSystem)

/* Global variables */
static bool g_slog2_runtime_disabled = true;
DltQnxSystemThreads g_threads;
#define INJECTION_SLOG2_ADAPTER     4096
#define DATA_LENGTH     2
#define DATA_DISABLED   "00"
#define DATA_ENABLED    "01"

/* Function prototype */
static void daemonize();
static void start_thread();
static void join_thread();
static int read_configuration_file(const char *file_name);
static int read_command_line(DltQnxSystemCliOptions *options, int argc, char *argv[]);

static int dlt_injection_cb(uint32_t service_id, void *data, uint32_t length);
/**
 * \brief Trims leading and trailing whitespace characters from a string.
 *
 * \note This function modifies the input buffer in-place by writing a null
 *       terminator ('\0') after the last non-whitespace character.
 *
 * \param s Pointer to the null-terminated string to be trimmed.
 * \return char* A pointer to the first non-whitespace character within the
 *         original input buffer, or NULL if the input string 's' is NULL.
 */
static char *trim_whitespace(char *s)
{
    char *end;
    if (!s)
        return NULL;
    /* Trim leading space */
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0')
        return s;
    /* Trim trailing space */
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return s;
}
/**
 * \brief Check whether the slog2 adapter is currently disabled.
 *
 * \return true if disabled, false if enabled.
 */
bool dlt_slog2_is_disabled(void)
{
    bool disabled;
    pthread_mutex_lock(&g_threads.lock);
    disabled = g_slog2_runtime_disabled;
    pthread_mutex_unlock(&g_threads.lock);
    return disabled;
}
/**
 * \brief Set the slog2 adapter runtime disabled state.
 *
 * When enabling (disabled == false), broadcasts on the condition
 * variable to wake any threads waiting for the adapter to be enabled.
 *
 * \param disabled  true to disable, false to enable.
 */
void dlt_slog2_set_disabled(bool disabled)
{
    pthread_mutex_lock(&g_threads.lock);
    g_slog2_runtime_disabled = disabled;
    /* If enabling, wake any waiters */
    if (!disabled) {
        pthread_cond_broadcast(&g_threads.cond);
    }
    pthread_mutex_unlock(&g_threads.lock);
}
/*
 * Global configuration pointer for DLT QNX System.
 * Lifecycle: allocated in init_configuration/read_configuration_file, freed in clean_up().
 * Only valid after successful configuration load and before clean_up().
 */
static DltQnxSystemConfiguration *g_dlt_qnx_conf;

static void init_configuration();
static void clean_up();
/**
 * \brief Initialize thread management structure, mutex, and condition variable.
 *
 * Must be called once before any other dlt_threads/dlt_set/dlt_get functions.
 * Exits the process on failure.
 */
int dlt_threads_init(void)
{
    memset(&g_threads, 0, sizeof(g_threads));
    int ret = 0;
    ret = pthread_mutex_init(&g_threads.lock, NULL);
    if (ret != 0) {
        fprintf(stderr, "pthread_mutex_init failed: %s\n", strerror(ret));
        return ret;
    }
    pthread_condattr_t cond_attr;
    ret = pthread_condattr_init(&cond_attr);
    if (ret != 0) {
        fprintf(stderr, "pthread_condattr_init failed: %s\n", strerror(ret));
        return ret;
    }
    ret = pthread_condattr_setclock(&cond_attr, CLOCK_MONOTONIC);
    if (ret != 0) {
        fprintf(stderr, "pthread_condattr_setclock failed: %s\n", strerror(ret));
        pthread_condattr_destroy(&cond_attr);
        return ret;
    }
    ret = pthread_cond_init(&g_threads.cond, &cond_attr);
    pthread_condattr_destroy(&cond_attr);
    if (ret != 0) {
        fprintf(stderr, "pthread_cond_init failed: %s\n", strerror(ret));
        return ret;
    }
    return ret;
}
/**
 * \brief Destroy thread management mutex and condition variable.
 *
 * Must be called during shutdown after all threads have been joined.
 */
int dlt_threads_destroy(void)
{
    int ret = 0;
    ret = pthread_mutex_destroy(&g_threads.lock);
    if (ret != 0) {
        fprintf(stderr, "pthread_mutex_destroy failed: %s\n", strerror(ret));
        return ret;
    }
    ret = pthread_cond_destroy(&g_threads.cond);
    if (ret != 0) {
        fprintf(stderr, "pthread_cond_destroy failed: %s\n", strerror(ret));
        return ret;
    }
    return ret;
}
/**
 * \brief Store the main thread ID in a thread-safe manner.
 *
 * \param tid  Thread ID of the main thread.
 */
void dlt_set_main_thread(pthread_t tid)
{
    pthread_mutex_lock(&g_threads.lock);
    g_threads.main_thread = tid;
    pthread_mutex_unlock(&g_threads.lock);
}
/**
 * \brief Retrieve the main thread ID in a thread-safe manner.
 *
 * \return Thread ID of the main thread.
 */
pthread_t dlt_get_main_thread(void)
{
    pthread_t tid;
    pthread_mutex_lock(&g_threads.lock);
    tid = g_threads.main_thread;
    pthread_mutex_unlock(&g_threads.lock);
    return tid;
}
/**
 * \brief Store the slog2 logging thread ID in a thread-safe manner.
 *
 * \param tid  Thread ID of the slog2 thread, or 0 to clear.
 */
void dlt_set_slog2_thread(pthread_t tid)
{
    pthread_mutex_lock(&g_threads.lock);
    g_threads.slog2_thread = tid;
    pthread_mutex_unlock(&g_threads.lock);
}
/**
 * \brief Retrieve the slog2 logging thread ID in a thread-safe manner.
 *
 * \return Thread ID of the slog2 thread, or 0 if not running.
 */
pthread_t dlt_get_slog2_thread(void)
{
    pthread_t tid;
    pthread_mutex_lock(&g_threads.lock);
    tid = g_threads.slog2_thread;
    pthread_mutex_unlock(&g_threads.lock);
    return tid;
}
/**
 * \brief Build a signal set containing termination-related signals
 *        (SIGTERM, SIGINT, SIGQUIT, SIGHUP).
 *
 * \param term_mask  Pointer to the signal set to populate.
 */
static void dlt_build_term_mask(sigset_t *term_mask)
{
    sigemptyset(term_mask);
    sigaddset(term_mask, SIGTERM);
    sigaddset(term_mask, SIGINT);
    sigaddset(term_mask, SIGQUIT);
    sigaddset(term_mask, SIGHUP);
}
/**
 * \brief Wait until slog2 becomes enabled, or a termination signal is pending.
 *
 * Blocks in a loop using a timed condition wait (200 ms intervals) and
 * polls for termination signals (SIGTERM, SIGINT, SIGQUIT, SIGHUP)
 * via non-blocking sigtimedwait.
 *
 * \return 0 if enabled, or a signal number if termination was requested.
 */
static int dlt_slog2_heartbeat_loop(void)
{
    sigset_t term_mask;
    dlt_build_term_mask(&term_mask);
    pthread_mutex_lock(&g_threads.lock);
    while (g_slog2_runtime_disabled) {
        /* disabled */
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        ts.tv_nsec += 200 * 1000 * 1000L; /* 200 ms */
        if (ts.tv_nsec >= 1000000000L) {
            ts.tv_sec += 1;
            ts.tv_nsec -= 1000000000L;
        }
        (void)pthread_cond_timedwait(&g_threads.cond, &g_threads.lock, &ts);
        /* Non-blocking signal poll (zero-timeout safe while holding lock). */
        struct timespec zt = {0, 0};
        int sig = sigtimedwait(&term_mask, NULL, &zt);
        if (sig > 0) {
            pthread_mutex_unlock(&g_threads.lock);
            return sig;
        }
    }
    pthread_mutex_unlock(&g_threads.lock);
    /* Poll for a termination signal that may have arrived concurrently
     * with the enable broadcast, before committing to start the thread. */
    struct timespec zt2 = {0, 0};
    int sig2 = sigtimedwait(&term_mask, NULL, &zt2);
    if (sig2 > 0) {
        return sig2;
    }
    return 0; /* enabled */
}
/**
 * @brief dlt-qnx-system entry point.
 *
 * Processes command-line arguments and starts program execution.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 *
 * @return 0 on successful execution; a non-zero value on error.
 */
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
    if (dlt_threads_init() < 0) {
        DLT_LOG(dltQnxSystem, DLT_LOG_ERROR,
                DLT_STRING("Failed to initialize threads."));
        DLT_UNREGISTER_CONTEXT(dltQnxSystem);
        DLT_UNREGISTER_APP();
        return -1;
    }
    dlt_set_main_thread(pthread_self());
    dlt_slog2_set_disabled(!g_dlt_qnx_conf->qnxslogger2.startupEnabled);
    dlt_register_injection_callback(&dltQnxSystem,
            INJECTION_SLOG2_ADAPTER, dlt_injection_cb);
    DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG,
            DLT_STRING("Setting signals wait for abnormal exit"));

    if (!dlt_slog2_is_disabled()) {
        DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG, DLT_STRING("Launching logging thread (startup enabled)."));
        start_thread();
        if (!dlt_get_slog2_thread()) {
            DLT_LOG(dltQnxSystem, DLT_LOG_ERROR,
                    DLT_STRING("Failed to create slog2 thread, exiting."));
            clean_up();
            DLT_UNREGISTER_CONTEXT(dltQnxSystem);
            DLT_UNREGISTER_APP();
            return -1;
        }
    } else {
        DLT_LOG(dltQnxSystem, DLT_LOG_INFO,
                DLT_STRING("Slogger2 startup disabled; waiting for enable injection."));
        /* Do NOT start thread here. */
    }
    /* Main event loop: normal shutdown or injection-triggered stop/restart */
    while (1) {
        ret = sigwait(&mask, &sigNo);
        if (ret != 0) {
            DLT_LOG(dltQnxSystem, DLT_LOG_ERROR,
                    DLT_STRING("sigwait error:"), DLT_INT(ret));
            break;
        }
        DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG,
                DLT_STRING("Received signal:"),
                DLT_STRING(strsignal(sigNo)));
        if (sigNo == SIGUSR1) {
            if (dlt_slog2_is_disabled()) {
                /* STOP path: join thread (if any) then cleanup mapping */
                pthread_t tid = dlt_get_slog2_thread();
                dlt_set_slog2_thread(0);
                if (tid) {
                    ret = pthread_join(tid, NULL);
                    if (ret != 0) {
                        DLT_LOG(dltQnxSystem, DLT_LOG_ERROR,
                                DLT_STRING("Failed to join slog2 thread, ret="), DLT_INT(ret));
                    }
                }
                clean_qnx_slogger2();
                /* Now wait until enabled again */
                int term_sig = dlt_slog2_heartbeat_loop();
                if (term_sig != 0) {
                    /* termination requested while waiting for enable */
                    sigNo = term_sig;   /* reuse existing sigNo variable */
                    break;              /* exit main loop gracefully */
                }
                /* START path after enable */
                start_thread();
                if (!dlt_get_slog2_thread()) {
                    DLT_LOG(dltQnxSystem, DLT_LOG_ERROR,
                            DLT_STRING("Failed to restart slog2 thread."));
                    /* Unregister injection callback first, then clean_up() (which
                     * calls clean_qnx_slogger2() and may log via dltQnxSystem),
                     * and only then unregister the context and app. This matches
                     * the normal shutdown order and avoids logging via an already
                     * unregistered context. */
                    dlt_register_injection_callback(&dltQnxSystem, INJECTION_SLOG2_ADAPTER, NULL);
                    clean_up();
                    DLT_UNREGISTER_CONTEXT(dltQnxSystem);
                    DLT_UNREGISTER_APP_FLUSH_BUFFERED_LOGS();
                    return -1;
                }
            } else {
                /* SIGUSR1 while adapter is enabled.
                 * In a fast disable/enable race the slog2 thread may have
                 * already self-terminated (callback saw disabled, returned -1)
                 * before main processed the SIGUSR1. Use a non-blocking
                 * join attempt to detect and recover from this case. */
                pthread_t tid = dlt_get_slog2_thread();
                if (tid) {
                    /* Zero-timeout join: returns 0 if thread exited, ETIMEDOUT if alive. */
                    struct timespec zt = {0, 0};
                    if (pthread_timedjoin(tid, NULL, &zt) == 0) {
                        /* Thread already exited — disable/enable race; clean up and restart. */
                        DLT_LOG(dltQnxSystem, DLT_LOG_WARN,
                                DLT_STRING("slog2 thread self-exited during disable/enable race; restarting."));
                        dlt_set_slog2_thread(0);
                        clean_qnx_slogger2();
                        start_thread();
                        if (!dlt_get_slog2_thread()) {
                            DLT_LOG(dltQnxSystem, DLT_LOG_ERROR,
                                    DLT_STRING("Failed to restart slog2 thread after disable/enable race."));
                        }
                    }
                    /* ETIMEDOUT: thread is alive, spurious SIGUSR1 — nothing to do. */
                } else {
                    /* No thread running but adapter is enabled — start one. */
                    start_thread();
                }
            }
            continue;
        }
        /* SIGTERM/SIGHUP/SIGQUIT/SIGINT: shutdown */
        break;
    }
    dlt_slog2_set_disabled(true);
    join_thread();
    dlt_register_injection_callback(&dltQnxSystem, INJECTION_SLOG2_ADAPTER, NULL);
    clean_up();
    DLT_UNREGISTER_CONTEXT(dltQnxSystem);
    DLT_UNREGISTER_APP_FLUSH_BUFFERED_LOGS();
    return ret;
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
                if (options->configurationFileName && strcmp(options->configurationFileName, DEFAULT_CONF_FILE) != 0) {
                    free(options->configurationFileName);
                }
                options->configurationFileName = (char *)malloc(strlen(optarg)+1);
                MALLOC_ASSERT(options->configurationFileName);
                /**
                 * strcpy uncritical here, because size matches exactly the size
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
static void init_configuration()
{
    g_dlt_qnx_conf = calloc(1, sizeof(DltQnxSystemConfiguration));
    /* Common */
    g_dlt_qnx_conf->applicationId          = strdup("QSYM");
    g_dlt_qnx_conf->applicationContextId   = strdup("QSYC");

    /* Slogger2 */
    g_dlt_qnx_conf->qnxslogger2.startupEnabled     = 0;
    g_dlt_qnx_conf->qnxslogger2.contextId  = strdup("QSLA");
    g_dlt_qnx_conf->qnxslogger2.useOriginalTimestamp = 1;
}

/**
 * Read options from the configuration file
 */
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
            /* Common */
            if (strcmp(token, "ApplicationId") == 0)
            {
                if (g_dlt_qnx_conf->applicationId)
                    free(g_dlt_qnx_conf->applicationId);

                g_dlt_qnx_conf->applicationId = (char *)malloc(DLT_ID_SIZE + 1);
                MALLOC_ASSERT(g_dlt_qnx_conf->applicationId);
                strncpy(g_dlt_qnx_conf->applicationId, value, DLT_ID_SIZE);
                g_dlt_qnx_conf->applicationId[DLT_ID_SIZE] = '\0';
            }
            else if (strcmp(token, "ApplicationContextID") == 0)
            {
                if (g_dlt_qnx_conf->applicationContextId)
                    free(g_dlt_qnx_conf->applicationContextId);

                g_dlt_qnx_conf->applicationContextId = (char *)malloc(DLT_ID_SIZE + 1);
                MALLOC_ASSERT(g_dlt_qnx_conf->applicationContextId);
                strncpy(g_dlt_qnx_conf->applicationContextId, value, DLT_ID_SIZE);
                g_dlt_qnx_conf->applicationContextId[DLT_ID_SIZE] = '\0';
            }
            /* Slogger2 */
            else if (strcmp(token, "QnxSlogger2Enable") == 0)
            {
                g_dlt_qnx_conf->qnxslogger2.startupEnabled = atoi(value);
            }
            else if (strcmp(token, "QnxSlogger2ContextId") == 0)
            {
                if (g_dlt_qnx_conf->qnxslogger2.contextId)
                    free(g_dlt_qnx_conf->qnxslogger2.contextId);

                g_dlt_qnx_conf->qnxslogger2.contextId = (char *)malloc(DLT_ID_SIZE + 1);
                MALLOC_ASSERT(g_dlt_qnx_conf->qnxslogger2.contextId);
                strncpy(g_dlt_qnx_conf->qnxslogger2.contextId, value, DLT_ID_SIZE);
                g_dlt_qnx_conf->qnxslogger2.contextId[DLT_ID_SIZE] = '\0';
            }
            else if (strcmp(token, "QnxSlogger2UseOriginalTimestamp") == 0)
            {
                g_dlt_qnx_conf->qnxslogger2.useOriginalTimestamp = atoi(value);
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
    close(fd);
    /*
     * Ignore signals related to child processes and terminal handling.
     * This is intentional: as a daemon, we do not want to handle these signals.
     * If you need to change daemon signal behavior, review this block.
     */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    int ret;
    ret = sigaction(SIGCHLD, &sa, NULL);
    if (ret != 0) {
        err(-1, "%s failed on sigaction() SIGCHLD", __func__);
    }
    ret = sigaction(SIGTSTP, &sa, NULL);
    if (ret != 0) {
        err(-1, "%s failed on sigaction() SIGTSTP", __func__);
    }
    ret = sigaction(SIGTTOU, &sa, NULL);
    if (ret != 0) {
        err(-1, "%s failed on sigaction() SIGTTOU", __func__);
    }
    ret = sigaction(SIGTTIN, &sa, NULL);
    if (ret != 0) {
        err(-1, "%s failed on sigaction() SIGTTIN", __func__);
    }
}
/*
 * Start slogger2 logging thread.
 * Runtime enable/disable state is controlled separately
 * via g_slog2_runtime_disabled.
 */
static void start_thread()
{
    DLT_LOG(dltQnxSystem,
            DLT_LOG_DEBUG,
            DLT_STRING("Starting slogger2 logging thread."));
    start_qnx_slogger2(g_dlt_qnx_conf);
}

/**
 * Wait for the slog2 logging thread to exit, if it was started.
 * Side effects: Joins the thread, logs exit, and unregisters DLT context.
 * Thread safety: Assumes g_threads.slog2_thread is only set by start_qnx_slogger2.
 */
static void join_thread()
{
    int ret;
    pthread_t tid = dlt_get_slog2_thread();
    dlt_set_slog2_thread(0);
    if (tid) {
        ret = pthread_join(tid, NULL);
        if (ret != 0) {
            DLT_LOG(dltQnxSystem,
                    DLT_LOG_ERROR,
                    DLT_STRING("Failed to join slog2 thread."),
                    DLT_INT(ret));
        }
        DLT_LOG(dltQnxSystem,
                DLT_LOG_DEBUG,
                DLT_STRING("dlt-qnx-system, thread exit ..."));
    }
}
static int dlt_injection_cb(uint32_t service_id, void *data, uint32_t length)
{
    int ret = 0;
    if (data == NULL || length < (DATA_LENGTH)) {
        DLT_LOG(dltQnxSystem, DLT_LOG_ERROR,
                DLT_STRING("Invalid injection payload."));
        return -1;
    }
    DLT_LOG(dltQnxSystem, DLT_LOG_INFO,
            DLT_STRING("Injection received:"),
            DLT_INT32(service_id));
    if (service_id != INJECTION_SLOG2_ADAPTER) {
        ret = -1;
        DLT_LOG(dltQnxSystem, DLT_LOG_ERROR,
            DLT_STRING("Unknown injection service id:"),
            DLT_INT32(service_id));
        return ret;
    }
    if (length == (sizeof(DATA_DISABLED)-1) &&
        0 == strncmp((char*) data, DATA_DISABLED, sizeof(DATA_DISABLED)-1)) {
        dlt_slog2_set_disabled(true);
        /* SIGUSR1: wake main thread sigwait to join and cleanup */
        ret = pthread_kill(dlt_get_main_thread(), SIGUSR1);
        if (ret != 0) {
            DLT_LOG(dltQnxSystem, DLT_LOG_ERROR,
                DLT_STRING("Failed to send SIGUSR1 to main thread. DISABLED request not processed."),
                DLT_INT(ret));
            ret = -1;
        } else {
            DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG,
                DLT_STRING("slog2 callback is now DISABLED by user injection request."));
        }
    }
    else if (length == (sizeof(DATA_ENABLED)-1) &&
             0 == strncmp((char*) data, DATA_ENABLED, sizeof(DATA_ENABLED)-1)) {
        /* pthread_cond_broadcast inside dlt_slog2_set_disabled wakes the
         * heartbeat loop directly */
        dlt_slog2_set_disabled(false);
        /* SIGUSR1: wake main thread sigwait to start thread */
        ret = pthread_kill(dlt_get_main_thread(), SIGUSR1);
        if (ret != 0) {
            DLT_LOG(dltQnxSystem, DLT_LOG_ERROR,
                DLT_STRING("Failed to send SIGUSR1 to main thread. ENABLED request not processed."),
                DLT_INT(ret));
            ret = -1;
        } else {
            DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG,
                DLT_STRING("slog2 callback is now ENABLED by user injection request."));
        }
    }
    else {
        DLT_LOG(dltQnxSystem, DLT_LOG_ERROR,
            DLT_STRING("Unknown injection data for slog2 adapter:"),
            DLT_STRING((char*) data));
        ret = -1;
    }
    return ret;
}
static void clean_up()
{
    clean_qnx_slogger2();
    if (dlt_threads_destroy() != 0) {
        DLT_LOG(dltQnxSystem, DLT_LOG_ERROR,
                DLT_STRING("Failed to destroy threads."));
    }
    if (!g_dlt_qnx_conf)
        return;
    if (g_dlt_qnx_conf->applicationId)
        free(g_dlt_qnx_conf->applicationId);
    if (g_dlt_qnx_conf->applicationContextId)
        free(g_dlt_qnx_conf->applicationContextId);
    if (g_dlt_qnx_conf->qnxslogger2.contextId)
        free(g_dlt_qnx_conf->qnxslogger2.contextId);
    if (g_dlt_qnx_conf)
        free(g_dlt_qnx_conf);
    g_dlt_qnx_conf = NULL;
}
