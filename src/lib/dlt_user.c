/*
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
 */

/*!
 * \author
 * Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 * Markus Klein <Markus.Klein@esk.fraunhofer.de>
 * Mikko Rapeli <mikko.rapeli@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_user.c
 */

#include <stdlib.h> /* for getenv(), free(), atexit() */
#include <string.h> /* for strcmp(), strncmp(), strlen(), memset(), memcpy() */
#include <signal.h> /* for signal(), SIGPIPE, SIG_IGN */

#if !defined (__WIN32__)
#   include <syslog.h> /* for LOG_... */
#   include <semaphore.h>
#   include <pthread.h> /* POSIX Threads */
#endif

#include <sys/time.h>
#include <math.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/uio.h> /* writev() */
#include <poll.h>

#include <limits.h>
#ifdef linux
#   include <sys/prctl.h>
#endif

#include <sys/types.h> /* needed for getpid() */
#include <unistd.h>

#include <stdbool.h>
#ifdef DLT_USE_UNIX_SOCKET_IPC
#   include <sys/un.h>
#   include <sys/socket.h>
#endif

#include "dlt_user.h"
#include "dlt_common.h"
#include "dlt_user_shared.h"
#include "dlt_user_shared_cfg.h"
#include "dlt_user_cfg.h"

#ifdef DLT_FATAL_LOG_RESET_ENABLE
#   define DLT_LOG_FATAL_RESET_TRAP(LOGLEVEL) \
    do {                                   \
        if (LOGLEVEL == DLT_LOG_FATAL) {   \
            int *p = NULL;                 \
            *p = 0;                        \
        }                                  \
    } while (0)
#else /* DLT_FATAL_LOG_RESET_ENABLE */
#   define DLT_LOG_FATAL_RESET_TRAP(LOGLEVEL)
#endif /* DLT_FATAL_LOG_RESET_ENABLE */

static DltUser dlt_user;
static bool dlt_user_initialised = false;
static int dlt_user_freeing = 0;

#ifndef DLT_USE_UNIX_SOCKET_IPC
static char dlt_user_dir[DLT_PATH_MAX];
static char dlt_daemon_fifo[DLT_PATH_MAX];
#endif

static sem_t dlt_mutex;
static pthread_t dlt_housekeeperthread_handle;

/* calling dlt_user_atexit_handler() second time fails with error message */
static int atexit_registered = 0;

/* used to disallow DLT usage in fork() child */
static int g_dlt_is_child = 0;
/* String truncate message */
static const char STR_TRUNCATED_MESSAGE[] = "... <<Message truncated, too long>>";

/* Enum for type of string */
enum StringType
{
    ASCII_STRING = 0,
    UTF8_STRING = 1
};

#define DLT_UNUSED(x) (void)(x)

/* Network trace */
#ifdef DLT_NETWORK_TRACE_ENABLE
#define DLT_USER_SEGMENTED_THREAD (1<<2)

/* Segmented Network Trace */
#define DLT_MAX_TRACE_SEGMENT_SIZE 1024
#define DLT_MESSAGE_QUEUE_NAME "/dlt_message_queue"
#define DLT_DELAYED_RESEND_INDICATOR_PATTERN 0xFFFF

/* Mutex to wait on while message queue is not initialized */
pthread_mutex_t mq_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t mq_init_condition;
#endif /* DLT_NETWORK_TRACE_ENABLE */

void dlt_lock_mutex(pthread_mutex_t *mutex)
{
    int32_t lock_mutex_result = pthread_mutex_lock(mutex);

    if (lock_mutex_result != 0)
        dlt_vlog(LOG_ERR,
                 "Mutex lock failed unexpected pid=%i with result %i!\n",
                 getpid(), lock_mutex_result);
}

void dlt_unlock_mutex(pthread_mutex_t *mutex)
{
    pthread_mutex_unlock(mutex);
}

/* Structure to pass data to segmented thread */
typedef struct
{
    DltContext *handle;
    uint32_t id;
    DltNetworkTraceType nw_trace_type;
    uint32_t header_len;
    void *header;
    uint32_t payload_len;
    void *payload;
} s_segmented_data;

/* Function prototypes for internally used functions */
static void dlt_user_housekeeperthread_function(void *ptr);
static void dlt_user_atexit_handler(void);
static DltReturnValue dlt_user_log_init(DltContext *handle, DltContextData *log);
static DltReturnValue dlt_user_log_send_log(DltContextData *log, int mtype);
static DltReturnValue dlt_user_log_send_register_application(void);
static DltReturnValue dlt_user_log_send_unregister_application(void);
static DltReturnValue dlt_user_log_send_register_context(DltContextData *log);
static DltReturnValue dlt_user_log_send_unregister_context(DltContextData *log);
static DltReturnValue dlt_send_app_ll_ts_limit(const char *apid,
                                               DltLogLevelType loglevel,
                                               DltTraceStatusType tracestatus);
static DltReturnValue dlt_user_log_send_log_mode(DltUserLogMode mode);
static DltReturnValue dlt_user_log_send_marker();
static DltReturnValue dlt_user_print_msg(DltMessage *msg, DltContextData *log);
static DltReturnValue dlt_user_log_check_user_message(void);
static void dlt_user_log_reattach_to_daemon(void);
static DltReturnValue dlt_user_log_send_overflow(void);
static DltReturnValue dlt_user_log_out_error_handling(void *ptr1,
                                                      size_t len1,
                                                      void *ptr2,
                                                      size_t len2,
                                                      void *ptr3,
                                                      size_t len3);
static void dlt_user_cleanup_handler(void *arg);
static int dlt_start_threads();
static void dlt_stop_threads();
static void dlt_fork_child_fork_handler();
#ifdef DLT_NETWORK_TRACE_ENABLE
static void dlt_user_trace_network_segmented_thread(void *unused);
static void dlt_user_trace_network_segmented_thread_segmenter(s_segmented_data *data);
#endif

static DltReturnValue dlt_user_log_write_string_utils(DltContextData *log, const char *text,
                                                      const enum StringType type);
static DltReturnValue dlt_user_log_write_sized_string_utils(DltContextData *log, const char *text,
                                                            uint16_t text_len, const enum StringType type);


DltReturnValue dlt_user_check_library_version(const char *user_major_version, const char *user_minor_version)
{
    char lib_major_version[DLT_USER_MAX_LIB_VERSION_LENGTH];
    char lib_minor_version[DLT_USER_MAX_LIB_VERSION_LENGTH];

    dlt_get_major_version(lib_major_version, DLT_USER_MAX_LIB_VERSION_LENGTH);
    dlt_get_minor_version(lib_minor_version, DLT_USER_MAX_LIB_VERSION_LENGTH);

    if ((strcmp(lib_major_version, user_major_version) != 0) || (strcmp(lib_minor_version, user_minor_version) != 0)) {
        dlt_vnlog(LOG_WARNING,
                  DLT_USER_BUFFER_LENGTH,
                  "DLT Library version check failed! Installed DLT library version is %s.%s - Application using DLT library version %s.%s\n",
                  lib_major_version,
                  lib_minor_version,
                  user_major_version,
                  user_minor_version);

        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

#ifdef DLT_USE_UNIX_SOCKET_IPC
static DltReturnValue dlt_initialize_socket_connection(void)
{
    struct sockaddr_un remote;
    int status = 0;
    char dltSockBaseDir[DLT_IPC_PATH_MAX];
    struct linger l_opt;

    DLT_SEM_LOCK();
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sockfd == DLT_FD_INIT) {
        dlt_log(LOG_CRIT, "Failed to create socket\n");
        DLT_SEM_FREE();
        return DLT_RETURN_ERROR;
    }

    status = fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);

    if (status == -1) {
        dlt_vlog(LOG_INFO,
                 "Socket %s/dlt cannot be changed to NON BLOCK\n",
                 DLT_USER_IPC_PATH);
        return DLT_RETURN_ERROR;
    }

    /* Set SO_LINGER opt for the new client socket. */
    l_opt.l_onoff = 1;
    l_opt.l_linger = 10;

    if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &l_opt, sizeof l_opt) < 0)
        dlt_log(LOG_WARNING, "Failed to set linger option\n");

    remote.sun_family = AF_UNIX;
    snprintf(dltSockBaseDir, DLT_IPC_PATH_MAX, "%s/dlt", DLT_USER_IPC_PATH);
    strncpy(remote.sun_path, dltSockBaseDir, sizeof(remote.sun_path));

    if (strlen(DLT_USER_IPC_PATH) > DLT_IPC_PATH_MAX)
        dlt_vlog(LOG_INFO,
                 "Provided path too long...trimming it to path[%s]\n",
                 dltSockBaseDir);

    if (connect(sockfd, (struct sockaddr *)&remote, sizeof(remote)) == -1) {
        if (dlt_user.connection_state != DLT_USER_RETRY_CONNECT) {
            dlt_vlog(LOG_INFO,
                     "Socket %s cannot be opened. Retrying later...\n",
                     dltSockBaseDir);
            dlt_user.connection_state = DLT_USER_RETRY_CONNECT;
        }

        close(sockfd);
        dlt_user.dlt_log_handle = -1;
    }
    else {
        dlt_user.dlt_log_handle = sockfd;
        dlt_user.connection_state = DLT_USER_CONNECTED;

        if (dlt_receiver_init(&(dlt_user.receiver),
                              sockfd,
                              DLT_USER_RCVBUF_MAX_SIZE) == DLT_RETURN_ERROR) {
            dlt_user_initialised = false;
            DLT_SEM_FREE();
            return DLT_RETURN_ERROR;
        }
    }

    DLT_SEM_FREE();

    return DLT_RETURN_OK;
}
#else /* setup fifo*/
static DltReturnValue dlt_initialize_fifo_connection(void)
{
    char filename[DLT_PATH_MAX];
    int ret;

    snprintf(dlt_user_dir, DLT_PATH_MAX, "%s/dltpipes", dltFifoBaseDir);
    snprintf(dlt_daemon_fifo, DLT_PATH_MAX, "%s/dlt", dltFifoBaseDir);
    ret = mkdir(dlt_user_dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH | S_ISVTX);

    if ((ret == -1) && (errno != EEXIST)) {
        dlt_vnlog(LOG_ERR, DLT_USER_BUFFER_LENGTH, "FIFO user dir %s cannot be created!\n", dlt_user_dir);
        return DLT_RETURN_ERROR;
    }

    /* if dlt pipes directory is created by the application also chmod the directory */
    if (ret == 0) {
        /* S_ISGID cannot be set by mkdir, let's reassign right bits */
        ret = chmod(dlt_user_dir,
                    S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH | S_ISGID |
                    S_ISVTX);

        if (ret == -1) {
            dlt_vnlog(LOG_ERR, DLT_USER_BUFFER_LENGTH, "FIFO user dir %s cannot be chmoded!\n", dlt_user_dir);
            return DLT_RETURN_ERROR;
        }
    }

    /* create and open DLT user FIFO */
    snprintf(filename, DLT_PATH_MAX, "%s/dlt%d", dlt_user_dir, getpid());

    /* Try to delete existing pipe, ignore result of unlink */
    unlink(filename);

    ret = mkfifo(filename, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP);

    if (ret == -1)
        dlt_vnlog(LOG_WARNING, DLT_USER_BUFFER_LENGTH, "Loging disabled, FIFO user %s cannot be created!\n", filename);

    /* S_IWGRP cannot be set by mkfifo (???), let's reassign right bits */
    ret = chmod(filename, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP);

    if (ret == -1) {
        dlt_vnlog(LOG_WARNING, DLT_USER_BUFFER_LENGTH, "FIFO user %s cannot be chmoded!\n", dlt_user_dir);
        return DLT_RETURN_ERROR;
    }

    dlt_user.dlt_user_handle = open(filename, O_RDWR | O_NONBLOCK | O_CLOEXEC);

    if (dlt_user.dlt_user_handle == DLT_FD_INIT) {
        dlt_vnlog(LOG_WARNING, DLT_USER_BUFFER_LENGTH, "Logging disabled, FIFO user %s cannot be opened!\n", filename);
        unlink(filename);
        return DLT_RETURN_OK;
    }

    /* open DLT output FIFO */
    dlt_user.dlt_log_handle = open(dlt_daemon_fifo, O_WRONLY | O_NONBLOCK | O_CLOEXEC);

    if (dlt_user.dlt_log_handle == -1)
        /* This is a normal usecase. It is OK that the daemon (and thus the FIFO /tmp/dlt)
         * starts later and some DLT users have already been started before.
         * Thus it is OK if the FIFO can't be opened. */
        dlt_vnlog(LOG_INFO, DLT_USER_BUFFER_LENGTH, "FIFO %s cannot be opened. Retrying later...\n",
                  dlt_daemon_fifo);

    return DLT_RETURN_OK;
}
#endif

