/**
 * Copyright (C) 2013 - 2015  Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * This file is part of COVESA Project Dlt - Diagnostic Log and Trace console apps.
 *
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Syed Hameed <shameed@jp.adit-jv.com> ADIT 2013 - 2015
 * \author Christoph Lipka <clipka@jp.adit-jv.com> ADIT 2015
 * \author Frederic Berat <fberat@de.adit-jv.com> ADIT 2015
 *
 * \file dlt-logstorage-common.c
 * For further information see http://www.covesa.org/.
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-logstorage-common.c                                       **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Christoph Lipka clipka@jp.adit-jv.com                         **
**              Frederic Berat fberat@de.adit-jv.com                          **
**  PURPOSE   :                                                               **
**                                                                            **
**  REMARKS   : Code extracted from dlt-control-common.c and reworked.        **
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
**  cl          Christoph Lipka            ADIT                               **
**  fb          Frederic Berat             ADIT                               **
*******************************************************************************/
#define pr_fmt(fmt) "Logstorage common: "fmt

#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "dlt_common.h"
#include "dlt_protocol.h"
#include "dlt_client.h"

#include "dlt-control-common.h"
#include "dlt-logstorage-common.h"

#ifdef DLT_LOGSTORAGE_CTRL_UDEV_ENABLE
#   include "dlt-logstorage-udev.h"
#endif

#include "dlt-logstorage-prop.h"

static struct LogstorageOptions {
    int event_type; /**< EVENT_UNMOUNTING/EVENT_MOUNTED */
    char device_path[DLT_MOUNT_PATH_MAX]; /**< Default Mount path */
    DltLogstorageHandler handler_type; /**< be controlled by udev or prop */
    long timeout; /**< Default timeout */
} g_options = {
    .event_type = EVENT_MOUNTED,
    .handler_type = CTRL_NOHANDLER,
};

DltLogstorageHandler get_handler_type(void)
{
    return g_options.handler_type;
}

void set_handler_type(char *type)
{
    g_options.handler_type = CTRL_UDEV;

    if (type && check_proprietary_handling(type))
        g_options.handler_type = CTRL_PROPRIETARY;
}

int get_default_event_type(void)
{
    return g_options.event_type;
}

void set_default_event_type(long type)
{
    g_options.event_type = (int) type;
}

char *get_default_path(void)
{
    return g_options.device_path;
}

void set_default_path(char *path)
{
    memset(g_options.device_path, 0, DLT_MOUNT_PATH_MAX);

    if (path != NULL)
        strncpy(g_options.device_path, path, DLT_MOUNT_PATH_MAX - 1);
}

/* Used by the handlers */
static DltLogstorageCtrl lctrl;

DltLogstorageCtrl *get_logstorage_control(void)
{
    return &lctrl;
}

void *dlt_logstorage_get_handler_cb(void)
{
    return lctrl.callback;
}

int dlt_logstorage_get_handler_fd(void)
{
    return lctrl.fd;
}

/** @brief Initialized the handler based on configuration
 *
 * @return 0 on success, -1 otherwise.
 */
int dlt_logstorage_init_handler(void)
{
    switch (get_handler_type()) {
    case CTRL_PROPRIETARY:
        return dlt_logstorage_prop_init();
    case CTRL_UDEV:
    default:
#ifdef DLT_LOGSTORAGE_CTRL_UDEV_ENABLE
        return dlt_logstorage_udev_init();
#else
        return -1;
#endif
    }
}

/** @brief Clean-up the handler based on configuration
 *
 * @return 0 on success, -1 otherwise.
 */
int dlt_logstorage_deinit_handler(void)
{
    switch (get_handler_type()) {
    case CTRL_PROPRIETARY:
        return dlt_logstorage_prop_deinit();
    case CTRL_UDEV:
    default:
#ifdef DLT_LOGSTORAGE_CTRL_UDEV_ENABLE
        return dlt_logstorage_udev_deinit();
#else
        return -1;
#endif
    }
}

