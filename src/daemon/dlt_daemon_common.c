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
 * \file dlt_daemon_common.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_daemon_common.c                                           **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
**              Markus Klein                                                  **
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
**  aw          Alexander Wenzel           BMW                                **
**  mk          Markus Klein               Fraunhofer ESK                     **
*******************************************************************************/

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/

/*
 * $LastChangedRevision: 1670 $
 * $LastChangedDate: 2011-04-08 15:12:06 +0200 (Fr, 08. Apr 2011) $
 * $LastChangedBy$
 * Initials    Date         Comment
 * aw          13.01.2010   initial
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/socket.h> /* send() */

#include "dlt_types.h"
#include "dlt_daemon_common.h"
#include "dlt_daemon_common_cfg.h"
#include "dlt_user_shared.h"
#include "dlt_user_shared_cfg.h"
#include "dlt-daemon.h"

#include "dlt_daemon_socket.h"
#include "dlt_daemon_serial.h"

char *app_recv_buffer = NULL; /* pointer to receiver buffer for application msges */

static int dlt_daemon_cmp_apid(const void *m1, const void *m2)
{
    if ((m1 == NULL) || (m2 == NULL))
        return -1;

    DltDaemonApplication *mi1 = (DltDaemonApplication *)m1;
    DltDaemonApplication *mi2 = (DltDaemonApplication *)m2;

    return memcmp(mi1->apid, mi2->apid, DLT_ID_SIZE);
}

static int dlt_daemon_cmp_apid_ctid(const void *m1, const void *m2)
{
    if ((m1 == NULL) || (m2 == NULL))
        return -1;

    int ret, cmp;
    DltDaemonContext *mi1 = (DltDaemonContext *)m1;
    DltDaemonContext *mi2 = (DltDaemonContext *)m2;

    cmp = memcmp(mi1->apid, mi2->apid, DLT_ID_SIZE);

    if (cmp < 0)
        ret = -1;
    else if (cmp == 0)
        ret = memcmp(mi1->ctid, mi2->ctid, DLT_ID_SIZE);
    else
        ret = 1;

    return ret;
}

DltDaemonRegisteredUsers *dlt_daemon_find_users_list(DltDaemon *daemon,
                                                     char *ecu,
                                                     int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    int i = 0;

    if ((daemon == NULL) || (ecu == NULL)) {
        dlt_vlog(LOG_ERR, "%s: Wrong parameters", __func__);
        return (DltDaemonRegisteredUsers *)NULL;
    }

    for (i = 0; i < daemon->num_user_lists; i++)
        if (strncmp(ecu, daemon->user_list[i].ecu, DLT_ID_SIZE) == 0)
            return &daemon->user_list[i];

    dlt_vlog(LOG_ERR, "Cannot find user list for ECU: %4s\n", ecu);
    return (DltDaemonRegisteredUsers *)NULL;
}

#ifdef DLT_LOG_LEVEL_APP_CONFIG

static int dlt_daemon_cmp_log_settings(const void *lhs, const void *rhs) {
    if ((lhs == NULL) || (rhs == NULL))
        return -1;

    DltDaemonContextLogSettings *settings1 = (DltDaemonContextLogSettings *)lhs;
    DltDaemonContextLogSettings *settings2 = (DltDaemonContextLogSettings *)rhs;

    int cmp = memcmp(settings1->apid, settings2->apid, DLT_ID_SIZE);

    if (cmp < 0)
        return -1;
    else if (cmp == 0)
        return memcmp(settings1->ctid, settings2->ctid, DLT_ID_SIZE);
    else
        return 1;
}

/**
 * Find configuration for app/ctx id specific log settings configuration
 * @param daemon pointer to dlt daemon struct
 * @param apid application id to use
 * @param ctid context id to use, can be NULL
 * @return pointer to log settings if found, otherwise NULL
 */
DltDaemonContextLogSettings *dlt_daemon_find_configured_app_id_ctx_id_settings(
    const DltDaemon *daemon, const char *apid, const char *ctid) {
    DltDaemonContextLogSettings *app_id_settings = NULL;
    for (int i = 0; i < daemon->num_app_id_log_level_settings; ++i) {
        DltDaemonContextLogSettings *settings = &daemon->app_id_log_level_settings[i];

        if (strncmp(apid, settings->apid, DLT_ID_SIZE) != 0) {
            if (app_id_settings != NULL)
                return app_id_settings;
            continue;
        }

        if (strlen(settings->ctid) == 0) {
            app_id_settings = settings;
        }

        if (ctid == NULL || strlen(ctid) == 0) {
            if (app_id_settings != NULL) {
                return app_id_settings;
            }
        } else {
            if (strncmp(ctid, settings->ctid, DLT_ID_SIZE) == 0) {
                return settings;
            }
        }
    }

    return app_id_settings;
}

/**
 * Find configured log levels in a given DltDaemonApplication for the passed context id.
 * @param app The application settings which contain the previously loaded ap id settings
 * @param ctid The context id to find.
 * @return Pointer to DltDaemonApplicationLogSettings containing the log level
 *         for the requested application or NULL if none found.
 */
DltDaemonContextLogSettings *dlt_daemon_find_app_log_level_config(
    const DltDaemonApplication *const app, const char *const ctid) {

    if (NULL == ctid)
        return NULL;

    DltDaemonContextLogSettings settings;
    memcpy(settings.apid, app->apid, DLT_ID_SIZE);
    memcpy(settings.ctid, ctid, DLT_ID_SIZE);

    DltDaemonContextLogSettings* log_settings = NULL;
    log_settings =
        (DltDaemonContextLogSettings *)bsearch(
            &settings, app->context_log_level_settings,
            (size_t)app->num_context_log_level_settings,
            sizeof(DltDaemonContextLogSettings),
            dlt_daemon_cmp_log_settings);
    return log_settings;
}

#endif