DltReturnValue dlt_init(void)
{
    /* check environment variables */
    dlt_check_envvar();

    /* process is exiting. Do not allocate new resources. */
    if (dlt_user_freeing != 0) {
        dlt_vlog(LOG_INFO, "%s logging disabled, process is exiting", __func__);
        /* return negative value, to stop the current log */
        return DLT_RETURN_LOGGING_DISABLED;
    }

    /* WARNING: multithread unsafe ! */
    /* Another thread will check that dlt_user_initialised != 0, but the lib is not initialised ! */
    dlt_user_initialised = true;

    /* Initialize common part of dlt_init()/dlt_init_file() */
    if (dlt_init_common() == DLT_RETURN_ERROR) {
        dlt_user_initialised = false;
        return DLT_RETURN_ERROR;
    }

    dlt_user.dlt_is_file = 0;
    dlt_user.overflow = 0;
    dlt_user.overflow_counter = 0;
#ifdef DLT_SHM_ENABLE
    memset(&(dlt_user.dlt_shm), 0, sizeof(DltShm));

    /* init shared memory */
    if (dlt_shm_init_client(&(dlt_user.dlt_shm), dltShmName) < DLT_RETURN_OK)
        dlt_vnlog(LOG_WARNING, DLT_USER_BUFFER_LENGTH, "Logging disabled,"
                  " Shared memory %s cannot be created!\n", dltShmName);

#endif

#ifdef DLT_USE_UNIX_SOCKET_IPC

    if (dlt_initialize_socket_connection() != DLT_RETURN_OK)
        /* We could connect to the pipe, but not to the socket, which is normally */
        /* open before by the DLT daemon => bad failure => return error code */
        /* in case application is started before daemon, it is expected behaviour */
        return DLT_RETURN_ERROR;

#else /* FIFO connection */

    if (dlt_initialize_fifo_connection() != DLT_RETURN_OK)
        return DLT_RETURN_ERROR;

    if (dlt_receiver_init(&(dlt_user.receiver), dlt_user.dlt_user_handle,
                          DLT_USER_RCVBUF_MAX_SIZE) == DLT_RETURN_ERROR) {
        dlt_user_initialised = false;
        return DLT_RETURN_ERROR;
    }

#endif

#ifdef DLT_NETWORK_TRACE_ENABLE
    /* These will be lazy initialized only when needed */
    dlt_user.dlt_segmented_queue_read_handle = -1;
    dlt_user.dlt_segmented_queue_write_handle = -1;

    pthread_cond_init(&mq_init_condition, NULL);
#endif

    if (dlt_start_threads() < 0) {
        dlt_user_initialised = false;
        return DLT_RETURN_ERROR;
    }

    /* prepare for fork() call */
    pthread_atfork(NULL, NULL, &dlt_fork_child_fork_handler);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_get_appid(char *appid)
{
    if (appid != NULL) {
        strncpy(appid, dlt_user.appID, 4);
        return DLT_RETURN_OK;
    } else {
        dlt_log(LOG_ERR, "Invalid parameter.\n");
        return DLT_RETURN_WRONG_PARAMETER;
    }
}

DltReturnValue dlt_init_file(const char *name)
{
    /* check null pointer */
    if (!name)
        return DLT_RETURN_WRONG_PARAMETER;

    dlt_user_initialised = true;

    /* Initialize common part of dlt_init()/dlt_init_file() */
    if (dlt_init_common() == DLT_RETURN_ERROR) {
        dlt_user_initialised = false;
        return DLT_RETURN_ERROR;
    }

    dlt_user.dlt_is_file = 1;

    /* open DLT output file */
    dlt_user.dlt_log_handle = open(name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); /* mode: wb */

    if (dlt_user.dlt_log_handle == -1) {
        dlt_vnlog(LOG_ERR, DLT_USER_BUFFER_LENGTH, "Log file %s cannot be opened!\n", name);
        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

#ifdef DLT_NETWORK_TRACE_ENABLE
DltReturnValue dlt_init_message_queue(void)
{
    dlt_lock_mutex(&mq_mutex);

    if ((dlt_user.dlt_segmented_queue_read_handle >= 0) &&
        (dlt_user.dlt_segmented_queue_write_handle >= 0)) {
        /* Already intialized */
        dlt_unlock_mutex(&mq_mutex);
        return DLT_RETURN_OK;
    }

    /* Generate per process name for queue */
    char queue_name[NAME_MAX];
    snprintf(queue_name, NAME_MAX, "%s.%d", DLT_MESSAGE_QUEUE_NAME, getpid());

    /* Maximum queue size is 10, limit to size of pointers */
    struct mq_attr mqatr;
    mqatr.mq_flags = 0;
    mqatr.mq_maxmsg = 10;
    mqatr.mq_msgsize = sizeof(s_segmented_data *);
    mqatr.mq_curmsgs = 0;

    /**
     * Create the message queue. It must be newly created
     * if old one was left by a crashing process.
     * */
    dlt_user.dlt_segmented_queue_read_handle = mq_open(queue_name, O_CREAT | O_RDONLY | O_EXCL,
                                                       S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                                       &mqatr);

    if (dlt_user.dlt_segmented_queue_read_handle < 0) {
        if (errno == EEXIST) {
            dlt_log(LOG_WARNING, "Old message queue exists, trying to delete.\n");

            if (mq_unlink(queue_name) < 0)
                dlt_vnlog(LOG_CRIT, 256, "Could not delete existing message queue!: %s \n", strerror(errno));
            else /* Retry */

                dlt_user.dlt_segmented_queue_read_handle = mq_open(queue_name,
                                                                   O_CREAT | O_RDONLY | O_EXCL,
                                                                   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                                                   &mqatr);
        }

        if (dlt_user.dlt_segmented_queue_read_handle < 0) {
            dlt_vnlog(LOG_CRIT, 256, "Can't create message queue read handle!: %s \n", strerror(errno));
            dlt_unlock_mutex(&mq_mutex);
            return DLT_RETURN_ERROR;
        }
    }

    dlt_user.dlt_segmented_queue_write_handle = mq_open(queue_name, O_WRONLY | O_NONBLOCK);

    if (dlt_user.dlt_segmented_queue_write_handle < 0) {

        dlt_vnlog(LOG_CRIT, 256, "Can't open message queue write handle!: %s \n", strerror(errno));
        dlt_unlock_mutex(&mq_mutex);
        return DLT_RETURN_ERROR;
    }

    pthread_cond_signal(&mq_init_condition);
    dlt_unlock_mutex(&mq_mutex);
    return DLT_RETURN_OK;
}
#endif /* DLT_NETWORK_TRACE_ENABLE */

DltReturnValue dlt_init_common(void)
{
    char *env_local_print;
    char *env_initial_log_level;
    char *env_buffer_min;
    uint32_t buffer_min = DLT_USER_RINGBUFFER_MIN_SIZE;
    char *env_buffer_max;
    uint32_t buffer_max = DLT_USER_RINGBUFFER_MAX_SIZE;
    char *env_buffer_step;
    uint32_t buffer_step = DLT_USER_RINGBUFFER_STEP_SIZE;
    char *env_disable_extended_header_for_nonverbose;
    char *env_log_buffer_len;
    uint32_t buffer_max_configured = 0;
    uint32_t header_size = 0;

    /* Binary semaphore for threads */
    if (sem_init(&dlt_mutex, 0, 1) == -1) {
        dlt_user_initialised = false;
        return DLT_RETURN_ERROR;
    }

    /* set to unknown state of connected client */
    dlt_user.log_state = -1;

    dlt_user.dlt_log_handle = -1;
    dlt_user.dlt_user_handle = DLT_FD_INIT;

    dlt_set_id(dlt_user.ecuID, DLT_USER_DEFAULT_ECU_ID);
    dlt_set_id(dlt_user.appID, "");

    dlt_user.application_description = NULL;

    /* Verbose mode is enabled by default */
    dlt_user.verbose_mode = 1;

    /* header_size is used for resend buffer
     * so it won't include DltStorageHeader
     */
    header_size = sizeof(DltUserHeader) + sizeof(DltStandardHeader) +
                sizeof(DltStandardHeaderExtra);

    /* Use extended header for non verbose is enabled by default */
    dlt_user.use_extended_header_for_non_verbose =
            DLT_USER_USE_EXTENDED_HEADER_FOR_NONVERBOSE;

    /* Use extended header for non verbose is modified as per environment variable */
    env_disable_extended_header_for_nonverbose =
            getenv(DLT_USER_ENV_DISABLE_EXTENDED_HEADER_FOR_NONVERBOSE);

    if (env_disable_extended_header_for_nonverbose) {
        if (strcmp(env_disable_extended_header_for_nonverbose, "1") == 0)
            dlt_user.use_extended_header_for_non_verbose =
                    DLT_USER_NO_USE_EXTENDED_HEADER_FOR_NONVERBOSE;
    }

    if (dlt_user.use_extended_header_for_non_verbose ==
            DLT_USER_USE_EXTENDED_HEADER_FOR_NONVERBOSE)
        header_size += sizeof(DltExtendedHeader);

    /* With session id is enabled by default */
    dlt_user.with_session_id = DLT_USER_WITH_SESSION_ID;

    /* With timestamp is enabled by default */
    dlt_user.with_timestamp = DLT_USER_WITH_TIMESTAMP;

    /* With timestamp is enabled by default */
    dlt_user.with_ecu_id = DLT_USER_WITH_ECU_ID;

    /* Local print is disabled by default */
    dlt_user.enable_local_print = 0;

    dlt_user.local_print_mode = DLT_PM_UNSET;

    dlt_user.timeout_at_exit_handler = DLT_USER_ATEXIT_RESEND_BUFFER_EXIT_TIMEOUT;

    env_local_print = getenv(DLT_USER_ENV_LOCAL_PRINT_MODE);

    if (env_local_print) {
        if (strcmp(env_local_print, "AUTOMATIC") == 0)
            dlt_user.local_print_mode = DLT_PM_AUTOMATIC;
        else if (strcmp(env_local_print, "FORCE_ON") == 0)
            dlt_user.local_print_mode = DLT_PM_FORCE_ON;
        else if (strcmp(env_local_print, "FORCE_OFF") == 0)
            dlt_user.local_print_mode = DLT_PM_FORCE_OFF;
    }

    env_initial_log_level = getenv("DLT_INITIAL_LOG_LEVEL");

    if (env_initial_log_level != NULL) {
        if (dlt_env_extract_ll_set(&env_initial_log_level, &dlt_user.initial_ll_set) != 0)
            dlt_vlog(LOG_WARNING,
                     "Unable to parse initial set of log-levels from environment! Env:\n%s\n",
                     getenv("DLT_INITIAL_LOG_LEVEL"));
    }

    /* Initialize LogLevel/TraceStatus field */
    DLT_SEM_LOCK();
    dlt_user.dlt_ll_ts = NULL;
    dlt_user.dlt_ll_ts_max_num_entries = 0;
    dlt_user.dlt_ll_ts_num_entries = 0;


    env_buffer_min = getenv(DLT_USER_ENV_BUFFER_MIN_SIZE);
    env_buffer_max = getenv(DLT_USER_ENV_BUFFER_MAX_SIZE);
    env_buffer_step = getenv(DLT_USER_ENV_BUFFER_STEP_SIZE);

    if (env_buffer_min != NULL) {
        buffer_min = (uint32_t)strtol(env_buffer_min, NULL, 10);

        if ((errno == EINVAL) || (errno == ERANGE)) {
            dlt_vlog(LOG_ERR,
                     "Wrong value specified for %s. Using default\n",
                     DLT_USER_ENV_BUFFER_MIN_SIZE);
            buffer_min = DLT_USER_RINGBUFFER_MIN_SIZE;
        }
    }

    if (env_buffer_max != NULL) {
        buffer_max = (uint32_t)strtol(env_buffer_max, NULL, 10);

        if ((errno == EINVAL) || (errno == ERANGE)) {
            dlt_vlog(LOG_ERR,
                     "Wrong value specified for %s. Using default\n",
                     DLT_USER_ENV_BUFFER_MAX_SIZE);
            buffer_max = DLT_USER_RINGBUFFER_MAX_SIZE;
        }
    }

    if (env_buffer_step != NULL) {
        buffer_step = (uint32_t)strtol(env_buffer_step, NULL, 10);

        if ((errno == EINVAL) || (errno == ERANGE)) {
            dlt_vlog(LOG_ERR,
                     "Wrong value specified for %s. Using default\n",
                     DLT_USER_ENV_BUFFER_STEP_SIZE);
            buffer_step = DLT_USER_RINGBUFFER_STEP_SIZE;
        }
    }

    /* init log buffer size */
    dlt_user.log_buf_len = DLT_USER_BUF_MAX_SIZE;
    env_log_buffer_len = getenv(DLT_USER_ENV_LOG_MSG_BUF_LEN);

    if (env_log_buffer_len != NULL) {
        buffer_max_configured = (uint32_t)strtol(env_log_buffer_len, NULL, 10);

        if (buffer_max_configured > DLT_LOG_MSG_BUF_MAX_SIZE) {
            dlt_user.log_buf_len = DLT_LOG_MSG_BUF_MAX_SIZE;
            dlt_vlog(LOG_WARNING,
                     "Configured size exceeds maximum allowed size,restricting to max [65535 bytes]\n");
        }
        else {
            dlt_user.log_buf_len = buffer_max_configured;
            dlt_vlog(LOG_INFO,
                     "Configured buffer size to [%d bytes]\n",
                     buffer_max_configured);
        }
    }

    if (dlt_user.resend_buffer == NULL) {
        dlt_user.resend_buffer = calloc(sizeof(unsigned char),
                                        (dlt_user.log_buf_len + header_size));

        if (dlt_user.resend_buffer == NULL) {
            dlt_user_initialised = false;
            DLT_SEM_FREE();
            dlt_vlog(LOG_ERR, "cannot allocate memory for resend buffer\n");
            return DLT_RETURN_ERROR;
        }
    }

    if (dlt_buffer_init_dynamic(&(dlt_user.startup_buffer),
                                buffer_min,
                                buffer_max,
                                buffer_step) == DLT_RETURN_ERROR) {
        dlt_user_initialised = false;
        DLT_SEM_FREE();
        return DLT_RETURN_ERROR;
    }

    DLT_SEM_FREE();

    signal(SIGPIPE, SIG_IGN);                  /* ignore pipe signals */

    if (atexit_registered == 0) {
        atexit_registered = 1;
        atexit(dlt_user_atexit_handler);
    }

#ifdef DLT_TEST_ENABLE
    dlt_user.corrupt_user_header = 0;
    dlt_user.corrupt_message_size = 0;
    dlt_user.corrupt_message_size_size = 0;
#endif

    return DLT_RETURN_OK;
}

void dlt_user_atexit_handler(void)
{
    /* parent will do clean-up */
    if (g_dlt_is_child)
        return;

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        /* close file */
        dlt_log_free();
        return;
    }

    /* Try to resend potential log messages in the user buffer */
    int count = dlt_user_atexit_blow_out_user_buffer();

    if (count != 0)
        dlt_vnlog(LOG_WARNING, 128, "Lost log messages in user buffer when exiting: %i\n", count);

    /* Unregister app (this also unregisters all contexts in daemon) */
    /* Ignore return value */
    dlt_unregister_app();

    /* Cleanup */
    /* Ignore return value */
    dlt_free();
}

int dlt_user_atexit_blow_out_user_buffer(void)
{

    int count, ret;
    struct timespec ts;

    uint32_t exitTime = dlt_uptime() + dlt_user.timeout_at_exit_handler;

    /* Send content of ringbuffer */
    DLT_SEM_LOCK();
    count = dlt_buffer_get_message_count(&(dlt_user.startup_buffer));
    DLT_SEM_FREE();

    if (count > 0) {
        while (dlt_uptime() < exitTime) {
            if (dlt_user.dlt_log_handle == -1) {
                /* Reattach to daemon if neccesary */
                dlt_user_log_reattach_to_daemon();

                if ((dlt_user.dlt_log_handle != -1) && (dlt_user.overflow_counter)) {
                    if (dlt_user_log_send_overflow() == 0) {
                        dlt_vnlog(LOG_WARNING,
                                  DLT_USER_BUFFER_LENGTH,
                                  "%u messages discarded!\n",
                                  dlt_user.overflow_counter);
                        dlt_user.overflow_counter = 0;
                    }
                }
            }

            if (dlt_user.dlt_log_handle != -1) {
                ret = dlt_user_log_resend_buffer();

                if (ret == 0) {
                    DLT_SEM_LOCK();
                    count = dlt_buffer_get_message_count(&(dlt_user.startup_buffer));
                    DLT_SEM_FREE();

                    return count;
                }
            }

            ts.tv_sec = 0;
            ts.tv_nsec = DLT_USER_ATEXIT_RESEND_BUFFER_SLEEP;
            nanosleep(&ts, NULL);
        }

        DLT_SEM_LOCK();
        count = dlt_buffer_get_message_count(&(dlt_user.startup_buffer));
        DLT_SEM_FREE();
    }

    return count;
}

static void dlt_user_free_buffer(unsigned char **buffer)
{
    if (*buffer) {
        free(*buffer);
        *buffer = NULL;
    }
}

DltReturnValue dlt_free(void)
{
    uint32_t i;
    int ret = 0;
#ifndef DLT_USE_UNIX_SOCKET_IPC
    char filename[DLT_PATH_MAX];
#endif

    if (dlt_user_freeing != 0)
        /* resources are already being freed. Do nothing and return. */
        return DLT_RETURN_ERROR;

    /* library is freeing its resources. Avoid to allocate it in dlt_init() */
    dlt_user_freeing = 1;

    if (!dlt_user_initialised) {
        dlt_user_freeing = 0;
        return DLT_RETURN_ERROR;
    }

    dlt_user_initialised = false;

    dlt_stop_threads();

#ifndef DLT_USE_UNIX_SOCKET_IPC

    if (dlt_user.dlt_user_handle != DLT_FD_INIT) {
        close(dlt_user.dlt_user_handle);
        dlt_user.dlt_user_handle = DLT_FD_INIT;
        snprintf(filename, DLT_PATH_MAX, "%s/dlt%d", dlt_user_dir, getpid());
        unlink(filename);
    }

#endif

#ifdef DLT_SHM_ENABLE
    /* free shared memory */
    dlt_shm_free_client(&dlt_user.dlt_shm);
#endif

    if (dlt_user.dlt_log_handle != -1) {
        /* close log file/output fifo to daemon */
#ifdef DLT_USE_UNIX_SOCKET_IPC
        ret = shutdown(dlt_user.dlt_log_handle, SHUT_WR);

        if (ret < 0) {
            dlt_vlog(LOG_WARNING, "%s: shutdown failed: %s\n", __func__, strerror(errno));
        }
        else {
            while (1) {
                ssize_t bytes_read;

                bytes_read = read(dlt_user.dlt_log_handle, dlt_user.resend_buffer, dlt_user.log_buf_len);

                if (bytes_read < 0) {
                    dlt_vlog(LOG_DEBUG, "%s - %d: Reading...\n", __func__, __LINE__);
                    break;
                }
                else {
                    dlt_vlog(LOG_DEBUG, "%s - %d: %d bytes read from resend buffer\n", __func__, __LINE__);

                    if (!bytes_read)
                        break;
                }
            }
        }

#endif
        ret = close(dlt_user.dlt_log_handle);

        if (ret < 0)
            dlt_vlog(LOG_WARNING, "%s: close failed: %s\n", __func__, strerror(errno));

        dlt_user.dlt_log_handle = -1;
    }

    /* Ignore return value */
    DLT_SEM_LOCK();
    dlt_receiver_free(&(dlt_user.receiver));
    DLT_SEM_FREE();

    /* Ignore return value */
    DLT_SEM_LOCK();

    dlt_user_free_buffer(&(dlt_user.resend_buffer));
    dlt_buffer_free_dynamic(&(dlt_user.startup_buffer));
    DLT_SEM_FREE();

    DLT_SEM_LOCK();

    if (dlt_user.dlt_ll_ts) {
        for (i = 0; i < dlt_user.dlt_ll_ts_max_num_entries; i++) {
            if (dlt_user.dlt_ll_ts[i].context_description != NULL) {
                free (dlt_user.dlt_ll_ts[i].context_description);
                dlt_user.dlt_ll_ts[i].context_description = NULL;
            }

            if (dlt_user.dlt_ll_ts[i].log_level_ptr != NULL) {
                free(dlt_user.dlt_ll_ts[i].log_level_ptr);
                dlt_user.dlt_ll_ts[i].log_level_ptr = NULL;
            }

            if (dlt_user.dlt_ll_ts[i].trace_status_ptr != NULL) {
                free(dlt_user.dlt_ll_ts[i].trace_status_ptr);
                dlt_user.dlt_ll_ts[i].trace_status_ptr = NULL;
            }

            if (dlt_user.dlt_ll_ts[i].injection_table != NULL) {
                free(dlt_user.dlt_ll_ts[i].injection_table);
                dlt_user.dlt_ll_ts[i].injection_table = NULL;
            }

            dlt_user.dlt_ll_ts[i].nrcallbacks = 0;
            dlt_user.dlt_ll_ts[i].log_level_changed_callback = 0;
        }

        free(dlt_user.dlt_ll_ts);
        dlt_user.dlt_ll_ts = NULL;
        dlt_user.dlt_ll_ts_max_num_entries = 0;
        dlt_user.dlt_ll_ts_num_entries = 0;
    }

    dlt_env_free_ll_set(&dlt_user.initial_ll_set);
    DLT_SEM_FREE();

#ifdef DLT_NETWORK_TRACE_ENABLE
    char queue_name[NAME_MAX];
    snprintf(queue_name, NAME_MAX, "%s.%d", DLT_MESSAGE_QUEUE_NAME, getpid());

    /**
     * Ignore errors from these, to not to spam user if dlt_free
     * is accidentally called multiple times.
     */
    if (dlt_user.dlt_segmented_queue_write_handle > 0)
        mq_close(dlt_user.dlt_segmented_queue_write_handle);

    if (dlt_user.dlt_segmented_queue_read_handle > 0)
        mq_close(dlt_user.dlt_segmented_queue_read_handle);

    if ((dlt_user.dlt_segmented_queue_write_handle > 0) ||
        (dlt_user.dlt_segmented_queue_read_handle > 0))
        mq_unlink(queue_name);

    dlt_user.dlt_segmented_queue_write_handle = DLT_FD_INIT;
    dlt_user.dlt_segmented_queue_read_handle = DLT_FD_INIT;

    pthread_cond_destroy(&mq_init_condition);
#endif /* DLT_NETWORK_TRACE_ENABLE */
    sem_destroy(&dlt_mutex);

    /* allow the user app to do dlt_init() again. */
    /* The flag is unset only to keep almost the same behaviour as before, on EntryNav */
    /* This should be removed for other projects (see documentation of dlt_free() */
    dlt_user_freeing = 0;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_check_library_version(const char *user_major_version, const char *user_minor_version)
{
    return dlt_user_check_library_version(user_major_version, user_minor_version);
}

DltReturnValue dlt_register_app(const char *apid, const char *description)
{
    DltReturnValue ret = DLT_RETURN_OK;

    /* forbid dlt usage in child after fork */
    if (g_dlt_is_child)
        return DLT_RETURN_ERROR;

    if (!dlt_user_initialised) {
        if (dlt_init() < 0) {
            dlt_vlog(LOG_ERR, "%s Failed to initialise dlt", __FUNCTION__);
            return DLT_RETURN_ERROR;
        }
    }

    if ((apid == NULL) || (apid[0] == '\0'))
        return DLT_RETURN_WRONG_PARAMETER;

    /* check if application already registered */
    /* if yes do not register again */
    if (apid[1] == 0) {
        if (apid[0] == dlt_user.appID[0])
            return DLT_RETURN_OK;
    }
    else if (apid[2] == 0)
    {
        if ((apid[0] == dlt_user.appID[0]) &&
            (apid[1] == dlt_user.appID[1]))
            return DLT_RETURN_OK;
    }
    else if (apid[3] == 0)
    {
        if ((apid[0] == dlt_user.appID[0]) &&
            (apid[1] == dlt_user.appID[1]) &&
            (apid[2] == dlt_user.appID[2]))
            return DLT_RETURN_OK;
    }
    else if ((apid[0] == dlt_user.appID[0]) &&
             (apid[1] == dlt_user.appID[1]) &&
             (apid[2] == dlt_user.appID[2]) &&
             (apid[3] == dlt_user.appID[3]))
    {
        return DLT_RETURN_OK;
    }

    DLT_SEM_LOCK();

    /* Store locally application id and application description */
    dlt_set_id(dlt_user.appID, apid);

    if (dlt_user.application_description != NULL)
        free(dlt_user.application_description);

    dlt_user.application_description = NULL;

    if (description != NULL) {
        size_t desc_len = strlen(description);
        dlt_user.application_description = malloc(desc_len + 1);

        if (dlt_user.application_description) {
            strncpy(dlt_user.application_description, description, desc_len + 1);
        } else {
            DLT_SEM_FREE();
            return DLT_RETURN_ERROR;
        }
    }

    DLT_SEM_FREE();

    ret = dlt_user_log_send_register_application();

    if ((ret == DLT_RETURN_OK) && (dlt_user.dlt_log_handle != -1))
        ret = dlt_user_log_resend_buffer();

    return ret;
}

DltReturnValue dlt_register_context(DltContext *handle, const char *contextid, const char *description)
{
    /* check nullpointer */
    if (handle == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    /* forbid dlt usage in child after fork */
    if (g_dlt_is_child)
        return DLT_RETURN_ERROR;

    if (!dlt_user_initialised) {
        if (dlt_init() < 0) {
            dlt_vlog(LOG_ERR, "%s Failed to initialise dlt", __FUNCTION__);
            return DLT_RETURN_ERROR;
        }
    }

    if ((contextid == NULL) || (contextid[0] == '\0'))
        return DLT_RETURN_WRONG_PARAMETER;

    return dlt_register_context_ll_ts(handle,
                                      contextid,
                                      description,
                                      DLT_USER_LOG_LEVEL_NOT_SET,
                                      DLT_USER_TRACE_STATUS_NOT_SET);
}

DltReturnValue dlt_register_context_ll_ts_llccb(DltContext *handle,
                                                const char *contextid,
                                                const char *description,
                                                int loglevel,
                                                int tracestatus,
                                                void (*dlt_log_level_changed_callback)(char context_id[DLT_ID_SIZE],
                                                                                       uint8_t log_level,
                                                                                       uint8_t trace_status))
{
    DltContextData log;
    uint32_t i;
    int envLogLevel = DLT_USER_LOG_LEVEL_NOT_SET;

    /*check nullpointer */
    if ((handle == NULL) || (contextid == NULL) || (contextid[0] == '\0'))
        return DLT_RETURN_WRONG_PARAMETER;

    /* forbid dlt usage in child after fork */
    if (g_dlt_is_child)
        return DLT_RETURN_ERROR;

    if ((loglevel < DLT_USER_LOG_LEVEL_NOT_SET) || (loglevel >= DLT_LOG_MAX)) {
        dlt_vlog(LOG_ERR, "Loglevel %d is outside valid range", loglevel);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if ((tracestatus < DLT_USER_TRACE_STATUS_NOT_SET) || (tracestatus >= DLT_TRACE_STATUS_MAX)) {
        dlt_vlog(LOG_ERR, "Tracestatus %d is outside valid range", tracestatus);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (dlt_user_log_init(handle, &log) < DLT_RETURN_OK)
        return DLT_RETURN_ERROR;

    /* Reset message counter */
    handle->mcnt = 0;

    /* Store context id in log level/trace status field */

    /* Check if already registered, else register context */
    DLT_SEM_LOCK();

    /* Check of double context registration removed */
    /* Double registration is already checked by daemon */

    /* Allocate or expand context array */
    if (dlt_user.dlt_ll_ts == NULL) {
        dlt_user.dlt_ll_ts = (dlt_ll_ts_type *)malloc(sizeof(dlt_ll_ts_type) * DLT_USER_CONTEXT_ALLOC_SIZE);

        if (dlt_user.dlt_ll_ts == NULL) {
            DLT_SEM_FREE();
            return DLT_RETURN_ERROR;
        }

        dlt_user.dlt_ll_ts_max_num_entries = DLT_USER_CONTEXT_ALLOC_SIZE;

        /* Initialize new entries */
        for (i = 0; i < dlt_user.dlt_ll_ts_max_num_entries; i++) {
            dlt_set_id(dlt_user.dlt_ll_ts[i].contextID, "");

            /* At startup, logging and tracing is locally enabled */
            /* the correct log level/status is set after received from daemon */
            dlt_user.dlt_ll_ts[i].log_level = DLT_USER_INITIAL_LOG_LEVEL;
            dlt_user.dlt_ll_ts[i].trace_status = DLT_USER_INITIAL_TRACE_STATUS;

            dlt_user.dlt_ll_ts[i].log_level_ptr = 0;
            dlt_user.dlt_ll_ts[i].trace_status_ptr = 0;

            dlt_user.dlt_ll_ts[i].context_description = 0;

            dlt_user.dlt_ll_ts[i].injection_table = 0;
            dlt_user.dlt_ll_ts[i].nrcallbacks = 0;
            dlt_user.dlt_ll_ts[i].log_level_changed_callback = 0;
        }
    }
    else if ((dlt_user.dlt_ll_ts_num_entries % DLT_USER_CONTEXT_ALLOC_SIZE) == 0)
    {
        /* allocate memory in steps of DLT_USER_CONTEXT_ALLOC_SIZE, e.g. 500 */
        dlt_ll_ts_type *old_ll_ts;
        uint32_t old_max_entries;

        old_ll_ts = dlt_user.dlt_ll_ts;
        old_max_entries = dlt_user.dlt_ll_ts_max_num_entries;

        dlt_user.dlt_ll_ts_max_num_entries = ((dlt_user.dlt_ll_ts_num_entries
                                               / DLT_USER_CONTEXT_ALLOC_SIZE) + 1)
            * DLT_USER_CONTEXT_ALLOC_SIZE;
        dlt_user.dlt_ll_ts = (dlt_ll_ts_type *)malloc(sizeof(dlt_ll_ts_type) *
                                                      dlt_user.dlt_ll_ts_max_num_entries);

        if (dlt_user.dlt_ll_ts == NULL) {
            dlt_user.dlt_ll_ts = old_ll_ts;
            dlt_user.dlt_ll_ts_max_num_entries = old_max_entries;
            DLT_SEM_FREE();
            return DLT_RETURN_ERROR;
        }

        memcpy(dlt_user.dlt_ll_ts, old_ll_ts, sizeof(dlt_ll_ts_type) * dlt_user.dlt_ll_ts_num_entries);
        free(old_ll_ts);

        /* Initialize new entries */
        for (i = dlt_user.dlt_ll_ts_num_entries; i < dlt_user.dlt_ll_ts_max_num_entries; i++) {
            dlt_set_id(dlt_user.dlt_ll_ts[i].contextID, "");

            /* At startup, logging and tracing is locally enabled */
            /* the correct log level/status is set after received from daemon */
            dlt_user.dlt_ll_ts[i].log_level = DLT_USER_INITIAL_LOG_LEVEL;
            dlt_user.dlt_ll_ts[i].trace_status = DLT_USER_INITIAL_TRACE_STATUS;

            dlt_user.dlt_ll_ts[i].log_level_ptr = 0;
            dlt_user.dlt_ll_ts[i].trace_status_ptr = 0;

            dlt_user.dlt_ll_ts[i].context_description = 0;

            dlt_user.dlt_ll_ts[i].injection_table = 0;
            dlt_user.dlt_ll_ts[i].nrcallbacks = 0;
            dlt_user.dlt_ll_ts[i].log_level_changed_callback = 0;
        }
    }

    /* New context entry to be initialized */
    dlt_ll_ts_type *ctx_entry;
    ctx_entry = &dlt_user.dlt_ll_ts[dlt_user.dlt_ll_ts_num_entries];

    /* Store locally context id and context description */
    dlt_set_id(ctx_entry->contextID, contextid);

    if (ctx_entry->context_description != 0)
        free(ctx_entry->context_description);

    ctx_entry->context_description = 0;

    if (description != 0) {
        size_t desc_len = strlen(description);
        ctx_entry->context_description = malloc(desc_len + 1);

        if (ctx_entry->context_description == 0) {
            DLT_SEM_FREE();
            return DLT_RETURN_ERROR;
        }

        strncpy(ctx_entry->context_description, description, desc_len + 1);
    }

    if (ctx_entry->log_level_ptr == 0) {
        ctx_entry->log_level_ptr = malloc(sizeof(int8_t));

        if (ctx_entry->log_level_ptr == 0) {
            DLT_SEM_FREE();
            return DLT_RETURN_ERROR;
        }
    }

    if (ctx_entry->trace_status_ptr == 0) {
        ctx_entry->trace_status_ptr = malloc(sizeof(int8_t));

        if (ctx_entry->trace_status_ptr == 0) {
            DLT_SEM_FREE();
            return DLT_RETURN_ERROR;
        }
    }

    /* check if the log level is set in the environement */
    envLogLevel = dlt_env_adjust_ll_from_env(&dlt_user.initial_ll_set,
                                             dlt_user.appID,
                                             contextid,
                                             DLT_USER_LOG_LEVEL_NOT_SET);

    if (envLogLevel != DLT_USER_LOG_LEVEL_NOT_SET) {
        ctx_entry->log_level = envLogLevel;
        loglevel = envLogLevel;
    }
    else if (loglevel != DLT_USER_LOG_LEVEL_NOT_SET)
    {
        ctx_entry->log_level = loglevel;
    }

    if (tracestatus != DLT_USER_TRACE_STATUS_NOT_SET)
        ctx_entry->trace_status = tracestatus;

    /* Prepare transfer struct */
    dlt_set_id(handle->contextID, contextid);
    handle->log_level_pos = dlt_user.dlt_ll_ts_num_entries;

    handle->log_level_ptr = ctx_entry->log_level_ptr;
    handle->trace_status_ptr = ctx_entry->trace_status_ptr;

    log.context_description = ctx_entry->context_description;

    *(ctx_entry->log_level_ptr) = ctx_entry->log_level;
    *(ctx_entry->trace_status_ptr) = ctx_entry->trace_status = tracestatus;
    ctx_entry->log_level_changed_callback = dlt_log_level_changed_callback;

    log.log_level = loglevel;
    log.trace_status = tracestatus;

    dlt_user.dlt_ll_ts_num_entries++;

    DLT_SEM_FREE();

    return dlt_user_log_send_register_context(&log);
}

DltReturnValue dlt_register_context_ll_ts(DltContext *handle,
                                          const char *contextid,
                                          const char *description,
                                          int loglevel,
                                          int tracestatus)
{
    return dlt_register_context_ll_ts_llccb(handle,
                                            contextid,
                                            description,
                                            loglevel,
                                            tracestatus,
                                            NULL);

}

DltReturnValue dlt_register_context_llccb(DltContext *handle,
                                          const char *contextid,
                                          const char *description,
                                          void (*dlt_log_level_changed_callback)(char context_id[DLT_ID_SIZE],
                                                                                 uint8_t log_level,
                                                                                 uint8_t trace_status))
{
    if ((handle == NULL) || (contextid == NULL) || (contextid[0] == '\0'))
        return DLT_RETURN_WRONG_PARAMETER;

    /* forbid dlt usage in child after fork */
    if (g_dlt_is_child)
        return DLT_RETURN_ERROR;

    if (!dlt_user_initialised) {
        if (dlt_init() < 0) {
            dlt_vlog(LOG_ERR, "%s Failed to initialise dlt", __FUNCTION__);
            return DLT_RETURN_ERROR;
        }
    }

    return dlt_register_context_ll_ts_llccb(handle,
                                            contextid,
                                            description,
                                            DLT_USER_LOG_LEVEL_NOT_SET,
                                            DLT_USER_TRACE_STATUS_NOT_SET,
                                            dlt_log_level_changed_callback);
}

DltReturnValue dlt_unregister_app(void)
{
    DltReturnValue ret = DLT_RETURN_OK;

    /* forbid dlt usage in child after fork */
    if (g_dlt_is_child)
        return DLT_RETURN_ERROR;

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    /* Inform daemon to unregister application and all of its contexts */
    ret = dlt_user_log_send_unregister_application();

    DLT_SEM_LOCK();

    /* Clear and free local stored application information */
    dlt_set_id(dlt_user.appID, "");

    if (dlt_user.application_description != NULL)
        free(dlt_user.application_description);

    dlt_user.application_description = NULL;

    DLT_SEM_FREE();

    return ret;
}

DltReturnValue dlt_unregister_app_flush_buffered_logs(void)
{
    DltReturnValue ret = DLT_RETURN_OK;

    /* forbid dlt usage in child after fork */
    if (g_dlt_is_child)
        return DLT_RETURN_ERROR;

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_ERR, "%s dlt_user_initialised false\n", __func__);
        return DLT_RETURN_ERROR;
    }

    if (dlt_user.dlt_log_handle != -1) {
        do
            ret = dlt_user_log_resend_buffer();
        while ((ret != DLT_RETURN_OK) && (dlt_user.dlt_log_handle != -1));
    }

    return dlt_unregister_app();
}

DltReturnValue dlt_unregister_context(DltContext *handle)
{
    DltContextData log;
    DltReturnValue ret = DLT_RETURN_OK;

    /* forbid dlt usage in child after fork */
    if (g_dlt_is_child)
        return DLT_RETURN_ERROR;

    log.handle = NULL;
    log.context_description = NULL;

    if (dlt_user_log_init(handle, &log) <= DLT_RETURN_ERROR)
        return DLT_RETURN_ERROR;

    DLT_SEM_LOCK();

    handle->log_level_ptr = NULL;
    handle->trace_status_ptr = NULL;

    if (dlt_user.dlt_ll_ts != NULL) {
        /* Clear and free local stored context information */
        dlt_set_id(dlt_user.dlt_ll_ts[handle->log_level_pos].contextID, "");

        dlt_user.dlt_ll_ts[handle->log_level_pos].log_level = DLT_USER_INITIAL_LOG_LEVEL;
        dlt_user.dlt_ll_ts[handle->log_level_pos].trace_status = DLT_USER_INITIAL_TRACE_STATUS;

        if (dlt_user.dlt_ll_ts[handle->log_level_pos].context_description != NULL)
            free(dlt_user.dlt_ll_ts[handle->log_level_pos].context_description);

        if (dlt_user.dlt_ll_ts[handle->log_level_pos].log_level_ptr != NULL) {
            free(dlt_user.dlt_ll_ts[handle->log_level_pos].log_level_ptr);
            dlt_user.dlt_ll_ts[handle->log_level_pos].log_level_ptr = NULL;
        }

        if (dlt_user.dlt_ll_ts[handle->log_level_pos].trace_status_ptr != NULL) {
            free(dlt_user.dlt_ll_ts[handle->log_level_pos].trace_status_ptr);
            dlt_user.dlt_ll_ts[handle->log_level_pos].trace_status_ptr = NULL;
        }

        dlt_user.dlt_ll_ts[handle->log_level_pos].context_description = NULL;

        if (dlt_user.dlt_ll_ts[handle->log_level_pos].injection_table != NULL) {
            free(dlt_user.dlt_ll_ts[handle->log_level_pos].injection_table);
            dlt_user.dlt_ll_ts[handle->log_level_pos].injection_table = NULL;
        }

        dlt_user.dlt_ll_ts[handle->log_level_pos].nrcallbacks = 0;
        dlt_user.dlt_ll_ts[handle->log_level_pos].log_level_changed_callback = 0;
    }

    DLT_SEM_FREE();

    /* Inform daemon to unregister context */
    ret = dlt_user_log_send_unregister_context(&log);

    return ret;
}

DltReturnValue dlt_set_application_ll_ts_limit(DltLogLevelType loglevel, DltTraceStatusType tracestatus)
{
    uint32_t i;

    /* forbid dlt usage in child after fork */
    if (g_dlt_is_child)
        return DLT_RETURN_ERROR;

    if ((loglevel < DLT_USER_LOG_LEVEL_NOT_SET) || (loglevel >= DLT_LOG_MAX)) {
        dlt_vlog(LOG_ERR, "Loglevel %d is outside valid range", loglevel);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if ((tracestatus < DLT_USER_TRACE_STATUS_NOT_SET) || (tracestatus >= DLT_TRACE_STATUS_MAX)) {
        dlt_vlog(LOG_ERR, "Tracestatus %d is outside valid range", tracestatus);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (!dlt_user_initialised) {
        if (dlt_init() < 0) {
            dlt_vlog(LOG_ERR, "%s Failed to initialise dlt", __FUNCTION__);
            return DLT_RETURN_ERROR;
        }
    }

    DLT_SEM_LOCK();

    if (dlt_user.dlt_ll_ts == NULL) {
        DLT_SEM_FREE();
        return DLT_RETURN_ERROR;
    }

    /* Update local structures */
    for (i = 0; i < dlt_user.dlt_ll_ts_num_entries; i++) {
        dlt_user.dlt_ll_ts[i].log_level = loglevel;
        dlt_user.dlt_ll_ts[i].trace_status = tracestatus;

        if (dlt_user.dlt_ll_ts[i].log_level_ptr)
            *(dlt_user.dlt_ll_ts[i].log_level_ptr) = loglevel;

        if (dlt_user.dlt_ll_ts[i].trace_status_ptr)
            *(dlt_user.dlt_ll_ts[i].trace_status_ptr) = tracestatus;
    }

    DLT_SEM_FREE();

    /* Inform DLT server about update */
    return dlt_send_app_ll_ts_limit(dlt_user.appID, loglevel, tracestatus);
}

int dlt_get_log_state()
{
    return dlt_user.log_state;
}

DltReturnValue dlt_set_log_mode(DltUserLogMode mode)
{
    DLT_UNUSED(mode);

    /* forbid dlt usage in child after fork */
    if (g_dlt_is_child)
        return DLT_RETURN_ERROR;

    if ((mode < DLT_USER_MODE_UNDEFINED) || (mode >= DLT_USER_MODE_MAX)) {
        dlt_vlog(LOG_ERR, "User log mode %d is outside valid range", mode);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (!dlt_user_initialised) {
        if (dlt_init() < 0) {
            dlt_vlog(LOG_ERR, "%s Failed to initialise dlt", __FUNCTION__);
            return DLT_RETURN_ERROR;
        }
    }

    return dlt_user_log_send_log_mode(mode);
}

int dlt_set_resend_timeout_atexit(uint32_t timeout_in_milliseconds)
{
    /* forbid dlt usage in child after fork */
    if (g_dlt_is_child)
        return DLT_RETURN_ERROR;

    if (dlt_user_initialised == 0)
        if (dlt_init() < 0)
            return -1;

    dlt_user.timeout_at_exit_handler = timeout_in_milliseconds * 10;
    return 0;
}

/* ********************************************************************************************* */

inline DltReturnValue dlt_user_log_write_start(DltContext *handle, DltContextData *log, DltLogLevelType loglevel)
{
    return dlt_user_log_write_start_id(handle, log, loglevel, DLT_USER_DEFAULT_MSGID);
}

DltReturnValue dlt_user_log_write_start_id(DltContext *handle,
                                           DltContextData *log,
                                           DltLogLevelType loglevel,
                                           uint32_t messageid)
{
    DLT_LOG_FATAL_RESET_TRAP(loglevel);
    DltReturnValue ret = DLT_RETURN_OK;

    /* check nullpointer */
    if ((handle == NULL) || (log == NULL))
        return DLT_RETURN_WRONG_PARAMETER;

    /* forbid dlt usage in child after fork */
    if (g_dlt_is_child)
        return DLT_RETURN_ERROR;

    /* check log levels */
    ret = dlt_user_is_logLevel_enabled(handle, loglevel);

    if (ret == DLT_RETURN_WRONG_PARAMETER)
        return DLT_RETURN_WRONG_PARAMETER;
    else if (ret == DLT_RETURN_LOGGING_DISABLED)
        return DLT_RETURN_OK;

    /* initialize values */
    if ((dlt_user_log_init(handle, log) < DLT_RETURN_OK) || (dlt_user.dlt_ll_ts == NULL))
        return DLT_RETURN_ERROR;

    /* initialize values */
    if (log->buffer == NULL) {
        log->buffer = calloc(sizeof(unsigned char), dlt_user.log_buf_len);

        if (log->buffer == NULL) {
            dlt_vlog(LOG_ERR, "Cannot allocate buffer for DLT Log message\n");
            return DLT_RETURN_ERROR;
        }
    }

    log->args_num = 0;
    log->log_level = loglevel;
    log->size = 0;
    log->use_timestamp = DLT_AUTO_TIMESTAMP;

    /* In non-verbose mode, insert message id */
    if (dlt_user.verbose_mode == 0) {
        if ((sizeof(uint32_t)) > dlt_user.log_buf_len)
            return DLT_RETURN_USER_BUFFER_FULL;

        /* Write message id */
        memcpy(log->buffer, &(messageid), sizeof(uint32_t));
        log->size = sizeof(uint32_t);

        /* as the message id is part of each message in non-verbose mode,
         * it doesn't increment the argument counter in extended header (if used) */
    }

    return DLT_RETURN_TRUE;
}

DltReturnValue dlt_user_log_write_finish(DltContextData *log)
{
    int ret = DLT_RETURN_ERROR;

    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    ret = dlt_user_log_send_log(log, DLT_TYPE_LOG);

    dlt_user_free_buffer(&(log->buffer));

    return ret;
}

DltReturnValue dlt_user_log_write_raw(DltContextData *log, void *data, uint16_t length)
{
    return dlt_user_log_write_raw_formatted(log, data, length, DLT_FORMAT_DEFAULT);
}

DltReturnValue dlt_user_log_write_raw_formatted(DltContextData *log, void *data, uint16_t length, DltFormatType type)
{
    size_t new_log_size = 0;
    uint32_t type_info = 0;

    /* check nullpointer */
    if ((log == NULL) || ((data == NULL) && (length != 0)))
        return DLT_RETURN_WRONG_PARAMETER;

    /* Have to cast type to signed type because some compilers assume that DltFormatType is unsigned and issue a warning */
    if (((int16_t)type < DLT_FORMAT_DEFAULT) || (type >= DLT_FORMAT_MAX)) {
        dlt_vlog(LOG_ERR, "Format type %d is outside valid range", type);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    new_log_size = log->size + length + sizeof(uint16_t);

    if (new_log_size > dlt_user.log_buf_len)
        return DLT_RETURN_USER_BUFFER_FULL;

    if (dlt_user.verbose_mode) {
        new_log_size = log->size + length + sizeof(uint32_t) + sizeof(uint16_t);

        if (new_log_size > dlt_user.log_buf_len)
            return DLT_RETURN_USER_BUFFER_FULL;

        /* Transmit type information */
        type_info = DLT_TYPE_INFO_RAWD;

        if ((type >= DLT_FORMAT_HEX8) && (type <= DLT_FORMAT_HEX64)) {
            type_info |= DLT_SCOD_HEX;
            type_info += type;
        }
        else if ((type >= DLT_FORMAT_BIN8) && (type <= DLT_FORMAT_BIN16))
        {
            type_info |= DLT_SCOD_BIN;
            type_info += type - DLT_FORMAT_BIN8 + 1;
        }

        memcpy((log->buffer) + log->size, &(type_info), sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    /* First transmit length of raw data, then the raw data itself */
    memcpy((log->buffer) + log->size, &(length), sizeof(uint16_t));
    log->size += sizeof(uint16_t);

    memcpy((log->buffer) + log->size, data, length);
    log->size += length;

    log->args_num++;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_float32(DltContextData *log, float32_t data)
{
    uint32_t type_info;

    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    if (sizeof(float32_t) != 4)
        return DLT_RETURN_ERROR;

    if ((log->size + sizeof(float32_t)) > dlt_user.log_buf_len)
        return DLT_RETURN_USER_BUFFER_FULL;

    if (dlt_user.verbose_mode) {
        if ((log->size + sizeof(uint32_t) + sizeof(float32_t)) > dlt_user.log_buf_len)
            return DLT_RETURN_USER_BUFFER_FULL;

        type_info = DLT_TYPE_INFO_FLOA | DLT_TYLE_32BIT;

        memcpy((log->buffer) + log->size, &(type_info), sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer) + log->size, &data, sizeof(float32_t));
    log->size += sizeof(float32_t);

    log->args_num++;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_float64(DltContextData *log, float64_t data)
{
    uint32_t type_info;

    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    if (sizeof(float64_t) != 8)
        return DLT_RETURN_ERROR;

    if ((log->size + sizeof(float64_t)) > dlt_user.log_buf_len)
        return DLT_RETURN_USER_BUFFER_FULL;

    if (dlt_user.verbose_mode) {
        if ((log->size + sizeof(uint32_t) + sizeof(float64_t)) > dlt_user.log_buf_len)
            return DLT_RETURN_USER_BUFFER_FULL;

        type_info = DLT_TYPE_INFO_FLOA | DLT_TYLE_64BIT;

        memcpy((log->buffer) + log->size, &(type_info), sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer) + log->size, &data, sizeof(float64_t));
    log->size += sizeof(float64_t);

    log->args_num++;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_uint(DltContextData *log, unsigned int data)
{
    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    switch (sizeof(unsigned int)) {
    case 1:
    {
        return dlt_user_log_write_uint8(log, (uint8_t)data);
        break;
    }
    case 2:
    {
        return dlt_user_log_write_uint16(log, (uint16_t)data);
        break;
    }
    case 4:
    {
        return dlt_user_log_write_uint32(log, (uint32_t)data);
        break;
    }
    case 8:
    {
        return dlt_user_log_write_uint64(log, (uint64_t)data);
        break;
    }
    default:
    {
        return DLT_RETURN_ERROR;
        break;
    }
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_uint8(DltContextData *log, uint8_t data)
{
    uint32_t type_info;

    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    if ((log->size + sizeof(uint8_t)) > dlt_user.log_buf_len)
        return DLT_RETURN_USER_BUFFER_FULL;

    if (dlt_user.verbose_mode) {
        if ((log->size + sizeof(uint32_t) + sizeof(uint8_t)) > dlt_user.log_buf_len)
            return DLT_RETURN_USER_BUFFER_FULL;

        type_info = DLT_TYPE_INFO_UINT | DLT_TYLE_8BIT;

        memcpy((log->buffer) + log->size, &(type_info), sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer) + log->size, &data, sizeof(uint8_t));
    log->size += sizeof(uint8_t);

    log->args_num++;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_uint16(DltContextData *log, uint16_t data)
{
    uint32_t type_info;

    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    if ((log->size + sizeof(uint16_t)) > dlt_user.log_buf_len)
        return DLT_RETURN_USER_BUFFER_FULL;

    if (dlt_user.verbose_mode) {
        if ((log->size + sizeof(uint32_t) + sizeof(uint16_t)) > dlt_user.log_buf_len)
            return DLT_RETURN_USER_BUFFER_FULL;

        type_info = DLT_TYPE_INFO_UINT | DLT_TYLE_16BIT;

        memcpy((log->buffer) + log->size, &(type_info), sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer) + log->size, &data, sizeof(uint16_t));
    log->size += sizeof(uint16_t);

    log->args_num++;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_uint32(DltContextData *log, uint32_t data)
{
    uint32_t type_info;

    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    if ((log->size + sizeof(uint32_t)) > dlt_user.log_buf_len)
        return DLT_RETURN_USER_BUFFER_FULL;

    if (dlt_user.verbose_mode) {
        if ((log->size + sizeof(uint32_t) + sizeof(uint32_t)) > dlt_user.log_buf_len)
            return DLT_RETURN_USER_BUFFER_FULL;

        type_info = DLT_TYPE_INFO_UINT | DLT_TYLE_32BIT;

        memcpy((log->buffer) + log->size, &(type_info), sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer) + log->size, &data, sizeof(uint32_t));
    log->size += sizeof(uint32_t);

    log->args_num++;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_uint64(DltContextData *log, uint64_t data)
{
    uint32_t type_info;

    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    if ((log->size + sizeof(uint64_t)) > dlt_user.log_buf_len)
        return DLT_RETURN_USER_BUFFER_FULL;

    if (dlt_user.verbose_mode) {
        if ((log->size + sizeof(uint32_t) + sizeof(uint64_t)) > dlt_user.log_buf_len)
            return DLT_RETURN_USER_BUFFER_FULL;

        type_info = DLT_TYPE_INFO_UINT | DLT_TYLE_64BIT;

        memcpy((log->buffer) + log->size, &(type_info), sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer) + log->size, &data, sizeof(uint64_t));
    log->size += sizeof(uint64_t);

    log->args_num++;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_uint8_formatted(DltContextData *log, uint8_t data, DltFormatType type)
{
    uint32_t type_info;

    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    /* Have to cast type to signed type because some compilers assume that DltFormatType is unsigned and issue a warning */
    if (((int16_t)type < DLT_FORMAT_DEFAULT) || (type >= DLT_FORMAT_MAX)) {
        dlt_vlog(LOG_ERR, "Format type %d is outside valid range", type);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    if ((log->size + sizeof(uint16_t)) > dlt_user.log_buf_len)
        return DLT_RETURN_USER_BUFFER_FULL;

    if (dlt_user.verbose_mode) {
        if ((log->size + sizeof(uint32_t) + sizeof(uint16_t)) > dlt_user.log_buf_len)
            return DLT_RETURN_USER_BUFFER_FULL;

        type_info = DLT_TYPE_INFO_UINT | DLT_TYLE_8BIT;

        if ((type >= DLT_FORMAT_HEX8) && (type <= DLT_FORMAT_HEX64))
            type_info |= DLT_SCOD_HEX;

        else if ((type >= DLT_FORMAT_BIN8) && (type <= DLT_FORMAT_BIN16))
            type_info |= DLT_SCOD_BIN;

        memcpy((log->buffer) + log->size, &(type_info), sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer) + log->size, &data, sizeof(uint8_t));
    log->size += sizeof(uint8_t);

    log->args_num++;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_uint16_formatted(DltContextData *log, uint16_t data, DltFormatType type)
{
    uint32_t type_info;

    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    /* Have to cast type to signed type because some compilers assume that DltFormatType is unsigned and issue a warning */
    if (((int16_t)type < DLT_FORMAT_DEFAULT) || (type >= DLT_FORMAT_MAX)) {
        dlt_vlog(LOG_ERR, "Format type %d is outside valid range", type);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    if ((log->size + sizeof(uint16_t)) > dlt_user.log_buf_len)
        return DLT_RETURN_USER_BUFFER_FULL;

    if (dlt_user.verbose_mode) {
        if ((log->size + sizeof(uint32_t) + sizeof(uint16_t)) > dlt_user.log_buf_len)
            return DLT_RETURN_USER_BUFFER_FULL;

        type_info = DLT_TYPE_INFO_UINT | DLT_TYLE_16BIT;

        if ((type >= DLT_FORMAT_HEX8) && (type <= DLT_FORMAT_HEX64))
            type_info |= DLT_SCOD_HEX;

        else if ((type >= DLT_FORMAT_BIN8) && (type <= DLT_FORMAT_BIN16))
            type_info |= DLT_SCOD_BIN;

        memcpy((log->buffer) + log->size, &(type_info), sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer) + log->size, &data, sizeof(uint16_t));
    log->size += sizeof(uint16_t);

    log->args_num++;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_uint32_formatted(DltContextData *log, uint32_t data, DltFormatType type)
{
    uint32_t type_info;

    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    /* Have to cast type to signed type because some compilers assume that DltFormatType is unsigned and issue a warning */
    if (((int16_t)type < DLT_FORMAT_DEFAULT) || (type >= DLT_FORMAT_MAX)) {
        dlt_vlog(LOG_ERR, "Format type %d is outside valid range", type);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    if ((log->size + sizeof(uint16_t)) > dlt_user.log_buf_len)
        return DLT_RETURN_USER_BUFFER_FULL;

    if (dlt_user.verbose_mode) {
        if ((log->size + sizeof(uint32_t) + sizeof(uint16_t)) > dlt_user.log_buf_len)
            return DLT_RETURN_USER_BUFFER_FULL;

        type_info = DLT_TYPE_INFO_UINT | DLT_TYLE_32BIT;

        if ((type >= DLT_FORMAT_HEX8) && (type <= DLT_FORMAT_HEX64))
            type_info |= DLT_SCOD_HEX;

        else if ((type >= DLT_FORMAT_BIN8) && (type <= DLT_FORMAT_BIN16))
            type_info |= DLT_SCOD_BIN;

        memcpy((log->buffer) + log->size, &(type_info), sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer) + log->size, &data, sizeof(uint32_t));
    log->size += sizeof(uint32_t);

    log->args_num++;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_uint64_formatted(DltContextData *log, uint64_t data, DltFormatType type)
{
    uint32_t type_info;

    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    /* Have to cast type to signed type because some compilers assume that DltFormatType is unsigned and issue a warning */
    if (((int16_t)type < DLT_FORMAT_DEFAULT) || (type >= DLT_FORMAT_MAX)) {
        dlt_vlog(LOG_ERR, "Format type %d is outside valid range", type);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    if ((log->size + sizeof(uint16_t)) > dlt_user.log_buf_len)
        return DLT_RETURN_USER_BUFFER_FULL;

    if (dlt_user.verbose_mode) {
        if ((log->size + sizeof(uint32_t) + sizeof(uint16_t)) > dlt_user.log_buf_len)
            return DLT_RETURN_USER_BUFFER_FULL;

        type_info = DLT_TYPE_INFO_UINT | DLT_TYLE_64BIT;

        if ((type >= DLT_FORMAT_HEX8) && (type <= DLT_FORMAT_HEX64))
            type_info |= DLT_SCOD_HEX;

        else if ((type >= DLT_FORMAT_BIN8) && (type <= DLT_FORMAT_BIN16))
            type_info |= DLT_SCOD_BIN;

        memcpy((log->buffer) + log->size, &(type_info), sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer) + log->size, &data, sizeof(uint64_t));
    log->size += sizeof(uint64_t);

    log->args_num++;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_ptr(DltContextData *log, void *data)
{
    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    switch (sizeof(void *)) {
    case 4:
        return dlt_user_log_write_uint32_formatted(log,
                                                   (uintptr_t)data,
                                                   DLT_FORMAT_HEX32);
        break;
    case 8:
        return dlt_user_log_write_uint64_formatted(log,
                                                   (uintptr_t)data,
                                                   DLT_FORMAT_HEX64);
        break;
    default:
        ;     /* skip */
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_int(DltContextData *log, int data)
{
    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    switch (sizeof(int)) {
    case 1:
    {
        return dlt_user_log_write_int8(log, (int8_t)data);
        break;
    }
    case 2:
    {
        return dlt_user_log_write_int16(log, (int16_t)data);
        break;
    }
    case 4:
    {
        return dlt_user_log_write_int32(log, (int32_t)data);
        break;
    }
    case 8:
    {
        return dlt_user_log_write_int64(log, (int64_t)data);
        break;
    }
    default:
    {
        return DLT_RETURN_ERROR;
        break;
    }
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_int8(DltContextData *log, int8_t data)
{
    uint32_t type_info;

    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    if ((log->size + sizeof(int8_t)) > dlt_user.log_buf_len)
        return DLT_RETURN_USER_BUFFER_FULL;

    if (dlt_user.verbose_mode) {
        if ((log->size + sizeof(uint32_t) + sizeof(int8_t)) > dlt_user.log_buf_len)
            return DLT_RETURN_USER_BUFFER_FULL;

        type_info = DLT_TYPE_INFO_SINT | DLT_TYLE_8BIT;

        memcpy((log->buffer) + log->size, &(type_info), sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer) + log->size, &data, sizeof(int8_t));
    log->size += sizeof(int8_t);

    log->args_num++;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_int16(DltContextData *log, int16_t data)
{
    uint32_t type_info;

    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    if ((log->size + sizeof(int16_t)) > dlt_user.log_buf_len)
        return DLT_RETURN_USER_BUFFER_FULL;

    if (dlt_user.verbose_mode) {
        if ((log->size + sizeof(uint32_t) + sizeof(int16_t)) > dlt_user.log_buf_len)
            return DLT_RETURN_USER_BUFFER_FULL;

        type_info = DLT_TYPE_INFO_SINT | DLT_TYLE_16BIT;

        memcpy((log->buffer) + log->size, &(type_info), sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer) + log->size, &data, sizeof(int16_t));
    log->size += sizeof(int16_t);

    log->args_num++;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_int32(DltContextData *log, int32_t data)
{
    uint32_t type_info;

    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    if ((log->size + sizeof(int32_t)) > dlt_user.log_buf_len)
        return DLT_RETURN_USER_BUFFER_FULL;

    if (dlt_user.verbose_mode) {
        if ((log->size + sizeof(uint32_t) + sizeof(int32_t)) > dlt_user.log_buf_len)
            return DLT_RETURN_USER_BUFFER_FULL;

        type_info = DLT_TYPE_INFO_SINT | DLT_TYLE_32BIT;

        memcpy((log->buffer) + log->size, &(type_info), sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer) + log->size, &data, sizeof(int32_t));
    log->size += sizeof(int32_t);

    log->args_num++;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_int64(DltContextData *log, int64_t data)
{
    uint32_t type_info;

    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    if ((log->size + sizeof(int64_t)) > dlt_user.log_buf_len)
        return DLT_RETURN_USER_BUFFER_FULL;

    if (dlt_user.verbose_mode) {
        if ((log->size + sizeof(uint32_t) + sizeof(int64_t)) > dlt_user.log_buf_len)
            return DLT_RETURN_USER_BUFFER_FULL;

        type_info = DLT_TYPE_INFO_SINT | DLT_TYLE_64BIT;

        memcpy((log->buffer) + log->size, &(type_info), sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer) + log->size, &data, sizeof(int64_t));
    log->size += sizeof(int64_t);

    log->args_num++;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_bool(DltContextData *log, uint8_t data)
{
    uint32_t type_info;

    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    if ((log->size + sizeof(uint8_t)) > dlt_user.log_buf_len)
        return DLT_RETURN_USER_BUFFER_FULL;

    if (dlt_user.verbose_mode) {
        if ((log->size + sizeof(uint32_t) + sizeof(uint8_t)) > dlt_user.log_buf_len)
            return DLT_RETURN_USER_BUFFER_FULL;

        type_info = DLT_TYPE_INFO_BOOL;

        memcpy((log->buffer) + log->size, &(type_info), sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer) + log->size, &data, sizeof(uint8_t));
    log->size += sizeof(uint8_t);

    log->args_num++;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_string(DltContextData *log, const char *text)
{
    return dlt_user_log_write_string_utils(log, text, ASCII_STRING);
}

DltReturnValue dlt_user_log_write_sized_string(DltContextData *log, const char *text, uint16_t length)
{
    return dlt_user_log_write_sized_string_utils(log, text, length, ASCII_STRING);
}

DltReturnValue dlt_user_log_write_constant_string(DltContextData *log, const char *text)
{
    /* Send parameter only in verbose mode */
    return dlt_user.verbose_mode ? dlt_user_log_write_string(log, text) : DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_sized_constant_string(DltContextData *log, const char *text, uint16_t length)
{
    /* Send parameter only in verbose mode */
    return dlt_user.verbose_mode ? dlt_user_log_write_sized_string(log, text, length) : DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_write_utf8_string(DltContextData *log, const char *text)
{
    return dlt_user_log_write_string_utils(log, text, UTF8_STRING);
}

DltReturnValue dlt_user_log_write_sized_utf8_string(DltContextData *log, const char *text, uint16_t length)
{
    return dlt_user_log_write_sized_string_utils(log, text, length, UTF8_STRING);
}

DltReturnValue dlt_user_log_write_sized_string_utils(DltContextData *log, const char *text, uint16_t length, const enum StringType type)
{
    uint16_t arg_size = 0;
    uint32_t type_info = 0;
    size_t new_log_size = 0;
    DltReturnValue ret = DLT_RETURN_OK;

    size_t str_truncate_message_length = strlen(STR_TRUNCATED_MESSAGE) + 1;
    size_t max_payload_str_msg;

    if ((log == NULL) || (text == NULL))
        return DLT_RETURN_WRONG_PARAMETER;

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_WARNING, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    arg_size = length + 1;

    new_log_size = log->size + arg_size + sizeof(uint16_t);

    if (dlt_user.verbose_mode)
        new_log_size += sizeof(uint32_t);

    /* Check log size condition */
    if (new_log_size > dlt_user.log_buf_len) {
        ret = DLT_RETURN_USER_BUFFER_FULL;

        /* Re-calculate arg_size */
        arg_size = dlt_user.log_buf_len - log->size - sizeof(uint16_t);

        size_t min_payload_str_truncate_msg = log->size + str_truncate_message_length + sizeof(uint16_t);

        if (dlt_user.verbose_mode) {
            min_payload_str_truncate_msg += sizeof(uint32_t);
            arg_size -= sizeof(uint32_t);
        }

        /* Return when dlt_user.log_buf_len does not have enough space for min_payload_str_truncate_msg */
        if (min_payload_str_truncate_msg > dlt_user.log_buf_len) {
            dlt_vlog(LOG_WARNING, "%s not enough minimum space to store data\n", __FUNCTION__);
            return ret;
        }

        /* Calculate the maximum size of string will be copied after truncate */
        max_payload_str_msg = dlt_user.log_buf_len - min_payload_str_truncate_msg;

        if (type == UTF8_STRING) {
            /**
             * Adjust the lengh to truncate one utf8 character corectly
             * refer: https://en.wikipedia.org/wiki/UTF-8
             * one utf8 character will have maximum 4 bytes then maximum bytes will be truncate additional is 3
             */
            const char *tmp = (text + max_payload_str_msg - 3);
            uint16_t reduce_size = 0;

            if (tmp[2] & 0x80) {
                /* Is the last byte of truncated text is the first byte in multi-byte sequence (utf8 2 bytes) */
                if (tmp[2] & 0x40)
                    reduce_size = 1;
                /* Is the next to last byte of truncated text is the first byte in multi-byte sequence (utf8 3 bytes) */
                else if ((tmp[1] & 0xe0) == 0xe0)
                    reduce_size = 2;
                /* utf8 4 bytes */
                else if ((tmp[0] & 0xf0) == 0xf0)
                    reduce_size = 3;
            }

            max_payload_str_msg -= reduce_size;
            arg_size -= reduce_size;
        }
    }

    if (dlt_user.verbose_mode) {
        switch (type) {
        case ASCII_STRING:
        {
            type_info = DLT_TYPE_INFO_STRG | DLT_SCOD_ASCII;
            break;
        }
        case UTF8_STRING:
        {
            type_info = DLT_TYPE_INFO_STRG | DLT_SCOD_UTF8;
            break;
        }
        default:
        {
            /* Do nothing */
            break;
        }
        }

        memcpy((log->buffer) + log->size, &(type_info), sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer) + log->size, &(arg_size), sizeof(uint16_t));
    log->size += sizeof(uint16_t);

    switch (ret) {
    case DLT_RETURN_OK:
    {
        /* Whole string will be copied */
        memcpy((log->buffer) + log->size, text, length);
        /* The input string might not be null-terminated, so we're doing that by ourselves */
        log->buffer[log->size + length] = '\0';
        log->size += arg_size;
        break;
    }
    case DLT_RETURN_USER_BUFFER_FULL:
    {
        /* Only copy partial string */
        memcpy((log->buffer) + log->size, text, max_payload_str_msg);
        log->size += max_payload_str_msg;

        /* Append string truncate the input string */
        memcpy((log->buffer) + log->size, STR_TRUNCATED_MESSAGE, str_truncate_message_length);
        log->size += str_truncate_message_length;
        break;
    }
    default:
    {
        /* Do nothing */
        break;
    }
    }

    log->args_num++;

    return ret;
}

DltReturnValue dlt_user_log_write_string_utils(DltContextData *log, const char *text, const enum StringType type)
{
    if ((log == NULL) || (text == NULL))
        return DLT_RETURN_WRONG_PARAMETER;

    uint16_t length = strlen(text);
    return dlt_user_log_write_sized_string_utils(log, text, length, type);
}

DltReturnValue dlt_register_injection_callback_with_id(DltContext *handle, uint32_t service_id,
                                                       dlt_injection_callback_id dlt_injection_cbk, void *priv)
{
    DltContextData log;
    uint32_t i, j, k;
    int found = 0;

    DltUserInjectionCallback *old;

    if (dlt_user_log_init(handle, &log) < DLT_RETURN_OK)
        return DLT_RETURN_ERROR;

    if (service_id < DLT_USER_INJECTION_MIN)
        return DLT_RETURN_WRONG_PARAMETER;

    /* This function doesn't make sense storing to local file is choosen;
     * so terminate this function */
    if (dlt_user.dlt_is_file)
        return DLT_RETURN_OK;

    DLT_SEM_LOCK();

    if (dlt_user.dlt_ll_ts == NULL) {
        DLT_SEM_FREE();
        return DLT_RETURN_OK;
    }

    /* Insert callback in corresponding table */
    i = handle->log_level_pos;

    /* Insert each service_id only once */
    for (k = 0; k < dlt_user.dlt_ll_ts[i].nrcallbacks; k++)
        if ((dlt_user.dlt_ll_ts[i].injection_table) &&
            (dlt_user.dlt_ll_ts[i].injection_table[k].service_id == service_id)) {
            found = 1;
            break;
        }

    if (found) {
        j = k;
    }
    else {
        j = dlt_user.dlt_ll_ts[i].nrcallbacks;

        /* Allocate or expand injection table */
        if (dlt_user.dlt_ll_ts[i].injection_table == NULL) {
            dlt_user.dlt_ll_ts[i].injection_table =
                (DltUserInjectionCallback *)malloc(sizeof(DltUserInjectionCallback));

            if (dlt_user.dlt_ll_ts[i].injection_table == NULL) {
                DLT_SEM_FREE();
                return DLT_RETURN_ERROR;
            }
        }
        else {
            old = dlt_user.dlt_ll_ts[i].injection_table;
            dlt_user.dlt_ll_ts[i].injection_table = (DltUserInjectionCallback *)malloc(
                sizeof(DltUserInjectionCallback) * (j + 1));

            if (dlt_user.dlt_ll_ts[i].injection_table == NULL) {
                dlt_user.dlt_ll_ts[i].injection_table = old;
                DLT_SEM_FREE();
                return DLT_RETURN_ERROR;
            }

            memcpy(dlt_user.dlt_ll_ts[i].injection_table, old, sizeof(DltUserInjectionCallback) * j);
            free(old);
        }

        dlt_user.dlt_ll_ts[i].nrcallbacks++;
    }

    /* Store service_id and corresponding function pointer for callback function */
    dlt_user.dlt_ll_ts[i].injection_table[j].service_id = service_id;

    if (priv == NULL) {
        dlt_user.dlt_ll_ts[i].injection_table[j].injection_callback = (dlt_injection_callback)(void*)dlt_injection_cbk;
        dlt_user.dlt_ll_ts[i].injection_table[j].injection_callback_with_id = NULL;
        dlt_user.dlt_ll_ts[i].injection_table[j].data = NULL;
    }
    else {
        dlt_user.dlt_ll_ts[i].injection_table[j].injection_callback = NULL;
        dlt_user.dlt_ll_ts[i].injection_table[j].injection_callback_with_id = dlt_injection_cbk;
        dlt_user.dlt_ll_ts[i].injection_table[j].data = priv;
    }

    DLT_SEM_FREE();

    return DLT_RETURN_OK;
}

DltReturnValue dlt_register_injection_callback(DltContext *handle, uint32_t service_id,
                                               int (*dlt_injection_callback)(uint32_t service_id,
                                                                             void *data,
                                                                             uint32_t length))
{
    return dlt_register_injection_callback_with_id(handle,
                                                   service_id,
                                                   (dlt_injection_callback_id)(void*)dlt_injection_callback,
                                                   NULL);
}

DltReturnValue dlt_register_log_level_changed_callback(DltContext *handle,
                                                       void (*dlt_log_level_changed_callback)(
                                                           char context_id[DLT_ID_SIZE],
                                                           uint8_t log_level,
                                                           uint8_t trace_status))
{
    DltContextData log;
    uint32_t i;

    if (dlt_user_log_init(handle, &log) < DLT_RETURN_OK)
        return DLT_RETURN_ERROR;

    /* This function doesn't make sense storing to local file is choosen;
     * so terminate this function */
    if (dlt_user.dlt_is_file)
        return DLT_RETURN_OK;

    DLT_SEM_LOCK();

    if (dlt_user.dlt_ll_ts == NULL) {
        DLT_SEM_FREE();
        return DLT_RETURN_OK;
    }

    /* Insert callback in corresponding table */
    i = handle->log_level_pos;

    /* Store new callback function */
    dlt_user.dlt_ll_ts[i].log_level_changed_callback = dlt_log_level_changed_callback;

    DLT_SEM_FREE();

    return DLT_RETURN_OK;
}

/**
 * NW Trace related
 */

#ifdef DLT_NETWORK_TRACE_ENABLE
int check_buffer(void)
{
    int total_size, used_size;
    dlt_user_check_buffer(&total_size, &used_size);

    return (total_size - used_size < total_size / 2) ? -1 : 1;
}

/**
 * Send the start of a segment chain.
 * Returns DLT_RETURN_ERROR on failure
 */
DltReturnValue dlt_user_trace_network_segmented_start(uint32_t *id,
                                                      DltContext *handle,
                                                      DltNetworkTraceType nw_trace_type,
                                                      uint16_t header_len,
                                                      void *header,
                                                      uint16_t payload_len)
{
    DltContextData log = { 0 };
    struct timeval tv;
    int ret = DLT_RETURN_ERROR;

    /* check null pointer */
    if (id == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if ((nw_trace_type < DLT_NW_TRACE_IPC) || (nw_trace_type >= DLT_NW_TRACE_MAX)) {
        dlt_vlog(LOG_ERR, "Network trace type %d is outside valid range", nw_trace_type);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (dlt_user.dlt_ll_ts == NULL)
        return DLT_RETURN_ERROR;

    if (handle->trace_status_ptr && (*(handle->trace_status_ptr) == DLT_TRACE_STATUS_ON)) {
        /* initialize values */
        if (dlt_user_log_init(handle, &log) < DLT_RETURN_OK)
            return DLT_RETURN_ERROR;

        if (log.buffer == NULL) {
            log.buffer = calloc(sizeof(unsigned char), dlt_user.log_buf_len);

            if (log.buffer == NULL) {
                dlt_vlog(LOG_ERR, "Cannot allocate buffer for DLT Log message\n");
                return DLT_RETURN_ERROR;
            }
        }

        log.args_num = 0;
        log.trace_status = nw_trace_type;
        log.size = 0;

        gettimeofday(&tv, NULL);
        *id = tv.tv_usec;

        /* Write identifier */
        if (dlt_user_log_write_string(&log, DLT_TRACE_NW_START) < 0) {
            dlt_user_free_buffer(&(log.buffer));
            return DLT_RETURN_ERROR;
        }

        /* Write stream handle */
        if (dlt_user_log_write_uint32(&log, *id) < 0) {
            dlt_user_free_buffer(&(log.buffer));
            return DLT_RETURN_ERROR;
        }

        /* Write header */
        if (dlt_user_log_write_raw(&log, header, header_len) < 0) {
            dlt_user_free_buffer(&(log.buffer));
            return DLT_RETURN_ERROR;
        }

        /* Write size of payload */
        if (dlt_user_log_write_uint32(&log, payload_len) < 0) {
            dlt_user_free_buffer(&(log.buffer));
            return DLT_RETURN_ERROR;
        }

        /* Write expected segment count */
        uint16_t segment_count = payload_len / DLT_MAX_TRACE_SEGMENT_SIZE + 1;

        /* If segments align perfectly with segment size, avoid sending empty segment */
        if ((payload_len % DLT_MAX_TRACE_SEGMENT_SIZE) == 0)
            segment_count--;

        if (dlt_user_log_write_uint16(&log, segment_count) < 0) {
            dlt_user_free_buffer(&(log.buffer));
            return DLT_RETURN_ERROR;
        }

        /* Write length of one segment */
        if (dlt_user_log_write_uint16(&log, DLT_MAX_TRACE_SEGMENT_SIZE) < 0) {
            dlt_user_free_buffer(&(log.buffer));
            return DLT_RETURN_ERROR;
        }

        /* Send log */
        ret = dlt_user_log_send_log(&log, DLT_TYPE_NW_TRACE);

        dlt_user_free_buffer(&(log.buffer));

        return ret;
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_trace_network_segmented_segment(uint32_t id,
                                                        DltContext *handle,
                                                        DltNetworkTraceType nw_trace_type,
                                                        int sequence,
                                                        uint16_t payload_len,
                                                        void *payload)
{
    int ret = DLT_RETURN_ERROR;
    struct timespec ts;

    if ((nw_trace_type < DLT_NW_TRACE_IPC) || (nw_trace_type >= DLT_NW_TRACE_MAX)) {
        dlt_vlog(LOG_ERR, "Network trace type %d is outside valid range", nw_trace_type);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    while (check_buffer() < 0) {
        /* Wait 50ms */
        ts.tv_sec = 0;
        ts.tv_nsec = 1000000 * 50;
        nanosleep(&ts, NULL);
        dlt_user_log_resend_buffer();
    }

    if (dlt_user.dlt_ll_ts == NULL)
        return DLT_RETURN_ERROR;

    if (handle->trace_status_ptr && (*(handle->trace_status_ptr) == DLT_TRACE_STATUS_ON)) {
        DltContextData log = { 0 };

        if (dlt_user_log_init(handle, &log) < DLT_RETURN_OK)
            return DLT_RETURN_ERROR;

        /* initialize values */
        if (log.buffer == NULL) {
            log.buffer = calloc(sizeof(unsigned char), dlt_user.log_buf_len);

            if (log.buffer == NULL) {
                dlt_vlog(LOG_ERR, "Cannot allocate buffer for DLT Log message\n");
                return DLT_RETURN_ERROR;
            }
        }

        log.args_num = 0;
        log.trace_status = nw_trace_type;
        log.size = 0;

        /* Write identifier */
        if (dlt_user_log_write_string(&log, DLT_TRACE_NW_SEGMENT) < DLT_RETURN_OK) {
            dlt_user_free_buffer(&(log.buffer));
            return DLT_RETURN_ERROR;
        }

        /* Write stream handle */
        if (dlt_user_log_write_uint32(&log, id) < DLT_RETURN_OK) {
            dlt_user_free_buffer(&(log.buffer));
            return DLT_RETURN_ERROR;
        }

        /* Write segment sequence number */
        if (dlt_user_log_write_uint16(&log, sequence) < DLT_RETURN_OK) {
            dlt_user_free_buffer(&(log.buffer));
            return DLT_RETURN_ERROR;
        }

        /* Write data */
        if (dlt_user_log_write_raw(&log, payload, payload_len) < DLT_RETURN_OK) {
            dlt_user_free_buffer(&(log.buffer));
            return DLT_RETURN_ERROR;
        }

        ret = dlt_user_log_send_log(&log, DLT_TYPE_NW_TRACE);
        /* Send log */

        dlt_user_free_buffer(&(log.buffer));

        return ret;
    }

    /* Allow other threads to log between chunks */
    sched_yield();
    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_trace_network_segmented_end(uint32_t id, DltContext *handle, DltNetworkTraceType nw_trace_type)
{
    DltContextData log = { 0 };
    int ret = DLT_RETURN_ERROR;

    if ((nw_trace_type < DLT_NW_TRACE_IPC) || (nw_trace_type >= DLT_NW_TRACE_MAX)) {
        dlt_vlog(LOG_ERR, "Network trace type %d is outside valid range", nw_trace_type);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (dlt_user.dlt_ll_ts == NULL)
        return DLT_RETURN_ERROR;

    if (handle->trace_status_ptr && (*(handle->trace_status_ptr) == DLT_TRACE_STATUS_ON)) {
        /* initialize values */
        if (dlt_user_log_init(handle, &log) < DLT_RETURN_OK)
            return DLT_RETURN_ERROR;

        /* initialize values */
        if (log.buffer == NULL) {
            log.buffer = calloc(sizeof(unsigned char), dlt_user.log_buf_len);

            if (log.buffer == NULL) {
                dlt_vlog(LOG_ERR, "Cannot allocate buffer for DLT Log message\n");
                return DLT_RETURN_ERROR;
            }
        }

        log.args_num = 0;
        log.trace_status = nw_trace_type;
        log.size = 0;

        /* Write identifier */
        if (dlt_user_log_write_string(&log, DLT_TRACE_NW_END) < DLT_RETURN_OK) {
            dlt_user_free_buffer(&(log.buffer));
            return DLT_RETURN_ERROR;
        }

        /* Write stream handle */
        if (dlt_user_log_write_uint32(&log, id) < DLT_RETURN_OK) {
            dlt_user_free_buffer(&(log.buffer));
            return DLT_RETURN_ERROR;
        }

        ret = dlt_user_log_send_log(&log, DLT_TYPE_NW_TRACE);
        /* Send log */

        dlt_user_free_buffer(&(log.buffer));

        return ret;

    }

    return DLT_RETURN_OK;
}

void dlt_user_trace_network_segmented_thread(void *unused)
{
    /* Unused on purpose. */
    (void)unused;
#ifdef linux
    prctl(PR_SET_NAME, "dlt_segmented", 0, 0, 0);
#endif
    pthread_cleanup_push(dlt_user_cleanup_handler, NULL);

    s_segmented_data *data;

    while (1) {
        /* Wait until message queue is initialized */
        dlt_lock_mutex(&mq_mutex);

        while (dlt_user.dlt_segmented_queue_read_handle < 0)
        {
            pthread_cond_wait(&mq_init_condition, &mq_mutex);
        }

        dlt_unlock_mutex(&mq_mutex);

        ssize_t read = mq_receive(dlt_user.dlt_segmented_queue_read_handle, (char *)&data,
                                  sizeof(s_segmented_data *), NULL);

        if (read < 0) {
            if (errno != EINTR) {
                struct timespec req;
                long sec = (DLT_USER_MQ_ERROR_RETRY_INTERVAL / 1000000);
                dlt_vlog(LOG_WARNING, "NWTSegmented: Error while reading queue: %s\n", strerror(errno));
                req.tv_sec = sec;
                req.tv_nsec = (DLT_USER_MQ_ERROR_RETRY_INTERVAL - sec * 1000000) * 1000;
                nanosleep(&req, NULL);
            }

            continue;
        }

        if (read != sizeof(s_segmented_data *)) {
            /* This case will not happen. */
            /* When this thread is interrupted by signal, mq_receive() will not return */
            /* partial read length and will return -1. And also no data is removed from mq. */
            dlt_vlog(LOG_WARNING, "NWTSegmented: Could not read data fully from queue: %zd\n", read);
            continue;
        }

        dlt_user_trace_network_segmented_thread_segmenter(data);

        /* Send the end message */
        DltReturnValue err = dlt_user_trace_network_segmented_end(data->id, data->handle, data->nw_trace_type);

        if ((err == DLT_RETURN_BUFFER_FULL) || (err == DLT_RETURN_ERROR))
            dlt_log(LOG_WARNING, "NWTSegmented: Could not send end segment.\n");

        /* Free resources */
        free(data->header);
        free(data->payload);
        free(data);
    }

    pthread_cleanup_pop(1);
}

void dlt_user_trace_network_segmented_thread_segmenter(s_segmented_data *data)
{
    /* Segment the data and send the chunks */
    void *ptr = NULL;
    uint32_t offset = 0;
    uint16_t sequence = 0;

    do {
        uint16_t len = 0;

        if (offset + DLT_MAX_TRACE_SEGMENT_SIZE > data->payload_len)
            len = data->payload_len - offset;
        else
            len = DLT_MAX_TRACE_SEGMENT_SIZE;

        /* If payload size aligns perfectly with segment size, avoid sending empty segment */
        if (len == 0)
            break;

        ptr = data->payload + offset;
        DltReturnValue err = dlt_user_trace_network_segmented_segment(data->id,
                                                                      data->handle,
                                                                      data->nw_trace_type,
                                                                      sequence++,
                                                                      len,
                                                                      ptr);

        if ((err == DLT_RETURN_BUFFER_FULL) || (err == DLT_RETURN_ERROR)) {
            dlt_log(LOG_ERR, "NWTSegmented: Could not send segment. Aborting.\n");
            break;             /* loop */
        }

        offset += len;
    } while (ptr < data->payload + data->payload_len);
}


DltReturnValue dlt_user_trace_network_segmented(DltContext *handle,
                                                DltNetworkTraceType nw_trace_type,
                                                uint16_t header_len,
                                                void *header,
                                                uint16_t payload_len,
                                                void *payload)
{
    /* forbid dlt usage in child after fork */
    if (g_dlt_is_child)
        return DLT_RETURN_ERROR;

    /* Send as normal trace if possible */
    if (header_len + payload_len + sizeof(uint16_t) < dlt_user.log_buf_len)
        return dlt_user_trace_network(handle, nw_trace_type, header_len, header, payload_len, payload);

    /* Allocate Memory */
    s_segmented_data *thread_data = malloc(sizeof(s_segmented_data));

    if (thread_data == NULL)
        return DLT_RETURN_ERROR;

    thread_data->header = malloc(header_len);

    if (thread_data->header == NULL) {
        free(thread_data);
        return DLT_RETURN_ERROR;
    }

    thread_data->payload = malloc(payload_len);

    if (thread_data->payload == NULL) {
        free(thread_data->header);
        free(thread_data);
        return DLT_RETURN_ERROR;
    }

    /* Copy data */
    thread_data->handle = handle;
    thread_data->nw_trace_type = nw_trace_type;
    thread_data->header_len = header_len;
    memcpy(thread_data->header, header, header_len);
    thread_data->payload_len = payload_len;
    memcpy(thread_data->payload, payload, payload_len);

    /* Send start message */
    DltReturnValue err = dlt_user_trace_network_segmented_start(&(thread_data->id),
                                                                thread_data->handle, thread_data->nw_trace_type,
                                                                thread_data->header_len, thread_data->header,
                                                                thread_data->payload_len);

    if ((err == DLT_RETURN_BUFFER_FULL) || (err == DLT_RETURN_ERROR)) {
        dlt_log(LOG_ERR, "NWTSegmented: Could not send start segment. Aborting.\n");
        free(thread_data->header);
        free(thread_data->payload);
        free(thread_data);
        return DLT_RETURN_ERROR;
    }

    /* Open queue if it is not open */
    if (dlt_init_message_queue() < 0) {
        dlt_log(LOG_ERR, "NWTSegmented: Could not open queue.\n");
        free(thread_data->header);
        free(thread_data->payload);
        free(thread_data);

        return DLT_RETURN_ERROR;
    }

    /* Add to queue */
    if (mq_send(dlt_user.dlt_segmented_queue_write_handle,
                (char *)&thread_data, sizeof(s_segmented_data *), 1) < 0) {
        if (errno == EAGAIN)
            dlt_log(LOG_WARNING, "NWTSegmented: Queue full. Message discarded.\n");

        free(thread_data->header);
        free(thread_data->payload);
        free(thread_data);
        dlt_vnlog(LOG_WARNING, 256, "NWTSegmented: Could not write into queue: %s \n", strerror(errno));
        return DLT_RETURN_ERROR;
    }

    /*thread_data will be freed by the receiver function */
    /*coverity[leaked_storage] */
    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_trace_network(DltContext *handle,
                                      DltNetworkTraceType nw_trace_type,
                                      uint16_t header_len,
                                      void *header,
                                      uint16_t payload_len,
                                      void *payload)
{
    return dlt_user_trace_network_truncated(handle, nw_trace_type, header_len, header, payload_len, payload, 1);
}

DltReturnValue dlt_user_trace_network_truncated(DltContext *handle,
                                                DltNetworkTraceType nw_trace_type,
                                                uint16_t header_len,
                                                void *header,
                                                uint16_t payload_len,
                                                void *payload,
                                                int allow_truncate)
{
    int ret = DLT_RETURN_ERROR;
    DltContextData log = { 0 };

    if ((payload == NULL) && (payload_len > 0))
        return DLT_RETURN_WRONG_PARAMETER;

    if ((nw_trace_type < DLT_NW_TRACE_IPC) || (nw_trace_type >= DLT_NW_TRACE_MAX)) {
        dlt_vlog(LOG_ERR, "Network trace type %d is outside valid range", nw_trace_type);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (dlt_user.dlt_ll_ts == NULL)
        return DLT_RETURN_ERROR;

    if (handle->trace_status_ptr && (*(handle->trace_status_ptr) == DLT_TRACE_STATUS_ON)) {
        if ((dlt_user_log_init(handle, &log) < DLT_RETURN_OK) || (dlt_user.dlt_ll_ts == NULL))
            return DLT_RETURN_ERROR;

        /* initialize values */
        if (log.buffer == NULL) {
            log.buffer = calloc(sizeof(unsigned char), dlt_user.log_buf_len);

            if (log.buffer == NULL) {
                dlt_vlog(LOG_ERR, "Cannot allocate buffer for DLT Log message\n");
                return DLT_RETURN_ERROR;
            }
        }

        log.args_num = 0;
        log.trace_status = nw_trace_type;
        log.size = 0;

        if (header == NULL)
            header_len = 0;

        /* If truncation is allowed, check if we must do it */
        if ((allow_truncate > 0) && ((header_len + payload_len + sizeof(uint16_t)) > dlt_user.log_buf_len)) {
            /* Identify as truncated */
            if (dlt_user_log_write_string(&log, DLT_TRACE_NW_TRUNCATED) < DLT_RETURN_OK) {
                dlt_user_free_buffer(&(log.buffer));
                return DLT_RETURN_ERROR;
            }

            /* Write header and its length */
            if (dlt_user_log_write_raw(&log, header, header_len) < DLT_RETURN_OK) {
                dlt_user_free_buffer(&(log.buffer));
                return DLT_RETURN_ERROR;
            }

            /* Write original size of payload */
            if (dlt_user_log_write_uint32(&log, payload_len) < DLT_RETURN_OK) {
                dlt_user_free_buffer(&(log.buffer));
                return DLT_RETURN_ERROR;
            }

            /**
             *  Calculate maximum available space in sending buffer after headers.
             */

            int truncated_payload_len = dlt_user.log_buf_len - log.size - sizeof(uint16_t) - sizeof(uint32_t);

            /* Write truncated payload */
            if (dlt_user_log_write_raw(&log, payload, truncated_payload_len) < DLT_RETURN_OK) {
                dlt_user_free_buffer(&(log.buffer));
                return DLT_RETURN_ERROR;
            }
        }
        else { /* Truncation not allowed or data short enough */

            /* Write header and its length */
            if (dlt_user_log_write_raw(&log, header, header_len) < DLT_RETURN_OK) {
                dlt_user_free_buffer(&(log.buffer));
                return DLT_RETURN_ERROR;
            }

            if (payload == NULL)
                payload_len = 0;

            /* Write payload and its length */
            if (dlt_user_log_write_raw(&log, payload, payload_len) < DLT_RETURN_OK) {
                dlt_user_free_buffer(&(log.buffer));
                return DLT_RETURN_ERROR;
            }
        }

        ret = dlt_user_log_send_log(&log, DLT_TYPE_NW_TRACE);

        dlt_user_free_buffer(&(log.buffer));

        /* Send log */
        return ret;
    }

    return DLT_RETURN_OK;
}
#endif /* DLT_NETWORK_TRACE_ENABLE */

DltReturnValue dlt_log_string(DltContext *handle, DltLogLevelType loglevel, const char *text)
{
    DltReturnValue ret = DLT_RETURN_OK;
    DltContextData log;

    if (dlt_user.verbose_mode == 0)
        return DLT_RETURN_ERROR;

    if ((handle == NULL) || (text == NULL))
        return DLT_RETURN_WRONG_PARAMETER;

    if (dlt_user_log_write_start(handle, &log, loglevel) == DLT_RETURN_TRUE) {
        ret = dlt_user_log_write_string(&log, text);

        if (dlt_user_log_write_finish(&log) < DLT_RETURN_OK)
            ret = DLT_RETURN_ERROR;
    }

    return ret;
}

DltReturnValue dlt_log_string_int(DltContext *handle, DltLogLevelType loglevel, const char *text, int data)
{
    DltReturnValue ret = DLT_RETURN_OK;
    DltContextData log;

    if (dlt_user.verbose_mode == 0)
        return DLT_RETURN_ERROR;

    if ((handle == NULL) || (text == NULL))
        return DLT_RETURN_WRONG_PARAMETER;

    if (dlt_user_log_write_start(handle, &log, loglevel) == DLT_RETURN_TRUE) {
        ret = dlt_user_log_write_string(&log, text);
        dlt_user_log_write_int(&log, data);

        if (dlt_user_log_write_finish(&log) < DLT_RETURN_OK)
            ret = DLT_RETURN_ERROR;
    }

    return ret;
}

DltReturnValue dlt_log_string_uint(DltContext *handle, DltLogLevelType loglevel, const char *text, unsigned int data)
{
    DltReturnValue ret = DLT_RETURN_OK;
    DltContextData log;

    if (dlt_user.verbose_mode == 0)
        return DLT_RETURN_ERROR;

    if ((handle == NULL) || (text == NULL))
        return DLT_RETURN_WRONG_PARAMETER;

    if (dlt_user_log_write_start(handle, &log, loglevel) == DLT_RETURN_TRUE) {
        ret = dlt_user_log_write_string(&log, text);
        dlt_user_log_write_uint(&log, data);

        if (dlt_user_log_write_finish(&log) < DLT_RETURN_OK)
            ret = DLT_RETURN_ERROR;
    }

    return ret;
}

DltReturnValue dlt_log_int(DltContext *handle, DltLogLevelType loglevel, int data)
{
    DltContextData log;

    if (dlt_user.verbose_mode == 0)
        return DLT_RETURN_ERROR;

    if (handle == NULL)
        return DLT_RETURN_ERROR;

    if (dlt_user_log_write_start(handle, &log, loglevel) == DLT_RETURN_TRUE) {
        dlt_user_log_write_int(&log, data);

        if (dlt_user_log_write_finish(&log) < DLT_RETURN_OK)
            return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_log_uint(DltContext *handle, DltLogLevelType loglevel, unsigned int data)
{
    DltContextData log;

    if (dlt_user.verbose_mode == 0)
        return DLT_RETURN_ERROR;

    if (handle == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (dlt_user_log_write_start(handle, &log, loglevel) == DLT_RETURN_TRUE) {
        dlt_user_log_write_uint(&log, data);

        if (dlt_user_log_write_finish(&log) < DLT_RETURN_OK)
            return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_log_raw(DltContext *handle, DltLogLevelType loglevel, void *data, uint16_t length)
{
    DltContextData log;
    DltReturnValue ret = DLT_RETURN_OK;

    if (dlt_user.verbose_mode == 0)
        return DLT_RETURN_ERROR;

    if (handle == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (dlt_user_log_write_start(handle, &log, loglevel) > 0) {
        if ((ret = dlt_user_log_write_raw(&log, data, length)) < DLT_RETURN_OK) {
            dlt_user_free_buffer(&(log.buffer));
            return ret;
        }

        if (dlt_user_log_write_finish(&log) < DLT_RETURN_OK)
            return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_log_marker()
{
    if (!dlt_user_initialised) {
        if (dlt_init() < DLT_RETURN_OK) {
            dlt_vlog(LOG_ERR, "%s Failed to initialise dlt", __FUNCTION__);
            return DLT_RETURN_ERROR;
        }
    }

    return dlt_user_log_send_marker();
}

DltReturnValue dlt_verbose_mode(void)
{
    if (!dlt_user_initialised) {
        if (dlt_init() < DLT_RETURN_OK) {
            dlt_vlog(LOG_ERR, "%s Failed to initialise dlt", __FUNCTION__);
            return DLT_RETURN_ERROR;
        }
    }

    /* Switch to verbose mode */
    dlt_user.verbose_mode = 1;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_nonverbose_mode(void)
{
    if (!dlt_user_initialised) {
        if (dlt_init() < DLT_RETURN_OK) {
            dlt_vlog(LOG_ERR, "%s Failed to initialise dlt", __FUNCTION__);
            return DLT_RETURN_ERROR;
        }
    }

    /* Switch to non-verbose mode */
    dlt_user.verbose_mode = 0;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_use_extended_header_for_non_verbose(int8_t use_extended_header_for_non_verbose)
{
    if (!dlt_user_initialised) {
        if (dlt_init() < DLT_RETURN_OK) {
            dlt_vlog(LOG_ERR, "%s Failed to initialise dlt", __FUNCTION__);
            return DLT_RETURN_ERROR;
        }
    }

    /* Set use_extended_header_for_non_verbose */
    dlt_user.use_extended_header_for_non_verbose = use_extended_header_for_non_verbose;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_with_session_id(int8_t with_session_id)
{
    if (!dlt_user_initialised) {
        if (dlt_init() < DLT_RETURN_OK) {
            dlt_vlog(LOG_ERR, "%s Failed to initialise dlt", __FUNCTION__);
            return DLT_RETURN_ERROR;
        }
    }

    /* Set use_extended_header_for_non_verbose */
    dlt_user.with_session_id = with_session_id;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_with_timestamp(int8_t with_timestamp)
{
    if (!dlt_user_initialised) {
        if (dlt_init() < DLT_RETURN_OK) {
            dlt_vlog(LOG_ERR, "%s Failed to initialise dlt", __FUNCTION__);
            return DLT_RETURN_ERROR;
        }
    }

    /* Set with_timestamp */
    dlt_user.with_timestamp = with_timestamp;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_with_ecu_id(int8_t with_ecu_id)
{
    if (!dlt_user_initialised) {
        if (dlt_init() < DLT_RETURN_OK) {
            dlt_vlog(LOG_ERR, "%s Failed to initialise dlt", __FUNCTION__);
            return DLT_RETURN_ERROR;
        }
    }

    /* Set with_timestamp */
    dlt_user.with_ecu_id = with_ecu_id;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_enable_local_print(void)
{
    if (!dlt_user_initialised) {
        if (dlt_init() < DLT_RETURN_OK) {
            dlt_vlog(LOG_ERR, "%s Failed to initialise dlt", __FUNCTION__);
            return DLT_RETURN_ERROR;
        }
    }

    dlt_user.enable_local_print = 1;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_disable_local_print(void)
{
    if (!dlt_user_initialised) {
        if (dlt_init() < DLT_RETURN_OK) {
            dlt_vlog(LOG_ERR, "%s Failed to initialise dlt", __FUNCTION__);
            return DLT_RETURN_ERROR;
        }
    }

    dlt_user.enable_local_print = 0;

    return DLT_RETURN_OK;
}

/* Cleanup on thread cancellation, thread may hold lock release it here */
static void dlt_user_cleanup_handler(void *arg)
{
    DLT_UNUSED(arg); /* Satisfy compiler */

#ifdef DLT_NETWORK_TRACE_ENABLE
    /* unlock the message queue */
    dlt_unlock_mutex(&mq_mutex);
#endif

    /* unlock DLT (dlt_mutex) */
    DLT_SEM_FREE();
}

void dlt_user_housekeeperthread_function(__attribute__((unused)) void *ptr)
{
    struct timespec ts;
#ifdef linux
    prctl(PR_SET_NAME, "dlt_housekeeper", 0, 0, 0);
#endif

    pthread_cleanup_push(dlt_user_cleanup_handler, NULL);

    while (1) {
        /* Check for new messages from DLT daemon */
        if (dlt_user_log_check_user_message() < DLT_RETURN_OK)
            /* Critical error */
            dlt_log(LOG_CRIT, "Housekeeper thread encountered error condition\n");

        /* Reattach to daemon if neccesary */
        dlt_user_log_reattach_to_daemon();

        /* flush buffer to DLT daemon if possible */
        if (dlt_user.dlt_log_handle != DLT_FD_INIT)
            dlt_user_log_resend_buffer();
        /* delay */
        ts.tv_sec = 0;
        ts.tv_nsec = DLT_USER_RECEIVE_NDELAY;
        nanosleep(&ts, NULL);
    }

    pthread_cleanup_pop(1);
}

/* Private functions of user library */

DltReturnValue dlt_user_log_init(DltContext *handle, DltContextData *log)
{
    int ret = DLT_RETURN_OK;

    if ((handle == NULL) || (log == NULL))
        return DLT_RETURN_WRONG_PARAMETER;

    if (!dlt_user_initialised) {
        ret = dlt_init();

        if (ret < DLT_RETURN_OK) {
            if (ret != DLT_RETURN_LOGGING_DISABLED)
                dlt_vlog(LOG_ERR, "%s Failed to initialise dlt", __FUNCTION__);

            return ret;
        }
    }

    log->handle = handle;
    log->buffer = NULL;
    return ret;
}

DltReturnValue dlt_user_log_send_log(DltContextData *log, int mtype)
{
    DltMessage msg;
    DltUserHeader userheader;
    int32_t len;

    DltReturnValue ret = DLT_RETURN_OK;

    if (!dlt_user_initialised) {
        dlt_vlog(LOG_ERR, "%s dlt_user_initialised false\n", __FUNCTION__);
        return DLT_RETURN_ERROR;
    }

    if ((log == NULL) ||
        (log->handle == NULL) ||
        (log->handle->contextID[0] == '\0') ||
        (mtype < DLT_TYPE_LOG) || (mtype > DLT_TYPE_CONTROL)
        )
        return DLT_RETURN_WRONG_PARAMETER;

    /* also for Trace messages */
    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_LOG) < DLT_RETURN_OK)
        return DLT_RETURN_ERROR;

    if (dlt_message_init(&msg, 0) == DLT_RETURN_ERROR)
        return DLT_RETURN_ERROR;

    msg.storageheader = (DltStorageHeader *)msg.headerbuffer;

    if (dlt_set_storageheader(msg.storageheader, dlt_user.ecuID) == DLT_RETURN_ERROR)
        return DLT_RETURN_ERROR;

    msg.standardheader = (DltStandardHeader *)(msg.headerbuffer + sizeof(DltStorageHeader));
    msg.standardheader->htyp = DLT_HTYP_PROTOCOL_VERSION1;

    /* send ecu id */
    if (dlt_user.with_ecu_id)
        msg.standardheader->htyp |= DLT_HTYP_WEID;

    /* send timestamp */
    if (dlt_user.with_timestamp)
        msg.standardheader->htyp |= DLT_HTYP_WTMS;

    /* send session id */
    if (dlt_user.with_session_id) {
        msg.standardheader->htyp |= DLT_HTYP_WSID;
        msg.headerextra.seid = getpid();
    }

    if (dlt_user.verbose_mode)
        /* In verbose mode, send extended header */
        msg.standardheader->htyp = (msg.standardheader->htyp | DLT_HTYP_UEH);
    else
        /* In non-verbose, send extended header if desired */
        if (dlt_user.use_extended_header_for_non_verbose)
            msg.standardheader->htyp = (msg.standardheader->htyp | DLT_HTYP_UEH);

#if (BYTE_ORDER == BIG_ENDIAN)
    msg.standardheader->htyp = (msg.standardheader->htyp | DLT_HTYP_MSBF);
#endif

    msg.standardheader->mcnt = log->handle->mcnt++;

    /* Set header extra parameters */
    dlt_set_id(msg.headerextra.ecu, dlt_user.ecuID);

    /*msg.headerextra.seid = 0; */
    if (log->use_timestamp == DLT_AUTO_TIMESTAMP) {
        msg.headerextra.tmsp = dlt_uptime();
    }
    else {
        msg.headerextra.tmsp = log->user_timestamp;
    }

    if (dlt_message_set_extraparameters(&msg, 0) == DLT_RETURN_ERROR)
        return DLT_RETURN_ERROR;

    /* Fill out extended header, if extended header should be provided */
    if (DLT_IS_HTYP_UEH(msg.standardheader->htyp)) {
        /* with extended header */
        msg.extendedheader =
            (DltExtendedHeader *)(msg.headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader) +
                                  DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp));

        switch (mtype) {
        case DLT_TYPE_LOG:
        {
            msg.extendedheader->msin = (DLT_TYPE_LOG << DLT_MSIN_MSTP_SHIFT) |
                ((log->log_level << DLT_MSIN_MTIN_SHIFT) & DLT_MSIN_MTIN);
            break;
        }
        case DLT_TYPE_NW_TRACE:
        {
            msg.extendedheader->msin = (DLT_TYPE_NW_TRACE << DLT_MSIN_MSTP_SHIFT) |
                ((log->trace_status << DLT_MSIN_MTIN_SHIFT) & DLT_MSIN_MTIN);
            break;
        }
        default:
        {
            /* This case should not occur */
            return DLT_RETURN_ERROR;
            break;
        }
        }

        /* If in verbose mode, set flag in header for verbose mode */
        if (dlt_user.verbose_mode)
            msg.extendedheader->msin |= DLT_MSIN_VERB;

        msg.extendedheader->noar = log->args_num;              /* number of arguments */
        dlt_set_id(msg.extendedheader->apid, dlt_user.appID);       /* application id */
        dlt_set_id(msg.extendedheader->ctid, log->handle->contextID);   /* context id */

        msg.headersize = sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + sizeof(DltExtendedHeader) +
            DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp);
    }
    else {
        /* without extended header */
        msg.headersize = sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + DLT_STANDARD_HEADER_EXTRA_SIZE(
            msg.standardheader->htyp);
    }

    len = msg.headersize - sizeof(DltStorageHeader) + log->size;

    if (len > UINT16_MAX) {
        dlt_log(LOG_WARNING, "Huge message discarded!\n");
        return DLT_RETURN_ERROR;
    }

    msg.standardheader->len = DLT_HTOBE_16(len);

    /* print to std out, if enabled */
    if ((dlt_user.local_print_mode != DLT_PM_FORCE_OFF) &&
        (dlt_user.local_print_mode != DLT_PM_AUTOMATIC)) {
        if ((dlt_user.enable_local_print) || (dlt_user.local_print_mode == DLT_PM_FORCE_ON))
            if (dlt_user_print_msg(&msg, log) == DLT_RETURN_ERROR)
                return DLT_RETURN_ERROR;
    }

    if (dlt_user.dlt_is_file) {
        /* log to file */
        ret = dlt_user_log_out2(dlt_user.dlt_log_handle, msg.headerbuffer, msg.headersize, log->buffer, log->size);
        return ret;
    }
    else {
        if (dlt_user.overflow_counter) {
            if (dlt_user_log_send_overflow() == DLT_RETURN_OK) {
                dlt_vnlog(LOG_WARNING, DLT_USER_BUFFER_LENGTH, "%u messages discarded!\n", dlt_user.overflow_counter);
                dlt_user.overflow_counter = 0;
            }
        }

        /* try to resent old data first */
        ret = DLT_RETURN_OK;

        if ((dlt_user.dlt_log_handle != -1) && (dlt_user.appID[0] != '\0'))
            ret = dlt_user_log_resend_buffer();

        if ((ret == DLT_RETURN_OK) && (dlt_user.appID[0] != '\0')) {
            /* resend ok or nothing to resent */
#ifdef DLT_SHM_ENABLE

            if (dlt_user.dlt_log_handle != -1)
                dlt_shm_push(&dlt_user.dlt_shm, msg.headerbuffer + sizeof(DltStorageHeader),
                             msg.headersize - sizeof(DltStorageHeader),
                             log->buffer, log->size, 0, 0);

            ret = dlt_user_log_out3(dlt_user.dlt_log_handle,
                                    &(userheader), sizeof(DltUserHeader),
                                    0, 0,
                                    0, 0);
#else
#   ifdef DLT_TEST_ENABLE

            if (dlt_user.corrupt_user_header) {
                userheader.pattern[0] = 0xff;
                userheader.pattern[1] = 0xff;
                userheader.pattern[2] = 0xff;
                userheader.pattern[3] = 0xff;
            }

            if (dlt_user.corrupt_message_size)
                msg.standardheader->len = DLT_HTOBE_16(dlt_user.corrupt_message_size_size);

#   endif

            ret = dlt_user_log_out3(dlt_user.dlt_log_handle,
                                    &(userheader), sizeof(DltUserHeader),
                                    msg.headerbuffer + sizeof(DltStorageHeader),
                                    msg.headersize - sizeof(DltStorageHeader),
                                    log->buffer, log->size);
#endif
        }

        /* store message in ringbuffer, if an error has occured */
        if ((ret != DLT_RETURN_OK) || (dlt_user.appID[0] == '\0'))
            ret = dlt_user_log_out_error_handling(&(userheader),
                                                  sizeof(DltUserHeader),
                                                  msg.headerbuffer + sizeof(DltStorageHeader),
                                                  msg.headersize - sizeof(DltStorageHeader),
                                                  log->buffer,
                                                  log->size);

        switch (ret) {
        case DLT_RETURN_BUFFER_FULL:
        {
            /* Buffer full */
            dlt_user.overflow_counter += 1;
            return DLT_RETURN_BUFFER_FULL;
        }
        case DLT_RETURN_PIPE_FULL:
        {
            /* data could not be written */
            return DLT_RETURN_PIPE_FULL;
        }
        case DLT_RETURN_PIPE_ERROR:
        {
            /* handle not open or pipe error */
            close(dlt_user.dlt_log_handle);
            dlt_user.dlt_log_handle = -1;
#ifdef DLT_USE_UNIX_SOCKET_IPC
            dlt_user.connection_state = DLT_USER_RETRY_CONNECT;
#endif

    #ifdef DLT_SHM_ENABLE
            /* free shared memory */
            dlt_shm_free_client(&dlt_user.dlt_shm);
    #endif

            if (dlt_user.local_print_mode == DLT_PM_AUTOMATIC)
                dlt_user_print_msg(&msg, log);

            return DLT_RETURN_PIPE_ERROR;
        }
        case DLT_RETURN_ERROR:
        {
            /* other error condition */
            return DLT_RETURN_ERROR;
        }
        case DLT_RETURN_OK:
        {
            return DLT_RETURN_OK;
        }
        default:
        {
            /* This case should never occur. */
            return DLT_RETURN_ERROR;
        }
        }
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_send_register_application(void)
{
    DltUserHeader userheader;
    DltUserControlMsgRegisterApplication usercontext;

    DltReturnValue ret;

    if (dlt_user.appID[0] == '\0')
        return DLT_RETURN_ERROR;

    /* set userheader */
    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_REGISTER_APPLICATION) < DLT_RETURN_OK)
        return DLT_RETURN_ERROR;

    /* set usercontext */
    dlt_set_id(usercontext.apid, dlt_user.appID);       /* application id */
    usercontext.pid = getpid();

    if (dlt_user.application_description != NULL)
        usercontext.description_length = strlen(dlt_user.application_description);
    else
        usercontext.description_length = 0;

    if (dlt_user.dlt_is_file)
        return DLT_RETURN_OK;

    ret = dlt_user_log_out3(dlt_user.dlt_log_handle,
                            &(userheader), sizeof(DltUserHeader),
                            &(usercontext), sizeof(DltUserControlMsgRegisterApplication),
                            dlt_user.application_description, usercontext.description_length);

    /* store message in ringbuffer, if an error has occured */
    if (ret < DLT_RETURN_OK)
        return dlt_user_log_out_error_handling(&(userheader),
                                               sizeof(DltUserHeader),
                                               &(usercontext),
                                               sizeof(DltUserControlMsgRegisterApplication),
                                               dlt_user.application_description,
                                               usercontext.description_length);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_send_unregister_application(void)
{
    DltUserHeader userheader;
    DltUserControlMsgUnregisterApplication usercontext;
    DltReturnValue ret = DLT_RETURN_OK;

    if (dlt_user.appID[0] == '\0')
        return DLT_RETURN_ERROR;

    /* set userheader */
    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_UNREGISTER_APPLICATION) < DLT_RETURN_OK)
        return DLT_RETURN_ERROR;

    /* set usercontext */
    dlt_set_id(usercontext.apid, dlt_user.appID);       /* application id */
    usercontext.pid = getpid();

    if (dlt_user.dlt_is_file)
        return DLT_RETURN_OK;

    ret = dlt_user_log_out2(dlt_user.dlt_log_handle,
                            &(userheader), sizeof(DltUserHeader),
                            &(usercontext), sizeof(DltUserControlMsgUnregisterApplication));

    /* store message in ringbuffer, if an error has occured */
    if (ret < DLT_RETURN_OK)
        return dlt_user_log_out_error_handling(&(userheader),
                                               sizeof(DltUserHeader),
                                               &(usercontext),
                                               sizeof(DltUserControlMsgUnregisterApplication),
                                               NULL,
                                               0);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_send_register_context(DltContextData *log)
{
    DltUserHeader userheader;
    DltUserControlMsgRegisterContext usercontext;
    DltReturnValue ret = DLT_RETURN_ERROR;

    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (log->handle == NULL)
        return DLT_RETURN_ERROR;

    if (log->handle->contextID[0] == '\0')
        return DLT_RETURN_ERROR;

    /* set userheader */
    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_REGISTER_CONTEXT) < DLT_RETURN_OK)
        return DLT_RETURN_ERROR;

    /* set usercontext */
    dlt_set_id(usercontext.apid, dlt_user.appID);       /* application id */
    dlt_set_id(usercontext.ctid, log->handle->contextID);       /* context id */
    usercontext.log_level_pos = log->handle->log_level_pos;
    usercontext.pid = getpid();

    usercontext.log_level = (int8_t)log->log_level;
    usercontext.trace_status = (int8_t)log->trace_status;

    if (log->context_description != NULL)
        usercontext.description_length = strlen(log->context_description);
    else
        usercontext.description_length = 0;

    if (dlt_user.dlt_is_file)
        return DLT_RETURN_OK;

    if (dlt_user.appID[0] != '\0')
        ret =
            dlt_user_log_out3(dlt_user.dlt_log_handle,
                              &(userheader),
                              sizeof(DltUserHeader),
                              &(usercontext),
                              sizeof(DltUserControlMsgRegisterContext),
                              log->context_description,
                              usercontext.description_length);

    /* store message in ringbuffer, if an error has occured */
    if ((ret != DLT_RETURN_OK) || (dlt_user.appID[0] == '\0'))
        return dlt_user_log_out_error_handling(&(userheader),
                                               sizeof(DltUserHeader),
                                               &(usercontext),
                                               sizeof(DltUserControlMsgRegisterContext),
                                               log->context_description,
                                               usercontext.description_length);

    return DLT_RETURN_OK;

}

DltReturnValue dlt_user_log_send_unregister_context(DltContextData *log)
{
    DltUserHeader userheader;
    DltUserControlMsgUnregisterContext usercontext;
    DltReturnValue ret;

    if (log == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (log->handle == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (log->handle->contextID[0] == '\0')
        return DLT_RETURN_ERROR;

    /* set userheader */
    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_UNREGISTER_CONTEXT) < DLT_RETURN_OK)
        return DLT_RETURN_ERROR;

    /* set usercontext */
    dlt_set_id(usercontext.apid, dlt_user.appID);       /* application id */
    dlt_set_id(usercontext.ctid, log->handle->contextID);       /* context id */
    usercontext.pid = getpid();

    if (dlt_user.dlt_is_file)
        return DLT_RETURN_OK;

    ret = dlt_user_log_out2(dlt_user.dlt_log_handle,
                            &(userheader),
                            sizeof(DltUserHeader),
                            &(usercontext),
                            sizeof(DltUserControlMsgUnregisterContext));

    /* store message in ringbuffer, if an error has occured */
    if (ret < DLT_RETURN_OK)
        return dlt_user_log_out_error_handling(&(userheader),
                                               sizeof(DltUserHeader),
                                               &(usercontext),
                                               sizeof(DltUserControlMsgUnregisterContext),
                                               NULL,
                                               0);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_send_app_ll_ts_limit(const char *apid, DltLogLevelType loglevel, DltTraceStatusType tracestatus)
{
    DltUserHeader userheader;
    DltUserControlMsgAppLogLevelTraceStatus usercontext;
    DltReturnValue ret;

    if ((loglevel < DLT_USER_LOG_LEVEL_NOT_SET) || (loglevel >= DLT_LOG_MAX)) {
        dlt_vlog(LOG_ERR, "Loglevel %d is outside valid range", loglevel);
        return DLT_RETURN_ERROR;
    }

    if ((tracestatus < DLT_USER_TRACE_STATUS_NOT_SET) || (tracestatus >= DLT_TRACE_STATUS_MAX)) {
        dlt_vlog(LOG_ERR, "Tracestatus %d is outside valid range", tracestatus);
        return DLT_RETURN_ERROR;
    }

    if ((apid == NULL) || (apid[0] == '\0'))
        return DLT_RETURN_ERROR;

    /* set userheader */
    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_APP_LL_TS) < DLT_RETURN_OK)
        return DLT_RETURN_ERROR;

    /* set usercontext */
    dlt_set_id(usercontext.apid, apid);       /* application id */
    usercontext.log_level = loglevel;
    usercontext.trace_status = tracestatus;

    if (dlt_user.dlt_is_file)
        return DLT_RETURN_OK;

    ret = dlt_user_log_out2(dlt_user.dlt_log_handle,
                            &(userheader), sizeof(DltUserHeader),
                            &(usercontext), sizeof(DltUserControlMsgAppLogLevelTraceStatus));

    /* store message in ringbuffer, if an error has occured */
    if (ret < DLT_RETURN_OK)
        return dlt_user_log_out_error_handling(&(userheader),
                                               sizeof(DltUserHeader),
                                               &(usercontext),
                                               sizeof(DltUserControlMsgAppLogLevelTraceStatus),
                                               NULL,
                                               0);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_send_log_mode(DltUserLogMode mode)
{
    DltUserHeader userheader;
    DltUserControlMsgLogMode logmode;
    DltReturnValue ret;

    if ((mode < DLT_USER_MODE_UNDEFINED) || (mode >= DLT_USER_MODE_MAX)) {
        dlt_vlog(LOG_ERR, "User log mode %d is outside valid range", mode);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* set userheader */
    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_LOG_MODE) < DLT_RETURN_OK)
        return DLT_RETURN_ERROR;

    /* set data */
    logmode.log_mode = (unsigned char)mode;

    if (dlt_user.dlt_is_file)
        return DLT_RETURN_OK;

    ret = dlt_user_log_out2(dlt_user.dlt_log_handle,
                            &(userheader), sizeof(DltUserHeader),
                            &(logmode), sizeof(DltUserControlMsgLogMode));

    /* store message in ringbuffer, if an error has occured */
    if (ret < DLT_RETURN_OK)
        return dlt_user_log_out_error_handling(&(userheader),
                                               sizeof(DltUserHeader),
                                               &(logmode),
                                               sizeof(DltUserControlMsgLogMode),
                                               NULL,
                                               0);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_send_marker()
{
    DltUserHeader userheader;
    DltReturnValue ret;

    /* set userheader */
    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_MARKER) < DLT_RETURN_OK)
        return DLT_RETURN_ERROR;

    if (dlt_user.dlt_is_file)
        return DLT_RETURN_OK;

    /* log to FIFO */
    ret = dlt_user_log_out2(dlt_user.dlt_log_handle,
                            &(userheader), sizeof(DltUserHeader), 0, 0);

    /* store message in ringbuffer, if an error has occured */
    if (ret < DLT_RETURN_OK)
        return dlt_user_log_out_error_handling(&(userheader),
                                               sizeof(DltUserHeader),
                                               NULL,
                                               0,
                                               NULL,
                                               0);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_print_msg(DltMessage *msg, DltContextData *log)
{
    uint8_t *databuffer_tmp;
    int32_t datasize_tmp;
    int32_t databuffersize_tmp;
    static char text[DLT_USER_TEXT_LENGTH];

    if ((msg == NULL) || (log == NULL))
        return DLT_RETURN_WRONG_PARAMETER;

    /* Save variables before print */
    databuffer_tmp = msg->databuffer;
    datasize_tmp = msg->datasize;
    databuffersize_tmp = msg->databuffersize;

    /* Act like a receiver, convert header back to host format */
    msg->standardheader->len = DLT_BETOH_16(msg->standardheader->len);
    dlt_message_get_extraparameters(msg, 0);

    msg->databuffer = log->buffer;
    msg->datasize = log->size;
    msg->databuffersize = log->size;

    /* Print message as ASCII */
    if (dlt_message_print_ascii(msg, text, DLT_USER_TEXT_LENGTH, 0) == DLT_RETURN_ERROR)
        return DLT_RETURN_ERROR;

    /* Restore variables and set len to BE*/
    msg->databuffer = databuffer_tmp;
    msg->databuffersize = databuffersize_tmp;
    msg->datasize = datasize_tmp;

    msg->standardheader->len = DLT_HTOBE_16(msg->standardheader->len);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_check_user_message(void)
{
    int offset = 0;
    int leave_while = 0;
    int ret = 0;

    uint32_t i;
    int fd;
    DltReceiverType from_src_type = DLT_RECEIVE_FD;
    struct pollfd nfd[1];

    DltUserHeader *userheader;
    DltReceiver *receiver = &(dlt_user.receiver);

    DltUserControlMsgLogLevel *usercontextll;
    DltUserControlMsgInjection *usercontextinj;
    DltUserControlMsgLogState *userlogstate;
    unsigned char *userbuffer;

    /* For delayed calling of injection callback, to avoid deadlock */
    DltUserInjectionCallback delayed_injection_callback;
    DltUserLogLevelChangedCallback delayed_log_level_changed_callback;
    unsigned char *delayed_inject_buffer = 0;
    uint32_t delayed_inject_data_length = 0;

    /* Ensure that callback is null before searching for it */
    delayed_injection_callback.injection_callback = 0;
    delayed_injection_callback.injection_callback_with_id = 0;
    delayed_injection_callback.service_id = 0;
    delayed_log_level_changed_callback.log_level_changed_callback = 0;
    delayed_injection_callback.data = 0;

#ifdef DLT_USE_UNIX_SOCKET_IPC
    fd = dlt_user.dlt_log_handle;
    from_src_type = DLT_RECEIVE_SOCKET;
#else
    fd = dlt_user.dlt_user_handle;
    from_src_type = DLT_RECEIVE_FD;
#endif
    nfd[0].events = POLLIN;
    nfd[0].fd = fd;

#ifdef DLT_USE_UNIX_SOCKET_IPC
    if (fd != DLT_FD_INIT) {
        ret = poll(nfd, 1, -1);
#else
    if (fd != DLT_FD_INIT && dlt_user.dlt_log_handle > 0) {
        ret = poll(nfd, 1, DLT_USER_RECEIVE_NDELAY);
#endif
        if (ret) {
            if (nfd[0].revents & (POLLHUP | POLLNVAL | POLLERR)) {
                dlt_user.dlt_log_handle = DLT_FD_INIT;
                return DLT_RETURN_ERROR;
            }

            if (dlt_receiver_receive(receiver, from_src_type) <= 0)
                /* No new message available */
                return DLT_RETURN_OK;

            /* look through buffer as long as data is in there */
            while (1) {
                if (receiver->bytesRcvd < (int32_t)sizeof(DltUserHeader))
                    break;

                /* resync if necessary */
                offset = 0;

                do {
                    userheader = (DltUserHeader *)(receiver->buf + offset);

                    /* Check for user header pattern */
                    if (dlt_user_check_userheader(userheader))
                        break;

                    offset++;

                } while ((int32_t)(sizeof(DltUserHeader) + offset) <= receiver->bytesRcvd);

                /* Check for user header pattern */
                if ((dlt_user_check_userheader(userheader) < 0) ||
                    (dlt_user_check_userheader(userheader) == 0))
                    break;

                /* Set new start offset */
                if (offset > 0) {
                    receiver->buf += offset;
                    receiver->bytesRcvd -= offset;
                }

                switch (userheader->message) {
                case DLT_USER_MESSAGE_LOG_LEVEL:
                {
                    if (receiver->bytesRcvd < (int32_t)(sizeof(DltUserHeader) + sizeof(DltUserControlMsgLogLevel))) {
                        leave_while = 1;
                        break;
                    }

                    usercontextll = (DltUserControlMsgLogLevel *)(receiver->buf + sizeof(DltUserHeader));

                    /* Update log level and trace status */
                    if (usercontextll != NULL) {
                        DLT_SEM_LOCK();

                        if ((usercontextll->log_level_pos >= 0) &&
                            (usercontextll->log_level_pos < (int32_t)dlt_user.dlt_ll_ts_num_entries)) {
                            if (dlt_user.dlt_ll_ts) {
                                dlt_user.dlt_ll_ts[usercontextll->log_level_pos].log_level = usercontextll->log_level;
                                dlt_user.dlt_ll_ts[usercontextll->log_level_pos].trace_status =
                                    usercontextll->trace_status;

                                if (dlt_user.dlt_ll_ts[usercontextll->log_level_pos].log_level_ptr)
                                    *(dlt_user.dlt_ll_ts[usercontextll->log_level_pos].log_level_ptr) =
                                        usercontextll->log_level;

                                if (dlt_user.dlt_ll_ts[usercontextll->log_level_pos].trace_status_ptr)
                                    *(dlt_user.dlt_ll_ts[usercontextll->log_level_pos].trace_status_ptr) =
                                        usercontextll->trace_status;

                                delayed_log_level_changed_callback.log_level_changed_callback =
                                    dlt_user.dlt_ll_ts[usercontextll->log_level_pos].log_level_changed_callback;
                                memcpy(delayed_log_level_changed_callback.contextID,
                                       dlt_user.dlt_ll_ts[usercontextll->log_level_pos].contextID, DLT_ID_SIZE);
                                delayed_log_level_changed_callback.log_level = usercontextll->log_level;
                                delayed_log_level_changed_callback.trace_status = usercontextll->trace_status;
                            }
                        }

                        DLT_SEM_FREE();
                    }

                    /* call callback outside of semaphore */
                    if (delayed_log_level_changed_callback.log_level_changed_callback != 0)
                        delayed_log_level_changed_callback.log_level_changed_callback(
                            delayed_log_level_changed_callback.contextID,
                            delayed_log_level_changed_callback.log_level,
                            delayed_log_level_changed_callback.trace_status);

                    /* keep not read data in buffer */
                    if (dlt_receiver_remove(receiver,
                                            sizeof(DltUserHeader) + sizeof(DltUserControlMsgLogLevel)) ==
                        DLT_RETURN_ERROR)
                        return DLT_RETURN_ERROR;
                }
                break;
                case DLT_USER_MESSAGE_INJECTION:
                {
                    /* At least, user header, user context, and service id and data_length of injected message is available */
                    if (receiver->bytesRcvd < (int32_t)(sizeof(DltUserHeader) + sizeof(DltUserControlMsgInjection))) {
                        leave_while = 1;
                        break;
                    }

                    usercontextinj = (DltUserControlMsgInjection *)(receiver->buf + sizeof(DltUserHeader));
                    userbuffer =
                        (unsigned char *)(receiver->buf + sizeof(DltUserHeader) + sizeof(DltUserControlMsgInjection));

                    if (userbuffer != NULL) {

                        if (receiver->bytesRcvd <
                            (int32_t)(sizeof(DltUserHeader) + sizeof(DltUserControlMsgInjection) +
                                      usercontextinj->data_length_inject)) {
                            leave_while = 1;
                            break;
                        }

                        DLT_SEM_LOCK();

                        if ((usercontextinj->data_length_inject > 0) && (dlt_user.dlt_ll_ts))
                            /* Check if injection callback is registered for this context */
                            for (i = 0; i < dlt_user.dlt_ll_ts[usercontextinj->log_level_pos].nrcallbacks; i++)
                                if ((dlt_user.dlt_ll_ts[usercontextinj->log_level_pos].injection_table) &&
                                    (dlt_user.dlt_ll_ts[usercontextinj->log_level_pos].injection_table[i].service_id ==
                                     usercontextinj->service_id)) {
                                    /* Prepare delayed injection callback call */
                                    if (dlt_user.dlt_ll_ts[usercontextinj->log_level_pos].injection_table[i].
                                        injection_callback != NULL) {
                                        delayed_injection_callback.injection_callback =
                                            dlt_user.dlt_ll_ts[usercontextinj->log_level_pos].injection_table[i].
                                            injection_callback;
                                    }
                                    else if (dlt_user.dlt_ll_ts[usercontextinj->log_level_pos].injection_table[i].
                                             injection_callback_with_id != NULL)
                                    {
                                        delayed_injection_callback.injection_callback_with_id =
                                            dlt_user.dlt_ll_ts[usercontextinj->log_level_pos].injection_table[i].
                                            injection_callback_with_id;
                                        delayed_injection_callback.data =
                                            dlt_user.dlt_ll_ts[usercontextinj->log_level_pos].injection_table[i].data;
                                    }

                                    delayed_injection_callback.service_id = usercontextinj->service_id;
                                    delayed_inject_data_length = usercontextinj->data_length_inject;
                                    delayed_inject_buffer = malloc(delayed_inject_data_length);

                                    if (delayed_inject_buffer != NULL) {
                                        memcpy(delayed_inject_buffer, userbuffer, delayed_inject_data_length);
                                    }
                                    else {
                                        DLT_SEM_FREE();
                                        dlt_log(LOG_WARNING, "malloc failed!\n");
                                        return DLT_RETURN_ERROR;
                                    }

                                    break;
                                }

                        DLT_SEM_FREE();

                        /* Delayed injection callback call */
                        if ((delayed_inject_buffer != NULL) &&
                            (delayed_injection_callback.injection_callback != NULL)) {
                            delayed_injection_callback.injection_callback(delayed_injection_callback.service_id,
                                                                          delayed_inject_buffer,
                                                                          delayed_inject_data_length);
                            delayed_injection_callback.injection_callback = NULL;
                        }
                        else if ((delayed_inject_buffer != NULL) &&
                                 (delayed_injection_callback.injection_callback_with_id != NULL))
                        {
                            delayed_injection_callback.injection_callback_with_id(delayed_injection_callback.service_id,
                                                                                  delayed_inject_buffer,
                                                                                  delayed_inject_data_length,
                                                                                  delayed_injection_callback.data);
                            delayed_injection_callback.injection_callback_with_id = NULL;
                        }

                        free(delayed_inject_buffer);
                        delayed_inject_buffer = NULL;

                        /* keep not read data in buffer */
                        if (dlt_receiver_remove(receiver,
                                                (sizeof(DltUserHeader) +
                                                 sizeof(DltUserControlMsgInjection) +
                                                 usercontextinj->data_length_inject)) != DLT_RETURN_OK)
                            return DLT_RETURN_ERROR;
                    }
                }
                break;
                case DLT_USER_MESSAGE_LOG_STATE:
                {
                    /* At least, user header, user context, and service id and data_length of injected message is available */
                    if (receiver->bytesRcvd < (int32_t)(sizeof(DltUserHeader) + sizeof(DltUserControlMsgLogState))) {
                        leave_while = 1;
                        break;
                    }

                    userlogstate = (DltUserControlMsgLogState *)(receiver->buf + sizeof(DltUserHeader));
                    dlt_user.log_state = userlogstate->log_state;

                    /* keep not read data in buffer */
                    if (dlt_receiver_remove(receiver,
                                            (sizeof(DltUserHeader) + sizeof(DltUserControlMsgLogState))) ==
                        DLT_RETURN_ERROR)
                        return DLT_RETURN_ERROR;
                }
                break;
                default:
                {
                    dlt_log(LOG_WARNING, "Invalid user message type received!\n");
                    /* Ignore result */
                    dlt_receiver_remove(receiver, sizeof(DltUserHeader));
                    /* In next invocation of while loop, a resync will be triggered if additional data was received */
                }
                break;
                } /* switch() */

                if (leave_while == 1) {
                    leave_while = 0;
                    break;
                }
            } /* while buffer*/

            if (dlt_receiver_move_to_begin(receiver) == DLT_RETURN_ERROR)
                return DLT_RETURN_ERROR;
        } /* while receive */

    } /* if */

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_resend_buffer(void)
{
    int num, count;
    int size;
    DltReturnValue ret;

    DLT_SEM_LOCK();

    if (dlt_user.appID[0] == '\0') {
        DLT_SEM_FREE();
        return 0;
    }

    /* Send content of ringbuffer */
    count = dlt_buffer_get_message_count(&(dlt_user.startup_buffer));
    DLT_SEM_FREE();

    for (num = 0; num < count; num++) {

        DLT_SEM_LOCK();
        size = dlt_buffer_copy(&(dlt_user.startup_buffer), dlt_user.resend_buffer, dlt_user.log_buf_len);

        if (size > 0) {
            DltUserHeader *userheader = (DltUserHeader *)(dlt_user.resend_buffer);

            /* Add application id to the messages of needed*/
            if (dlt_user_check_userheader(userheader)) {
                switch (userheader->message) {
                case DLT_USER_MESSAGE_REGISTER_CONTEXT:
                {
                    DltUserControlMsgRegisterContext *usercontext =
                        (DltUserControlMsgRegisterContext *)(dlt_user.resend_buffer + sizeof(DltUserHeader));

                    if ((usercontext != 0) && (usercontext->apid[0] == '\0'))
                        dlt_set_id(usercontext->apid, dlt_user.appID);

                    break;
                }
                case DLT_USER_MESSAGE_LOG:
                {
                    DltExtendedHeader *extendedHeader =
                        (DltExtendedHeader *)(dlt_user.resend_buffer + sizeof(DltUserHeader) +
                                              sizeof(DltStandardHeader) +
                                              sizeof(DltStandardHeaderExtra));

                    if (((extendedHeader) != 0) && (extendedHeader->apid[0] == '\0')) /* if application id is empty, add it */
                        dlt_set_id(extendedHeader->apid, dlt_user.appID);

                    break;
                }
                default:
                {
                    break;
                }
                }
            }

#ifdef DLT_SHM_ENABLE
            dlt_shm_push(&dlt_user.dlt_shm,
                         dlt_user.resend_buffer + sizeof(DltUserHeader),
                         size - sizeof(DltUserHeader),
                         0,
                         0,
                         0,
                         0);

            ret = dlt_user_log_out3(dlt_user.dlt_log_handle, dlt_user.resend_buffer, sizeof(DltUserHeader), 0, 0, 0, 0);
#else
            ret = dlt_user_log_out3(dlt_user.dlt_log_handle, dlt_user.resend_buffer, size, 0, 0, 0, 0);
#endif

            /* in case of error, keep message in ringbuffer */
            if (ret == DLT_RETURN_OK) {
                dlt_buffer_remove(&(dlt_user.startup_buffer));
            }
            else {
                if (ret == DLT_RETURN_PIPE_ERROR) {
                    /* handle not open or pipe error */
                    close(dlt_user.dlt_log_handle);
                    dlt_user.dlt_log_handle = -1;
                }

                /* keep message in ringbuffer */
                DLT_SEM_FREE();
                return ret;
            }
        }

        DLT_SEM_FREE();
    }

    return DLT_RETURN_OK;
}

void dlt_user_log_reattach_to_daemon(void)
{
    uint32_t num;
    DltContext handle;
    DltContextData log_new;

    if (dlt_user.dlt_log_handle < 0) {
        dlt_user.dlt_log_handle = DLT_FD_INIT;

#ifdef DLT_USE_UNIX_SOCKET_IPC
        /* try to open connection to dlt daemon */
        dlt_initialize_socket_connection();

        if (dlt_user.connection_state != DLT_USER_CONNECTED)
            /* return if not connected */
            return;

#else
        /* try to open pipe to dlt daemon */
        int fd = open(dlt_daemon_fifo, O_WRONLY | O_NONBLOCK);

        if (fd < 0)
            return;

        dlt_user.dlt_log_handle = fd;
#endif

        if (dlt_user_log_init(&handle, &log_new) < DLT_RETURN_OK)
            return;

#ifdef DLT_SHM_ENABLE

        /* init shared memory */
        if (dlt_shm_init_client(&dlt_user.dlt_shm, dltShmName) < DLT_RETURN_OK)
            dlt_vnlog(LOG_WARNING, DLT_USER_BUFFER_LENGTH, "Logging disabled,"
                      " Shared memory %s cannot be created!\n", dltShmName);

#endif

        dlt_log(LOG_NOTICE, "Logging (re-)enabled!\n");

        /* Re-register application */
        if (dlt_user_log_send_register_application() < DLT_RETURN_ERROR)
            return;

        DLT_SEM_LOCK();

        /* Re-register all stored contexts */
        for (num = 0; num < dlt_user.dlt_ll_ts_num_entries; num++)
            /* Re-register stored context */
            if ((dlt_user.appID[0] != '\0') && (dlt_user.dlt_ll_ts) && (dlt_user.dlt_ll_ts[num].contextID[0] != '\0')) {
                /*dlt_set_id(log_new.appID, dlt_user.appID); */
                dlt_set_id(handle.contextID, dlt_user.dlt_ll_ts[num].contextID);
                handle.log_level_pos = num;
                log_new.context_description = dlt_user.dlt_ll_ts[num].context_description;

                /* Release the mutex for sending context registration: */
                /* function  dlt_user_log_send_register_context() can take the mutex to write to the DLT buffer. => dead lock */
                DLT_SEM_FREE();

                log_new.log_level = DLT_USER_LOG_LEVEL_NOT_SET;
                log_new.trace_status = DLT_USER_TRACE_STATUS_NOT_SET;

                if (dlt_user_log_send_register_context(&log_new) < DLT_RETURN_ERROR)
                    return;

                /* Lock again the mutex */
                /* it is necessary in the for(;;) test, in order to have coherent dlt_user data all over the critical section. */
                DLT_SEM_LOCK();
            }
        DLT_SEM_FREE();
    }
}

DltReturnValue dlt_user_log_send_overflow(void)
{
    DltUserHeader userheader;
    DltUserControlMsgBufferOverflow userpayload;

    /* set userheader */
    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_OVERFLOW) < DLT_RETURN_OK)
        return DLT_RETURN_ERROR;

    if (dlt_user.dlt_is_file)
        return DLT_RETURN_OK;

    /* set user message parameters */
    userpayload.overflow_counter = dlt_user.overflow_counter;
    dlt_set_id(userpayload.apid, dlt_user.appID);

    return dlt_user_log_out2(dlt_user.dlt_log_handle,
                             &(userheader), sizeof(DltUserHeader),
                             &(userpayload), sizeof(DltUserControlMsgBufferOverflow));
}

DltReturnValue dlt_user_check_buffer(int *total_size, int *used_size)
{
    if ((total_size == NULL) || (used_size == NULL))
        return DLT_RETURN_WRONG_PARAMETER;

    DLT_SEM_LOCK();

#ifdef DLT_SHM_ENABLE
    *total_size = dlt_shm_get_total_size(&(dlt_user.dlt_shm));
    *used_size = dlt_shm_get_used_size(&(dlt_user.dlt_shm));
#else
    *total_size = dlt_buffer_get_total_size(&(dlt_user.startup_buffer));
    *used_size = dlt_buffer_get_used_size(&(dlt_user.startup_buffer));
#endif

    DLT_SEM_FREE();
    return DLT_RETURN_OK; /* ok */
}

#ifdef DLT_TEST_ENABLE
void dlt_user_test_corrupt_user_header(int enable)
{
    dlt_user.corrupt_user_header = enable;
}
void dlt_user_test_corrupt_message_size(int enable, int16_t size)
{
    dlt_user.corrupt_message_size = enable;
    dlt_user.corrupt_message_size_size = size;
}
#endif


int dlt_start_threads()
{
    /* Start housekeeper thread */
    if (pthread_create(&(dlt_housekeeperthread_handle),
                       0,
                       (void *)&dlt_user_housekeeperthread_function,
                       0) != 0) {
        dlt_log(LOG_CRIT, "Can't create housekeeper thread!\n");
        return -1;
    }

#ifdef DLT_NETWORK_TRACE_ENABLE
    /* Start the segmented thread */
    if (pthread_create(&(dlt_user.dlt_segmented_nwt_handle), NULL,
                       (void *)dlt_user_trace_network_segmented_thread, NULL)) {
        dlt_log(LOG_CRIT, "Can't start segmented thread!\n");
        return -1;
    }
#endif
    return 0;
}

void dlt_stop_threads()
{
    int dlt_housekeeperthread_result = 0;
    int joined = 0;

    if (dlt_housekeeperthread_handle) {
        /* do not ignore return value */
#ifndef __ANDROID_API__
        dlt_housekeeperthread_result = pthread_cancel(dlt_housekeeperthread_handle);
#else

#ifdef DLT_NETWORK_TRACE_ENABLE
        dlt_lock_mutex(&mq_mutex);
#endif /* DLT_NETWORK_TRACE_ENABLE */
        dlt_housekeeperthread_result = pthread_kill(dlt_housekeeperthread_handle, SIGKILL);
        dlt_user_cleanup_handler(NULL);
#endif


        if (dlt_housekeeperthread_result != 0)
            dlt_vlog(LOG_ERR,
                     "ERROR %s(dlt_housekeeperthread_handle): %s\n",
#ifndef __ANDROID_API__
                     "pthread_cancel",
#else
                     "pthread_kill",
#endif
                     strerror(dlt_housekeeperthread_result));
    }

#ifdef DLT_NETWORK_TRACE_ENABLE
    int dlt_segmented_nwt_result = 0;

    if (dlt_user.dlt_segmented_nwt_handle) {
        dlt_lock_mutex(&mq_mutex);
        pthread_cond_signal(&mq_init_condition);
        dlt_unlock_mutex(&mq_mutex);

        dlt_segmented_nwt_result = pthread_cancel(dlt_user.dlt_segmented_nwt_handle);

        if (dlt_segmented_nwt_result != 0)
            dlt_vlog(LOG_ERR,
                     "ERROR pthread_cancel(dlt_user.dlt_segmented_nwt_handle): %s\n",
                     strerror(dlt_segmented_nwt_result));
    }
#endif /* DLT_NETWORK_TRACE_ENABLE */
    /* make sure that the threads really finished working */
    if ((dlt_housekeeperthread_result == 0) && dlt_housekeeperthread_handle) {
        joined = pthread_join(dlt_housekeeperthread_handle, NULL);

        if (joined != 0)
            dlt_vlog(LOG_ERR,
                     "ERROR pthread_join(dlt_housekeeperthread_handle, NULL): %s\n",
                     strerror(joined));

        dlt_housekeeperthread_handle = 0; /* set to invalid */
    }

#ifdef DLT_NETWORK_TRACE_ENABLE
    if ((dlt_segmented_nwt_result == 0) && dlt_user.dlt_segmented_nwt_handle) {
        joined = pthread_join(dlt_user.dlt_segmented_nwt_handle, NULL);

        if (joined != 0)
            dlt_vlog(LOG_ERR,
                     "ERROR pthread_join(dlt_user.dlt_segmented_nwt_handle, NULL): %s\n",
                     strerror(joined));

        dlt_user.dlt_segmented_nwt_handle = 0; /* set to invalid */
    }
#endif /* DLT_NETWORK_TRACE_ENABLE */
}

static void dlt_fork_child_fork_handler()
{
    g_dlt_is_child = 1;
    dlt_user_initialised = false;
    dlt_user.dlt_log_handle = -1;
}

DltReturnValue dlt_user_log_out_error_handling(void *ptr1, size_t len1, void *ptr2, size_t len2, void *ptr3,
                                               size_t len3)
{
    int ret = DLT_RETURN_ERROR;
    int msg_size = len1 + len2 + len3;

    DLT_SEM_LOCK();
    ret = dlt_buffer_check_size(&(dlt_user.startup_buffer), msg_size);
    DLT_SEM_FREE();

    DLT_SEM_LOCK();

    if (dlt_buffer_push3(&(dlt_user.startup_buffer),
                         ptr1, len1,
                         ptr2, len2,
                         ptr3, len3) == DLT_RETURN_ERROR) {
        if (dlt_user.overflow_counter == 0)
            dlt_log(LOG_WARNING, "Buffer full! Messages will be discarded.\n");

        ret = DLT_RETURN_BUFFER_FULL;
    }

    DLT_SEM_FREE();

    return ret;
}
