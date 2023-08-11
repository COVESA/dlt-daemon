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
 * \file dlt-logstorage-common.h
 * For further information see http://www.covesa.org/.
 */

#ifndef _DLT_LOGSTORAGE_COMMON_H_
#define _DLT_LOGSTORAGE_COMMON_H_

#define CONF_NAME "dlt_logstorage.conf"

#define EVENT_UNMOUNTING    0
#define EVENT_MOUNTED       1
#define EVENT_SYNC_CACHE    2

typedef enum
{
    CTRL_NOHANDLER = 0, /**< one shot application */
    CTRL_UDEV,          /**< Handles udev events */
    CTRL_PROPRIETARY    /**< Handles proprietary event */
} DltLogstorageHandler;

DltLogstorageHandler get_handler_type(void);
void set_handler_type(char *);

char *get_default_path(void);
void set_default_path(char *);

int get_default_event_type(void);
void set_default_event_type(long type);

typedef struct {
    int fd;
    int (*callback)(void); /* callback for event handling */
    void *prvt; /* Private data */
} DltLogstorageCtrl;

/* Get a reference to the logstorage control instance */
DltLogstorageCtrl *get_logstorage_control(void);
void *dlt_logstorage_get_handler_cb(void);
int dlt_logstorage_get_handler_fd(void);
int dlt_logstorage_init_handler(void);
int dlt_logstorage_deinit_handler(void);

/**
 * Send an event to the dlt daemon
 *
 * @param type Event type (EVENT_UNMOUNTING/EVENT_MOUNTED)
 * @param mount_point The mount point path concerned by this event
 *
 * @return  0 on success, -1 on error
 */
int dlt_logstorage_send_event(int, char *);

/** @brief Search for config file in given mount point
 *
 * The file is searched at the top directory. The function exits once it
 * founds it.
 *
 * @param mnt_point The mount point to check
 *
 * @return 1 if the file is found, 0 otherwise.
 */
int dlt_logstorage_check_config_file(char *);

/** @brief Check if given mount point is writable
 *
 * @param mnt_point The mount point to check
 *
 * @return 1 if the file is writable, 0 otherwise.
 */
int dlt_logstorage_check_directory_permission(char *mnt_point);

#endif