/** @brief Search for config file in given mount point
 *
 * The file is searched at the top directory. The function exits once it
 * founds it.
 *
 * @param mnt_point The mount point to check
 *
 * @return 1 if the file is found, 0 otherwise.
 */
int dlt_logstorage_check_config_file(char *mnt_point)
{
    struct dirent **files;
    int n;
    int i = 0;
    int ret = 0;

    if ((mnt_point == NULL) || (mnt_point[0] == '\0')) {
        pr_error("Mount point missing.\n");
        return ret;
    }

    pr_verbose("Now scanning %s\n", mnt_point);

    n = scandir(mnt_point, &files, NULL, alphasort);

    if (n <= 0) {
        pr_error("Cannot read mounted directory\n");
        return ret;
    }

    do {
        pr_verbose("Checking %s.\n", files[i]->d_name);

        if (strncmp(files[i]->d_name, CONF_NAME, strlen(CONF_NAME)) == 0) {
            /* We found it ! */
            pr_verbose("File found.\n");
            ret = 1;
            break;
        }
    } while (++i < n);

    for (i = 0; i < n; i++)
        free(files[i]);

    free(files);
    return ret;
}

/** @brief Check if given mount point is writable
 *
 * @param mnt_point The mount point to check
 *
 * @return 1 if the file is writable, 0 otherwise.
 */
int dlt_logstorage_check_directory_permission(char *mnt_point)
{
    if (mnt_point == NULL) {
        pr_error("Given mount point is NULL\n");
        return 0;
    }

    if (access(mnt_point, W_OK) == 0)
        return 1;

    return 0;
}

/** @brief Prepares the body of the message to be send to DLT
 *
 * @param body A pointer to the MsgBody structure pointer
 * @param conn_type The type of the event (Mounted/Unmounting)
 * @param path The mount point path.
 *
 * @return The body once built or NULL.
 */
static DltControlMsgBody *prepare_message_body(DltControlMsgBody **body,
                                               int conn_type,
                                               char *path)
{
    DltServiceOfflineLogstorage *serv = NULL;

    if (path == NULL) {
        pr_error("Mount path is uninitialized.\n");
        return NULL;
    }

    pr_verbose("Sending event %d for %s.\n", conn_type, path);

    *body = calloc(1, sizeof(DltControlMsgBody));

    if (!*body) {
        pr_error("Not able to allocate memory for body.\n");
        return *body;
    }

    (*body)->data = calloc(1, sizeof(DltServiceOfflineLogstorage));

    if (!(*body)->data) {
        free(*body);
        *body = NULL;
        pr_error("Not able to allocate memory for body data.\n");
        return NULL;
    }

    (*body)->size = sizeof(DltServiceOfflineLogstorage);

    serv = (DltServiceOfflineLogstorage *)(*body)->data;

    serv->service_id = DLT_SERVICE_ID_OFFLINE_LOGSTORAGE;
    serv->connection_type = (uint8_t) conn_type;
    /* mount_point is DLT_MOUNT_PATH_MAX + 1 long,
     * and the memory is already zeroed.
     */
    strncpy(serv->mount_point, path, DLT_MOUNT_PATH_MAX - 1);

    pr_verbose("Body is now ready.\n");

    return *body;
}

/** @brief Send a logstorage event to DLT
 *
 * @param type The type of the event (Mounted/Unmounting)
 * @param mount_point The mount point for this event
 *
 * @return 0 On success, -1 otherwise.
 */
int dlt_logstorage_send_event(int type, char *mount_point)
{
    int ret = 0;
    DltControlMsgBody *msg_body = NULL;

    /* mount_point is checked against NULL in the preparation */
    if (!prepare_message_body(&msg_body, type, mount_point)) {
        pr_error("Data for Dlt Message body is NULL\n");
        return -1;
    }

    ret = dlt_control_send_message(msg_body, get_timeout());

    free(msg_body->data);
    free(msg_body);

    return ret;
}