int dlt_daemon_init_runtime_configuration(DltDaemon *daemon, const char *runtime_directory, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);
    size_t append_length = 0;

    if (daemon == NULL)
        return DLT_RETURN_ERROR;

    /* Default */
    daemon->mode = DLT_USER_MODE_EXTERNAL;

    if (runtime_directory == NULL)
        return DLT_RETURN_ERROR;

    /* prepare filenames for configuration */
    append_length = PATH_MAX - sizeof(DLT_RUNTIME_APPLICATION_CFG);

    if (runtime_directory[0]) {
        strncpy(daemon->runtime_application_cfg, runtime_directory, append_length);
        daemon->runtime_application_cfg[append_length] = 0;
    }
    else {
        strncpy(daemon->runtime_application_cfg, DLT_RUNTIME_DEFAULT_DIRECTORY, append_length);
        daemon->runtime_application_cfg[append_length] = 0;
    }

    strcat(daemon->runtime_application_cfg, DLT_RUNTIME_APPLICATION_CFG); /* strcat uncritical here, because max length already checked */

    append_length = PATH_MAX - sizeof(DLT_RUNTIME_CONTEXT_CFG);

    if (runtime_directory[0]) {
        strncpy(daemon->runtime_context_cfg, runtime_directory, append_length);
        daemon->runtime_context_cfg[append_length] = 0;
    }
    else {
        strncpy(daemon->runtime_context_cfg, DLT_RUNTIME_DEFAULT_DIRECTORY, append_length);
        daemon->runtime_context_cfg[append_length] = 0;
    }

    strcat(daemon->runtime_context_cfg, DLT_RUNTIME_CONTEXT_CFG); /* strcat uncritical here, because max length already checked */

    append_length = PATH_MAX - sizeof(DLT_RUNTIME_CONFIGURATION);

    if (runtime_directory[0]) {
        strncpy(daemon->runtime_configuration, runtime_directory, append_length);
        daemon->runtime_configuration[append_length] = 0;
    }
    else {
        strncpy(daemon->runtime_configuration, DLT_RUNTIME_DEFAULT_DIRECTORY, append_length);
        daemon->runtime_configuration[append_length] = 0;
    }

    strcat(daemon->runtime_configuration, DLT_RUNTIME_CONFIGURATION); /* strcat uncritical here, because max length already checked */

    return DLT_RETURN_OK;
}

int dlt_daemon_init(DltDaemon *daemon,
                    unsigned long RingbufferMinSize,
                    unsigned long RingbufferMaxSize,
                    unsigned long RingbufferStepSize,
                    const char *runtime_directory,
                    int InitialContextLogLevel,
                    int InitialContextTraceStatus,
                    int ForceLLTS,
                    int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (runtime_directory == NULL))
        return -1;

    daemon->user_list = NULL;
    daemon->num_user_lists = 0;

    daemon->default_log_level = (int8_t) InitialContextLogLevel;
    daemon->default_trace_status = (int8_t) InitialContextTraceStatus;
    daemon->force_ll_ts = (int8_t) ForceLLTS;

    daemon->overflow_counter = 0;

    daemon->runtime_context_cfg_loaded = 0;

    daemon->connectionState = 0; /* no logger connected */

    daemon->state = DLT_DAEMON_STATE_INIT; /* initial logging state */

    daemon->sendserialheader = 0;
    daemon->timingpackets = 0;

    dlt_set_id(daemon->ecuid, "");

    /* initialize ring buffer for client connection */
    dlt_vlog(LOG_INFO, "Ringbuffer configuration: %lu/%lu/%lu\n",
             RingbufferMinSize, RingbufferMaxSize, RingbufferStepSize);

    if (dlt_buffer_init_dynamic(&(daemon->client_ringbuffer),
                                (uint32_t) RingbufferMinSize,
                                (uint32_t) RingbufferMaxSize,
                                (uint32_t) RingbufferStepSize) < DLT_RETURN_OK)
        return -1;

    daemon->storage_handle = NULL;
#ifdef DLT_SYSTEMD_WATCHDOG_ENFORCE_MSG_RX_ENABLE
    daemon->received_message_since_last_watchdog_interval = 0;
#endif
    return 0;
}

int dlt_daemon_free(DltDaemon *daemon, int verbose)
{
    int i = 0;
    DltDaemonRegisteredUsers *user_list = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon->user_list == NULL))
        return -1;

    /* free all registered user information */
    for (i = 0; i < daemon->num_user_lists; i++) {
        user_list = &daemon->user_list[i];

        if (user_list != NULL) {
            /* ignore return values */
            dlt_daemon_contexts_clear(daemon, user_list->ecu, verbose);
            dlt_daemon_applications_clear(daemon, user_list->ecu, verbose);
        }
    }

    free(daemon->user_list);

#ifdef DLT_LOG_LEVEL_APP_CONFIG
    if (daemon->app_id_log_level_settings != NULL) {
      free(daemon->app_id_log_level_settings);
    }
#endif

    if (app_recv_buffer)
        free(app_recv_buffer);

    /* free ringbuffer */
    dlt_buffer_free_dynamic(&(daemon->client_ringbuffer));

    return 0;
}

int dlt_daemon_init_user_information(DltDaemon *daemon,
                                     DltGateway *gateway,
                                     int gateway_mode,
                                     int verbose)
{
    int nodes = 1;
    int i = 1;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || ((gateway_mode == 1) && (gateway == NULL)))
        return DLT_RETURN_ERROR;

    if (gateway_mode == 0) {
        /* initialize application list */
        daemon->user_list = calloc((size_t) nodes, sizeof(DltDaemonRegisteredUsers));

        if (daemon->user_list == NULL) {
            dlt_log(LOG_ERR, "Allocating memory for user information");
            return DLT_RETURN_ERROR;
        }

        dlt_set_id(daemon->user_list[0].ecu, daemon->ecuid);
        daemon->num_user_lists = 1;
    }
    else { /* gateway is active */
        nodes += gateway->num_connections;

        /* initialize application list */
        daemon->user_list = calloc((size_t) nodes, sizeof(DltDaemonRegisteredUsers));

        if (daemon->user_list == NULL) {
            dlt_log(LOG_ERR, "Allocating memory for user information");
            return DLT_RETURN_ERROR;
        }

        dlt_set_id(daemon->user_list[0].ecu, daemon->ecuid);
        daemon->num_user_lists = nodes;

        for (i = 1; i < nodes; i++)
            dlt_set_id(daemon->user_list[i].ecu, gateway->connections[i - 1].ecuid);
    }

    return DLT_RETURN_OK;
}

int dlt_daemon_applications_invalidate_fd(DltDaemon *daemon,
                                          char *ecu,
                                          int fd,
                                          int verbose)
{
    int i;
    DltDaemonRegisteredUsers *user_list = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (ecu == NULL))
        return DLT_RETURN_ERROR;

    user_list = dlt_daemon_find_users_list(daemon, ecu, verbose);

    if (user_list != NULL) {
        for (i = 0; i < user_list->num_applications; i++)
            if (user_list->applications[i].user_handle == fd)
                user_list->applications[i].user_handle = DLT_FD_INIT;

        return DLT_RETURN_OK;
    }

    return DLT_RETURN_ERROR;
}

int dlt_daemon_applications_clear(DltDaemon *daemon, char *ecu, int verbose)
{
    int i;
    DltDaemonRegisteredUsers *user_list = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon->user_list == NULL) || (ecu == NULL))
        return DLT_RETURN_WRONG_PARAMETER;

    user_list = dlt_daemon_find_users_list(daemon, ecu, verbose);

    if (user_list == NULL)
        return DLT_RETURN_ERROR;

    for (i = 0; i < user_list->num_applications; i++)
        if (user_list->applications[i].application_description != NULL) {

#ifdef DLT_LOG_LEVEL_APP_CONFIG
            if (user_list->applications[i].context_log_level_settings)
                free(user_list->applications[i].context_log_level_settings);
#endif

            free(user_list->applications[i].application_description);
            user_list->applications[i].application_description = NULL;
        }

    if (user_list->applications != NULL)
        free(user_list->applications);

    user_list->applications = NULL;
    user_list->num_applications = 0;

    return 0;
}

static void dlt_daemon_application_reset_user_handle(DltDaemon *daemon,
                                                     DltDaemonApplication *application,
                                                     int verbose)
{
    DltDaemonRegisteredUsers *user_list;
    DltDaemonContext *context;
    int i;

    if (application->user_handle == DLT_FD_INIT)
        return;

    user_list = dlt_daemon_find_users_list(daemon, daemon->ecuid, verbose);
    if (user_list != NULL) {
        for (i = 0; i < user_list->num_contexts; i++) {
            context = &user_list->contexts[i];
            if (context->user_handle == application->user_handle)
                context->user_handle = DLT_FD_INIT;
        }
    }

    if (application->owns_user_handle)
        close(application->user_handle);

    application->user_handle = DLT_FD_INIT;
    application->owns_user_handle = false;
}

DltDaemonApplication *dlt_daemon_application_add(DltDaemon *daemon,
                                                 char *apid,
                                                 pid_t pid,
                                                 char *description,
                                                 int fd,
                                                 char *ecu,
                                                 int verbose)
{
    DltDaemonApplication *application;
    DltDaemonApplication *old;
    int new_application;
    int dlt_user_handle;
    bool owns_user_handle;
    DltDaemonRegisteredUsers *user_list = NULL;
#ifdef DLT_DAEMON_USE_FIFO_IPC
    (void)fd;  /* To avoid compiler warning : unused variable */
    char filename[DLT_DAEMON_COMMON_TEXTBUFSIZE];
#endif

    if ((daemon == NULL) || (apid == NULL) || (apid[0] == '\0') || (ecu == NULL))
        return (DltDaemonApplication *)NULL;

    user_list = dlt_daemon_find_users_list(daemon, ecu, verbose);

    if (user_list == NULL)
        return (DltDaemonApplication *)NULL;

    if (user_list->applications == NULL) {
        user_list->applications = (DltDaemonApplication *)
            malloc(sizeof(DltDaemonApplication) * DLT_DAEMON_APPL_ALLOC_SIZE);

        if (user_list->applications == NULL)
            return (DltDaemonApplication *)NULL;
    }

    new_application = 0;

    /* Check if application [apid] is already available */
    application = dlt_daemon_application_find(daemon, apid, ecu, verbose);

    if (application == NULL) {
        user_list->num_applications += 1;

        if (user_list->num_applications != 0) {
            if ((user_list->num_applications % DLT_DAEMON_APPL_ALLOC_SIZE) == 0) {
                /* allocate memory in steps of DLT_DAEMON_APPL_ALLOC_SIZE, e.g. 100 */
                old = user_list->applications;
                user_list->applications = (DltDaemonApplication *)
                    malloc(sizeof(DltDaemonApplication) *
                           ((user_list->num_applications / DLT_DAEMON_APPL_ALLOC_SIZE) + 1) *
                           DLT_DAEMON_APPL_ALLOC_SIZE);

                if (user_list->applications == NULL) {
                    user_list->applications = old;
                    user_list->num_applications -= 1;
                    return (DltDaemonApplication *)NULL;
                }

                memcpy(user_list->applications,
                       old,
                       sizeof(DltDaemonApplication) * user_list->num_applications);
                free(old);
            }
        }

        application = &(user_list->applications[user_list->num_applications - 1]);

        dlt_set_id(application->apid, apid);
        application->pid = 0;
        application->application_description = NULL;
        application->num_contexts = 0;
        application->user_handle = DLT_FD_INIT;
        application->owns_user_handle = false;

        new_application = 1;

    }
    else if ((pid != application->pid) && (application->pid != 0))
    {

        dlt_vlog(LOG_WARNING,
                 "Duplicate registration of ApplicationID: '%.4s'; registering from PID %d, existing from PID %d\n",
                 apid,
                 pid,
                 application->pid);
    }

    /* Store application description and pid of application */
    if (application->application_description) {
        free(application->application_description);
        application->application_description = NULL;
    }

    if (description != NULL) {
        application->application_description = malloc(strlen(description) + 1);

        if (application->application_description) {
            memcpy(application->application_description, description, strlen(description) + 1);
        } else {
            dlt_log(LOG_ERR, "Cannot allocate memory to store application description\n");
            free(application);
            return (DltDaemonApplication *)NULL;
        }
    }

    if (application->pid != pid) {
        dlt_daemon_application_reset_user_handle(daemon, application, verbose);
        application->pid = 0;
    }

    /* open user pipe only if it is not yet opened */
    if ((application->user_handle == DLT_FD_INIT) && (pid != 0)) {
        dlt_user_handle = DLT_FD_INIT;
        owns_user_handle = false;

#if defined DLT_DAEMON_USE_UNIX_SOCKET_IPC || defined DLT_DAEMON_VSOCK_IPC_ENABLE
        if (fd >= DLT_FD_MINIMUM) {
            dlt_user_handle = fd;
            owns_user_handle = false;
        }
#endif
#ifdef DLT_DAEMON_USE_FIFO_IPC
        if (dlt_user_handle < DLT_FD_MINIMUM) {
            snprintf(filename,
                     DLT_DAEMON_COMMON_TEXTBUFSIZE,
                     "%s/dltpipes/dlt%d",
                     dltFifoBaseDir,
                     pid);

            dlt_user_handle = open(filename, O_WRONLY | O_NONBLOCK);

            if (dlt_user_handle < 0) {
                int prio = (errno == ENOENT) ? LOG_INFO : LOG_WARNING;
                dlt_vlog(prio, "open() failed to %s, errno=%d (%s)!\n", filename, errno, strerror(errno));
            } else {
                owns_user_handle = true;
            }
        }
#endif
        /* check if file descriptor was already used, and make it invalid if it
        * is reused. This prevents sending messages to wrong file descriptor */
        dlt_daemon_applications_invalidate_fd(daemon, ecu, dlt_user_handle, verbose);
        dlt_daemon_contexts_invalidate_fd(daemon, ecu, dlt_user_handle, verbose);

        application->user_handle = dlt_user_handle;
        application->owns_user_handle = owns_user_handle;
        application->pid = pid;
    }

    /* Sort */
    if (new_application) {
        qsort(user_list->applications,
              (size_t) user_list->num_applications,
              sizeof(DltDaemonApplication),
              dlt_daemon_cmp_apid);

        /* Find new position of application with apid*/
        application = dlt_daemon_application_find(daemon, apid, ecu, verbose);
    }

#ifdef DLT_LOG_LEVEL_APP_CONFIG
    application->num_context_log_level_settings = 0;
    application->context_log_level_settings = NULL;
#endif

    return application;
}

int dlt_daemon_application_del(DltDaemon *daemon,
                               DltDaemonApplication *application,
                               char *ecu,
                               int verbose)
{
    int pos;
    DltDaemonRegisteredUsers *user_list = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (application == NULL) || (ecu == NULL))
        return -1;

    user_list = dlt_daemon_find_users_list(daemon, ecu, verbose);

    if (user_list == NULL)
        return -1;

    if (user_list->num_applications > 0) {
        dlt_daemon_application_reset_user_handle(daemon, application, verbose);

        /* Free description of application to be deleted */
        if (application->application_description) {
            free(application->application_description);
            application->application_description = NULL;
        }

        pos = (int) (application - (user_list->applications));

        /* move all applications above pos to pos */
        memmove(&(user_list->applications[pos]),
                &(user_list->applications[pos + 1]),
                sizeof(DltDaemonApplication) * ((user_list->num_applications - 1) - pos));

        /* Clear last application */
        memset(&(user_list->applications[user_list->num_applications - 1]),
               0,
               sizeof(DltDaemonApplication));

        user_list->num_applications--;
    }

    return 0;
}

DltDaemonApplication *dlt_daemon_application_find(DltDaemon *daemon,
                                                  char *apid,
                                                  char *ecu,
                                                  int verbose)
{
    DltDaemonApplication application;
    DltDaemonRegisteredUsers *user_list = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon->user_list == NULL) || (apid == NULL) ||
        (apid[0] == '\0') || (ecu == NULL))
        return (DltDaemonApplication *)NULL;

    user_list = dlt_daemon_find_users_list(daemon, ecu, verbose);

    if ((user_list == NULL) || (user_list->num_applications == 0))
        return (DltDaemonApplication *)NULL;

    /* Check, if apid is smaller than smallest apid or greater than greatest apid */
    if ((memcmp(apid, user_list->applications[0].apid, DLT_ID_SIZE) < 0) ||
        (memcmp(apid,
                user_list->applications[user_list->num_applications - 1].apid,
                DLT_ID_SIZE) > 0))
        return (DltDaemonApplication *)NULL;

    dlt_set_id(application.apid, apid);
    return (DltDaemonApplication *)bsearch(&application,
                                           user_list->applications,
                                           (size_t) user_list->num_applications,
                                           sizeof(DltDaemonApplication),
                                           dlt_daemon_cmp_apid);
}

int dlt_daemon_applications_load(DltDaemon *daemon, const char *filename, int verbose)
{
    FILE *fd;
    ID4 apid;
    char buf[DLT_DAEMON_COMMON_TEXTBUFSIZE];
    char *ret;
    char *pb;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (filename == NULL) || (filename[0] == '\0'))
        return -1;

    fd = fopen(filename, "r");

    if (fd == NULL) {
        dlt_vlog(LOG_WARNING,
                 "%s: cannot open file %s: %s\n",
                 __func__,
                 filename,
                 strerror(errno));

        return -1;
    }

    while (!feof(fd)) {
        /* Clear buf */
        memset(buf, 0, sizeof(buf));

        /* Get line */
        ret = fgets(buf, sizeof(buf), fd);

        if (NULL == ret) {
            /* fgets always null pointer if the last byte of the file is a new line
             * We need to check here if there was an error or was it feof.*/
            if (ferror(fd)) {
                dlt_vlog(LOG_WARNING,
                         "%s: fgets(buf,sizeof(buf),fd) returned NULL. %s\n",
                         __func__,
                         strerror(errno));
                fclose(fd);
                return -1;
            }
            else if (feof(fd))
            {
                fclose(fd);
                return 0;
            }
            else {
                dlt_vlog(LOG_WARNING,
                         "%s: fgets(buf,sizeof(buf),fd) returned NULL. Unknown error.\n",
                         __func__);
                fclose(fd);
                return -1;
            }
        }

        if (strcmp(buf, "") != 0) {
            /* Split line */
            pb = strtok(buf, ":");

            if (pb != NULL) {
                dlt_set_id(apid, pb);
                pb = strtok(NULL, ":");

                if (pb != NULL) {
                    /* pb contains now the description */
                    /* pid is unknown at loading time */
                    if (dlt_daemon_application_add(daemon,
                                                   apid,
                                                   0,
                                                   pb,
                                                   -1,
                                                   daemon->ecuid,
                                                   verbose) == 0) {
                        dlt_vlog(LOG_WARNING,
                                 "%s: dlt_daemon_application_add failed for %4s\n",
                                 __func__,
                                 apid);
                        fclose(fd);
                        return -1;
                    }
                }
            }
        }
    }

    fclose(fd);

    return 0;
}

int dlt_daemon_applications_save(DltDaemon *daemon, const char *filename, int verbose)
{
    FILE *fd;
    int i;

    char apid[DLT_ID_SIZE + 1]; /* DLT_ID_SIZE+1, because the 0-termination is required here */
    DltDaemonRegisteredUsers *user_list = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (filename == NULL) || (filename[0] == '\0'))
        return -1;

    memset(apid, 0, sizeof(apid));

    user_list = dlt_daemon_find_users_list(daemon, daemon->ecuid, verbose);

    if (user_list == NULL)
        return -1;

    if ((user_list->applications != NULL) && (user_list->num_applications > 0)) {
        fd = fopen(filename, "w");

        if (fd != NULL) {
            for (i = 0; i < user_list->num_applications; i++) {
                dlt_set_id(apid, user_list->applications[i].apid);

                if ((user_list->applications[i].application_description) &&
                    (user_list->applications[i].application_description[0] != '\0'))
                    fprintf(fd,
                            "%s:%s:\n",
                            apid,
                            user_list->applications[i].application_description);
                else
                    fprintf(fd, "%s::\n", apid);
            }

            fclose(fd);
        }
        else {
            dlt_vlog(LOG_ERR, "%s: open %s failed! No application information stored.\n",
                     __func__,
                     filename);
        }
    }

    return 0;
}

DltDaemonContext *dlt_daemon_context_add(DltDaemon *daemon,
                                         char *apid,
                                         char *ctid,
                                         int8_t log_level,
                                         int8_t trace_status,
                                         int log_level_pos,
                                         int user_handle,
                                         char *description,
                                         char *ecu,
                                         int verbose)
{
    DltDaemonApplication *application;
    DltDaemonContext *context;
    DltDaemonContext *old;
    int new_context = 0;
    DltDaemonRegisteredUsers *user_list = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (apid == NULL) || (apid[0] == '\0') ||
        (ctid == NULL) || (ctid[0] == '\0') || (ecu == NULL))
        return (DltDaemonContext *)NULL;

    if ((log_level < DLT_LOG_DEFAULT) || (log_level > DLT_LOG_VERBOSE))
        return (DltDaemonContext *)NULL;

    if ((trace_status < DLT_TRACE_STATUS_DEFAULT) || (trace_status > DLT_TRACE_STATUS_ON))
        return (DltDaemonContext *)NULL;

    user_list = dlt_daemon_find_users_list(daemon, ecu, verbose);

    if (user_list == NULL)
        return (DltDaemonContext *)NULL;

    if (user_list->contexts == NULL) {
        user_list->contexts = (DltDaemonContext *)malloc(sizeof(DltDaemonContext) * DLT_DAEMON_CONTEXT_ALLOC_SIZE);

        if (user_list->contexts == NULL)
            return (DltDaemonContext *)NULL;
    }

    /* Check if application [apid] is available */
    application = dlt_daemon_application_find(daemon, apid, ecu, verbose);

    if (application == NULL)
        return (DltDaemonContext *)NULL;

    /* Check if context [apid, ctid] is already available */
    context = dlt_daemon_context_find(daemon, apid, ctid, ecu, verbose);

    if (context == NULL) {
        user_list->num_contexts += 1;

        if (user_list->num_contexts != 0) {
            if ((user_list->num_contexts % DLT_DAEMON_CONTEXT_ALLOC_SIZE) == 0) {
                /* allocate memory for context in steps of DLT_DAEMON_CONTEXT_ALLOC_SIZE, e.g 100 */
                old = user_list->contexts;
                user_list->contexts = (DltDaemonContext *)malloc((size_t) sizeof(DltDaemonContext) *
                                                                 ((user_list->num_contexts /
                                                                   DLT_DAEMON_CONTEXT_ALLOC_SIZE) + 1) *
                                                                 DLT_DAEMON_CONTEXT_ALLOC_SIZE);

                if (user_list->contexts == NULL) {
                    user_list->contexts = old;
                    user_list->num_contexts -= 1;
                    return (DltDaemonContext *)NULL;
                }

                memcpy(user_list->contexts,
                       old,
                       (size_t) sizeof(DltDaemonContext) * user_list->num_contexts);
                free(old);
            }
        }

        context = &(user_list->contexts[user_list->num_contexts - 1]);

        dlt_set_id(context->apid, apid);
        dlt_set_id(context->ctid, ctid);
        context->context_description = NULL;

        application->num_contexts++;
        new_context = 1;
    }

    /* Set context description */
    if (context->context_description) {
        free(context->context_description);
        context->context_description = NULL;
    }

    if (description != NULL) {
        context->context_description = malloc(strlen(description) + 1);

        if (context->context_description) {
            memcpy(context->context_description, description, strlen(description) + 1);
        }
    }

#ifdef DLT_LOG_LEVEL_APP_CONFIG
    /* configure initial log level */
    DltDaemonContextLogSettings *settings = NULL;
    settings = dlt_daemon_find_configured_app_id_ctx_id_settings(
            daemon, context->apid, ctid);

    if (settings != NULL) {
        /* set log level */
        log_level = settings->log_level;

        DltDaemonContextLogSettings *ct_settings = NULL;
        ct_settings = dlt_daemon_find_app_log_level_config(application, ctid);

        /* ct_settings != null: context and app id combination already exists */
        if (ct_settings == NULL) {
          /* copy the configuration into the DltDaemonApplication for faster access later */
          DltDaemonContextLogSettings *tmp =
              realloc(application->context_log_level_settings,
                      (++application->num_context_log_level_settings) *
                          sizeof(DltDaemonContextLogSettings));
          application->context_log_level_settings = tmp;

          ct_settings =
              &application->context_log_level_settings[application->num_context_log_level_settings - 1];
          memcpy(ct_settings, settings, sizeof(DltDaemonContextLogSettings));
          memcpy(ct_settings->ctid, ctid, DLT_ID_SIZE);
      }
    }
#endif

    if ((strncmp(daemon->ecuid, ecu, DLT_ID_SIZE) == 0) && (daemon->force_ll_ts)) {
#ifdef DLT_LOG_LEVEL_APP_CONFIG
        if (log_level > daemon->default_log_level && settings == NULL)
#else
        if (log_level > daemon->default_log_level)
#endif
            log_level = daemon->default_log_level;

        if (trace_status > daemon->default_trace_status)
            trace_status = daemon->default_trace_status;

        dlt_vlog(LOG_NOTICE,
            "Adapting ll_ts for context: %.4s:%.4s with %i %i\n",
            apid,
            ctid,
            log_level,
            trace_status);
    }

    /* Store log level and trace status,
     * if this is a new context, or
     * if this is an old context and the runtime cfg was not loaded */
    if ((new_context == 1) ||
        ((new_context == 0) && (daemon->runtime_context_cfg_loaded == 0))) {
        context->log_level = log_level;
        context->trace_status = trace_status;
    }

    context->log_level_pos = log_level_pos;
    context->user_handle = user_handle;

    /* In case a context is loaded from runtime config file,
     * the user_handle is 0 and we mark that context as predefined.
     */
    if (context->user_handle == 0)
        context->predefined = true;
    else
        context->predefined = false;

    /* Sort */
    if (new_context) {
        qsort(user_list->contexts,
              (size_t) user_list->num_contexts,
              sizeof(DltDaemonContext),
              dlt_daemon_cmp_apid_ctid);

        /* Find new position of context with apid, ctid */
        context = dlt_daemon_context_find(daemon, apid, ctid, ecu, verbose);
    }

    return context;
}

#ifdef DLT_LOG_LEVEL_APP_CONFIG
static void dlt_daemon_free_context_log_settings(
    DltDaemonApplication *application,
    DltDaemonContext *context)
{
    DltDaemonContextLogSettings *ct_settings;
    int i;
    int skipped = 0;

    ct_settings = dlt_daemon_find_app_log_level_config(application, context->ctid);
    if (ct_settings == NULL) {
        return;
    }

    /* move all data forward */
    for (i = 0; i < application->num_context_log_level_settings; ++i) {
        /* skip given context to delete it */
        if (i + skipped < application->num_context_log_level_settings &&
            strncmp(application->context_log_level_settings[i+skipped].ctid, context->ctid, DLT_ID_SIZE) == 0) {
            ++skipped;
            continue;
        }

        memcpy(&application->context_log_level_settings[i-skipped],
                &application->context_log_level_settings[i],
                sizeof(DltDaemonContextLogSettings));
    }

    application->num_context_log_level_settings -= skipped;

    /* if size is equal to zero, and ptr is not NULL, then realloc is equivalent to free(ptr) */
    application->context_log_level_settings = realloc(application->context_log_level_settings,
            sizeof(DltDaemonContextLogSettings) * (application->num_context_log_level_settings));

}
#endif

int dlt_daemon_context_del(DltDaemon *daemon,
                           DltDaemonContext *context,
                           char *ecu,
                           int verbose)
{
    int pos;
    DltDaemonApplication *application;
    DltDaemonRegisteredUsers *user_list = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (context == NULL) || (ecu == NULL))
        return -1;

    user_list = dlt_daemon_find_users_list(daemon, ecu, verbose);

    if (user_list == NULL)
        return -1;

    if (user_list->num_contexts > 0) {
        application = dlt_daemon_application_find(daemon, context->apid, ecu, verbose);

#ifdef DLT_LOG_LEVEL_APP_CONFIG
        dlt_daemon_free_context_log_settings(application, context);
#endif
        /* Free description of context to be deleted */
        if (context->context_description) {
            free(context->context_description);
            context->context_description = NULL;
        }

        pos = (int) (context - (user_list->contexts));

        /* move all contexts above pos to pos */
        memmove(&(user_list->contexts[pos]),
                &(user_list->contexts[pos + 1]),
                sizeof(DltDaemonContext) * ((user_list->num_contexts - 1) - pos));

        /* Clear last context */
        memset(&(user_list->contexts[user_list->num_contexts - 1]),
               0,
               sizeof(DltDaemonContext));

        user_list->num_contexts--;

        /* Check if application [apid] is available */
        if (application != NULL)
            application->num_contexts--;
    }

    return 0;
}

DltDaemonContext *dlt_daemon_context_find(DltDaemon *daemon,
                                          char *apid,
                                          char *ctid,
                                          char *ecu,
                                          int verbose)
{
    DltDaemonContext context;
    DltDaemonRegisteredUsers *user_list = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (apid == NULL) || (apid[0] == '\0') ||
        (ctid == NULL) || (ctid[0] == '\0') || (ecu == NULL))
        return (DltDaemonContext *)NULL;

    user_list = dlt_daemon_find_users_list(daemon, ecu, verbose);

    if ((user_list == NULL) || (user_list->num_contexts == 0))
        return (DltDaemonContext *)NULL;

    /* Check, if apid is smaller than smallest apid or greater than greatest apid */
    if ((memcmp(apid, user_list->contexts[0].apid, DLT_ID_SIZE) < 0) ||
        (memcmp(apid,
                user_list->contexts[user_list->num_contexts - 1].apid,
                DLT_ID_SIZE) > 0))
        return (DltDaemonContext *)NULL;

    dlt_set_id(context.apid, apid);
    dlt_set_id(context.ctid, ctid);

    return (DltDaemonContext *)bsearch(&context,
                                       user_list->contexts,
                                       (size_t) user_list->num_contexts,
                                       sizeof(DltDaemonContext),
                                       dlt_daemon_cmp_apid_ctid);
}

int dlt_daemon_contexts_invalidate_fd(DltDaemon *daemon,
                                      char *ecu,
                                      int fd,
                                      int verbose)
{
    int i;
    DltDaemonRegisteredUsers *user_list = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (ecu == NULL))
        return -1;

    user_list = dlt_daemon_find_users_list(daemon, ecu, verbose);

    if (user_list != NULL) {
        for (i = 0; i < user_list->num_contexts; i++)
            if (user_list->contexts[i].user_handle == fd)
                user_list->contexts[i].user_handle = DLT_FD_INIT;

        return 0;
    }

    return -1;
}

int dlt_daemon_contexts_clear(DltDaemon *daemon, char *ecu, int verbose)
{
    int i;
    DltDaemonRegisteredUsers *users = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (ecu == NULL))
        return DLT_RETURN_WRONG_PARAMETER;

    users = dlt_daemon_find_users_list(daemon, ecu, verbose);

    if (users == NULL)
        return DLT_RETURN_ERROR;

    for (i = 0; i < users->num_contexts; i++)
        if (users->contexts[i].context_description != NULL) {
            free(users->contexts[i].context_description);
            users->contexts[i].context_description = NULL;
        }

    if (users->contexts) {
        free(users->contexts);
        users->contexts = NULL;
    }

    for (i = 0; i < users->num_applications; i++)
        users->applications[i].num_contexts = 0;

    users->num_contexts = 0;

    return 0;
}

int dlt_daemon_contexts_load(DltDaemon *daemon, const char *filename, int verbose)
{
    FILE *fd;
    ID4 apid, ctid;
    char buf[DLT_DAEMON_COMMON_TEXTBUFSIZE];
    char *ret;
    char *pb;
    int ll, ts;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (filename == NULL) || (filename[0] == '\0'))
        return -1;

    fd = fopen(filename, "r");

    if (fd == NULL) {
        dlt_vlog(LOG_WARNING,
                 "DLT runtime-context load, cannot open file %s: %s\n",
                 filename,
                 strerror(errno));

        return -1;
    }

    while (!feof(fd)) {
        /* Clear buf */
        memset(buf, 0, sizeof(buf));

        /* Get line */
        ret = fgets(buf, sizeof(buf), fd);

        if (NULL == ret) {
            /* fgets always returns null pointer if the last byte of the file is a new line.
             * We need to check here if there was an error or was it feof.*/
            if (ferror(fd)) {
                dlt_vlog(LOG_WARNING,
                         "%s fgets(buf,sizeof(buf),fd) returned NULL. %s\n",
                         __func__,
                         strerror(errno));
                fclose(fd);
                return -1;
            }
            else if (feof(fd))
            {
                fclose(fd);
                return 0;
            }
            else {
                dlt_vlog(LOG_WARNING,
                         "%s fgets(buf,sizeof(buf),fd) returned NULL. Unknown error.\n",
                         __func__);
                fclose(fd);
                return -1;
            }
        }

        if (strcmp(buf, "") != 0) {
            /* Split line */
            pb = strtok(buf, ":");

            if (pb != NULL) {
                dlt_set_id(apid, pb);
                pb = strtok(NULL, ":");

                if (pb != NULL) {
                    dlt_set_id(ctid, pb);
                    pb = strtok(NULL, ":");

                    if (pb != NULL) {
                        sscanf(pb, "%d", &ll);
                        pb = strtok(NULL, ":");

                        if (pb != NULL) {
                            sscanf(pb, "%d", &ts);
                            pb = strtok(NULL, ":");

                            if (pb != NULL) {
                                /* pb contains now the description */

                                /* log_level_pos, and user_handle are unknown at loading time */
                                if (dlt_daemon_context_add(daemon,
                                                           apid,
                                                           ctid,
                                                           (int8_t)ll,
                                                           (int8_t)ts,
                                                           0,
                                                           0,
                                                           pb,
                                                           daemon->ecuid,
                                                           verbose) == NULL) {
                                    dlt_vlog(LOG_WARNING,
                                             "%s dlt_daemon_context_add failed\n",
                                             __func__);
                                    fclose(fd);
                                    return -1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    fclose(fd);

    return 0;
}

int dlt_daemon_contexts_save(DltDaemon *daemon, const char *filename, int verbose)
{
    FILE *fd;
    int i;

    char apid[DLT_ID_SIZE + 1], ctid[DLT_ID_SIZE + 1]; /* DLT_ID_SIZE+1, because the 0-termination is required here */
    DltDaemonRegisteredUsers *user_list = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (filename == NULL) || (filename[0] == '\0'))
        return -1;

    user_list = dlt_daemon_find_users_list(daemon, daemon->ecuid, verbose);

    if (user_list == NULL)
        return -1;

    memset(apid, 0, sizeof(apid));
    memset(ctid, 0, sizeof(ctid));

    if ((user_list->contexts) && (user_list->num_contexts > 0)) {
        fd = fopen(filename, "w");

        if (fd != NULL) {
            for (i = 0; i < user_list->num_contexts; i++) {
                dlt_set_id(apid, user_list->contexts[i].apid);
                dlt_set_id(ctid, user_list->contexts[i].ctid);

                if ((user_list->contexts[i].context_description) &&
                    (user_list->contexts[i].context_description[0] != '\0'))
                    fprintf(fd, "%s:%s:%d:%d:%s:\n", apid, ctid,
                            (int)(user_list->contexts[i].log_level),
                            (int)(user_list->contexts[i].trace_status),
                            user_list->contexts[i].context_description);
                else
                    fprintf(fd, "%s:%s:%d:%d::\n", apid, ctid,
                            (int)(user_list->contexts[i].log_level),
                            (int)(user_list->contexts[i].trace_status));
            }

            fclose(fd);
        }
        else {
            dlt_vlog(LOG_ERR,
                     "%s: Cannot open %s. No context information stored\n",
                     __func__,
                     filename);
        }
    }

    return 0;
}

int dlt_daemon_configuration_save(DltDaemon *daemon, const char *filename, int verbose)
{
    FILE *fd;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (filename == NULL) || (filename[0] == '\0'))
        return -1;

    fd = fopen(filename, "w");

    if (fd != NULL) {
        fprintf(fd, "# 0 = off, 1 = external, 2 = internal, 3 = both\n");
        fprintf(fd, "LoggingMode = %d\n", daemon->mode);

        fclose(fd);
    }

    return 0;
}

int dlt_daemon_configuration_load(DltDaemon *daemon, const char *filename, int verbose)
{
    if ((daemon == NULL) || (filename == NULL))
        return -1;

    FILE *pFile;
    char line[1024];
    char token[1024];
    char value[1024];
    char *pch;

    PRINT_FUNCTION_VERBOSE(verbose);

    pFile = fopen (filename, "r");

    if (pFile != NULL) {
        while (1) {
            /* fetch line from configuration file */
            if (fgets (line, 1024, pFile) != NULL) {
                pch = strtok (line, " =\r\n");
                token[0] = 0;
                value[0] = 0;

                while (pch != NULL) {
                    if (strcmp(pch, "#") == 0)
                        break;

                    if (token[0] == 0) {
                        strncpy(token, pch, sizeof(token) - 1);
                        token[sizeof(token) - 1] = 0;
                    }
                    else {
                        strncpy(value, pch, sizeof(value) - 1);
                        value[sizeof(value) - 1] = 0;
                        break;
                    }

                    pch = strtok (NULL, " =\r\n");
                }

                if (token[0] && value[0]) {
                    /* parse arguments here */
                    if (strcmp(token, "LoggingMode") == 0) {
                        daemon->mode = atoi(value);
                        dlt_vlog(LOG_INFO, "Runtime Option: %s=%d\n", token,
                                 daemon->mode);
                    }
                    else {
                        dlt_vlog(LOG_WARNING, "Unknown option: %s=%s\n", token,
                                 value);
                    }
                }
            }
            else {
                break;
            }
        }

        fclose (pFile);
    }
    else {
        dlt_vlog(LOG_INFO, "Cannot open configuration file: %s\n", filename);
    }

    return 0;
}

int dlt_daemon_user_send_log_level(DltDaemon *daemon, DltDaemonContext *context, int verbose)
{
    DltUserHeader userheader;
    DltUserControlMsgLogLevel usercontext;
    DltReturnValue ret;
    DltDaemonApplication *app;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (context == NULL)) {
        dlt_vlog(LOG_ERR, "NULL parameter in %s", __func__);
        return -1;
    }

    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_LOG_LEVEL) < DLT_RETURN_OK) {
        dlt_vlog(LOG_ERR, "Failed to set userheader in %s", __func__);
        return -1;
    }

    if ((context->storage_log_level != DLT_LOG_DEFAULT) &&
        (daemon->maintain_logstorage_loglevel != DLT_MAINTAIN_LOGSTORAGE_LOGLEVEL_OFF))
            usercontext.log_level = (uint8_t) (context->log_level >
                context->storage_log_level ? context->log_level : context->storage_log_level);
    else /* Storage log level is not updated (is DEFAULT) then  no device is yet connected so ignore */
        usercontext.log_level =
            (uint8_t) ((context->log_level == DLT_LOG_DEFAULT) ? daemon->default_log_level : context->log_level);

    usercontext.trace_status =
        (uint8_t) ((context->trace_status == DLT_TRACE_STATUS_DEFAULT) ? daemon->default_trace_status : context->trace_status);

    usercontext.log_level_pos = context->log_level_pos;

    dlt_vlog(LOG_NOTICE, "Send log-level to context: %.4s:%.4s [%i -> %i] [%i -> %i]\n",
             context->apid,
             context->ctid,
             context->log_level,
             usercontext.log_level,
             context->trace_status,
             usercontext.trace_status);

    /* log to FIFO */
    errno = 0;
    ret = dlt_user_log_out2_with_timeout(context->user_handle,
                            &(userheader), sizeof(DltUserHeader),
                            &(usercontext), sizeof(DltUserControlMsgLogLevel));

    if (ret < DLT_RETURN_OK) {
        dlt_vlog(LOG_ERR, "Failed to send data to application in %s: %s",
                 __func__,
                 errno != 0 ? strerror(errno) : "Unknown error");

        if (errno == EPIPE) {
            app = dlt_daemon_application_find(daemon, context->apid, daemon->ecuid, verbose);
            if (app != NULL)
                dlt_daemon_application_reset_user_handle(daemon, app, verbose);
        }
    }

    return (ret == DLT_RETURN_OK) ? DLT_RETURN_OK : DLT_RETURN_ERROR;
}

int dlt_daemon_user_send_log_state(DltDaemon *daemon, DltDaemonApplication *app, int verbose)
{
    DltUserHeader userheader;
    DltUserControlMsgLogState logstate;
    DltReturnValue ret;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (app == NULL))
        return -1;

    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_LOG_STATE) < DLT_RETURN_OK)
        return -1;

    logstate.log_state = daemon->connectionState;

    /* log to FIFO */
    ret = dlt_user_log_out2_with_timeout(app->user_handle,
                            &(userheader), sizeof(DltUserHeader),
                            &(logstate), sizeof(DltUserControlMsgLogState));

    if (ret < DLT_RETURN_OK) {
        if (errno == EPIPE)
            dlt_daemon_application_reset_user_handle(daemon, app, verbose);
    }

    return (ret == DLT_RETURN_OK) ? DLT_RETURN_OK : DLT_RETURN_ERROR;
}

void dlt_daemon_control_reset_to_factory_default(DltDaemon *daemon,
                                                 const char *filename,
                                                 const char *filename1,
                                                 int InitialContextLogLevel,
                                                 int InitialContextTraceStatus,
                                                 int InitialEnforceLlTsStatus,
                                                 int verbose)
{
    FILE *fd;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (filename == NULL) || (filename1 == NULL)) {
        dlt_log(LOG_WARNING, "Wrong parameter: Null pointer\n");
        return;
    }

    if ((filename[0] == '\0') || (filename1[0] == '\0')) {
        dlt_log(LOG_WARNING, "Wrong parameter: Empty string\n");
        return;
    }

    /* Check for runtime cfg file and delete it, if available */
    fd = fopen(filename, "r");

    if (fd != NULL) {
        /* Close and delete file */
        fclose(fd);
        unlink(filename);
    }

    fd = fopen(filename1, "r");

    if (fd != NULL) {
        /* Close and delete file */
        fclose(fd);
        unlink(filename1);
    }

    daemon->default_log_level = (int8_t) InitialContextLogLevel;
    daemon->default_trace_status = (int8_t) InitialContextTraceStatus;
    daemon->force_ll_ts = (int8_t) InitialEnforceLlTsStatus;

    /* Reset all other things (log level, trace status, etc.
     *                         to default values             */

    /* Inform user libraries about changed default log level/trace status */
    dlt_daemon_user_send_default_update(daemon, verbose);
}

void dlt_daemon_user_send_default_update(DltDaemon *daemon, int verbose)
{
    int32_t count;
    DltDaemonContext *context;
    DltDaemonRegisteredUsers *user_list = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == NULL) {
        dlt_log(LOG_WARNING, "Wrong parameter: Null pointer\n");
        return;
    }

    user_list = dlt_daemon_find_users_list(daemon, daemon->ecuid, verbose);

    if (user_list == NULL)
        return;

    for (count = 0; count < user_list->num_contexts; count++) {
        context = &(user_list->contexts[count]);

        if (context != NULL) {
            if ((context->log_level == DLT_LOG_DEFAULT) ||
                (context->trace_status == DLT_TRACE_STATUS_DEFAULT)) {
                if (context->user_handle >= DLT_FD_MINIMUM)
                    if (dlt_daemon_user_send_log_level(daemon,
                                                       context,
                                                       verbose) == -1)
                        dlt_vlog(LOG_WARNING, "Cannot update default of %.4s:%.4s\n", context->apid, context->ctid);
            }
        }
    }
}

void dlt_daemon_user_send_all_log_level_update(DltDaemon *daemon,
                                               int enforce_context_ll_and_ts,
                                               int8_t context_log_level,
                                               int8_t log_level,
                                               int verbose)
{
    int32_t count = 0;
    DltDaemonContext *context = NULL;
    DltDaemonRegisteredUsers *user_list = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == NULL)
        return;

    user_list = dlt_daemon_find_users_list(daemon, daemon->ecuid, verbose);

    if (user_list == NULL)
        return;

    for (count = 0; count < user_list->num_contexts; count++) {
        context = &(user_list->contexts[count]);

        if (context) {
            if (context->user_handle >= DLT_FD_MINIMUM) {
                context->log_level = log_level;

                if (enforce_context_ll_and_ts) {
#ifdef DLT_LOG_LEVEL_APP_CONFIG
                    DltDaemonContextLogSettings *settings =
                        dlt_daemon_find_configured_app_id_ctx_id_settings(
                            daemon, context->apid, context->ctid);
                    if (settings != NULL) {
                        if (log_level > settings->log_level) {
                          context->log_level = settings->log_level;
                        }
                    } else
#endif
                    if (log_level > context_log_level) {
                        context->log_level = (int8_t)context_log_level;
                    }
                }

                if (dlt_daemon_user_send_log_level(daemon,
                                                   context,
                                                   verbose) == -1)
                    dlt_vlog(LOG_WARNING,
                             "Cannot send log level %.4s:%.4s -> %i\n",
                             context->apid,
                             context->ctid,
                             context->log_level);
            }
        }
    }
}

void dlt_daemon_user_send_all_trace_status_update(DltDaemon *daemon, int8_t trace_status, int verbose)
{
    int32_t count = 0;
    DltDaemonContext *context = NULL;
    DltDaemonRegisteredUsers *user_list = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == NULL)
        return;

    user_list = dlt_daemon_find_users_list(daemon, daemon->ecuid, verbose);

    if (user_list == NULL)
        return;

    dlt_vlog(LOG_NOTICE, "All trace status is updated -> %i\n", trace_status);

    for (count = 0; count < user_list->num_contexts; count++) {
        context = &(user_list->contexts[count]);

        if (context) {
            if (context->user_handle >= DLT_FD_MINIMUM) {
                context->trace_status = trace_status;

                if (dlt_daemon_user_send_log_level(daemon, context, verbose) == -1)
                    dlt_vlog(LOG_WARNING,
                             "Cannot send trace status %.4s:%.4s -> %i\n",
                             context->apid,
                             context->ctid,
                             context->trace_status);
            }
        }
    }
}

void dlt_daemon_user_send_all_log_state(DltDaemon *daemon, int verbose)
{
    int32_t count;
    DltDaemonApplication *app;
    DltDaemonRegisteredUsers *user_list = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == NULL) {
        dlt_log(LOG_WARNING, "Wrong parameter: Null pointer\n");
        return;
    }

    user_list = dlt_daemon_find_users_list(daemon, daemon->ecuid, verbose);

    if (user_list == NULL)
        return;

    for (count = 0; count < user_list->num_applications; count++) {
        app = &(user_list->applications[count]);

        if (app != NULL) {
            if (app->user_handle >= DLT_FD_MINIMUM)
                if (dlt_daemon_user_send_log_state(daemon, app, verbose) == -1)
                    dlt_vlog(LOG_WARNING, "Cannot send log state to Apid: %.4s, PID: %d\n", app->apid, app->pid);
        }
    }
}

void dlt_daemon_change_state(DltDaemon *daemon, DltDaemonState newState)
{
    switch (newState) {
    case DLT_DAEMON_STATE_INIT:
        dlt_log(LOG_INFO, "Switched to init state.\n");
        daemon->state = DLT_DAEMON_STATE_INIT;
        break;
    case DLT_DAEMON_STATE_BUFFER:
        dlt_log(LOG_INFO, "Switched to buffer state for socket connections.\n");
        daemon->state = DLT_DAEMON_STATE_BUFFER;
        break;
    case DLT_DAEMON_STATE_BUFFER_FULL:
        dlt_log(LOG_INFO, "Switched to buffer full state.\n");
        daemon->state = DLT_DAEMON_STATE_BUFFER_FULL;
        break;
    case DLT_DAEMON_STATE_SEND_BUFFER:
        dlt_log(LOG_INFO, "Switched to send buffer state for socket connections.\n");
        daemon->state = DLT_DAEMON_STATE_SEND_BUFFER;
        break;
    case DLT_DAEMON_STATE_SEND_DIRECT:
        dlt_log(LOG_INFO, "Switched to send direct state.\n");
        daemon->state = DLT_DAEMON_STATE_SEND_DIRECT;
        break;
    }
}
