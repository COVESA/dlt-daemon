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
 * \author Anitha.BA <anithaammaji.baggam@in.bosch.com> ADIT 2015
 * \author Christoph Lipka <clipka@jp.adit-jv.com> ADIT 2015
 * \author Frederic Berat <fberat@de.adit-jv.com> ADIT 2015
 *
 * \file dlt-logstorage-list.c
 * For further information see http://www.covesa.org/.
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-logstorage-list.c                                         **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Christoph Lipka clipka@jp.adit-jv.com                         **
**              Anitha.B.A  anithaammaji.baggam@in.bosch.com                  **
**              Frederic Berat fberat@de.adit-jv.com                          **
**                                                                            **
**  PURPOSE   :  linked list implementation for storing the device info       **
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
**  BA          Anitha                     ADIT                               **
**  cl          Christoph Lipka            ADIT                               **
**  fb          Frederic Berat             ADIT                               **
*******************************************************************************/
#define pr_fmt(fmt) "Log storage list: "fmt

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dlt_common.h"
#include "dlt-control-common.h"
#include "dlt-logstorage-common.h"

static struct LogstorageDeviceInfo
{
    char *dev_node; /**< The device node */
    char *mnt_point; /**< Mount point for this device */
    struct LogstorageDeviceInfo *prev; /**< Previous element of the list */
    struct LogstorageDeviceInfo *next; /**< Next element of the list */
} *g_info;

/** @brief Prints the device list in verbose mode
 *
 * This can be used to debug the behavior.
 * Therefore, it's only available in verbose mode.
 */
void print_list()
{
    struct LogstorageDeviceInfo *ptr = g_info;
    pr_verbose(" -------Device list-------\n");

    while (ptr != NULL) {
        pr_verbose("%p:\t[%s][%s] \n", ptr, ptr->dev_node, ptr->mnt_point);
        ptr = ptr->next;
    }

    pr_verbose(" -------Device list end-------\n\n");

    return;
}

/** @brief Find element in the list based on device node
 *
 * Allows to check whether a device is already in the list or
 * to find out the one to be removed.
 *
 * @param node The device node to look for
 *
 * @return The element of the list found, NULL either.
 */
static struct LogstorageDeviceInfo *logstorage_find_dev_info(const char *node)
{
    struct LogstorageDeviceInfo *ptr = g_info;

    if (!node)
        return NULL;

    pr_verbose("Looking for %s.\n", node);

    while (ptr != NULL) {
        if (strncmp(ptr->dev_node, node, DLT_MOUNT_PATH_MAX) == 0) {
            pr_verbose("%s found in %p.\n", node, ptr);
            break;
        }
        else {
            ptr = ptr->next;
        }
    }

    return ptr;
}

/** @brief Add new device in the list
 *
 * The device is only added if a configuration file has been found and
 * if it's not already in the list.
 *
 * @param node The device node to add
 * @param path The corresponding mount point path
 *
 * @return 0 on success, -1 in case of error.
 */
int logstorage_store_dev_info(const char *node, const char *path)
{
    struct LogstorageDeviceInfo *ptr = NULL;
    size_t path_len = 0;

    if ((node == NULL) || (path == NULL)) {
        pr_error("Invalid input\n");
        return -1;
    }

    if (logstorage_find_dev_info(node)) {
        pr_verbose("%s already in list.\n", node);
        print_list();
        return 0;
    }

    ptr = calloc(1, sizeof(struct LogstorageDeviceInfo));

    if (ptr == NULL) {
        pr_error("Node creation failed\n");
        return -1;
    }

    ptr->dev_node = strdup(node);
    path_len = strlen(path);

    if (path_len > DLT_MOUNT_PATH_MAX)
        path_len = (size_t)DLT_MOUNT_PATH_MAX;

    ptr->mnt_point = (char *)calloc(1, path_len + 1);

    if (ptr->mnt_point == NULL) {
        pr_error("memory allocation failed for mnt_point\n");
        free(ptr);
        ptr = NULL;
        return -1;
    }

    ptr->mnt_point[path_len] = '\0';
    memcpy(ptr->mnt_point, path, path_len);

    /* Put it on head */
    ptr->next = g_info;

    if (g_info)
        g_info->prev = ptr;

    g_info = ptr;

    pr_verbose("%s added to list.\n", node);
    print_list();

    return 0;
}

/** @brief Remove a device from the list
 *
 * If the device is removed from the list, the mount point
 * pointer is given back to the caller. That means that
 * he has to free it.
 *
 * @param node The device node to be removed
 *
 * @return the mount point if the node is found, NULL either.
 */
char *logstorage_delete_dev_info(const char *node)
{
    struct LogstorageDeviceInfo *del = NULL;
    char *ret = NULL;

    del = logstorage_find_dev_info(node);

    if (del == NULL) {
        pr_verbose("%s not found in list.\n", node);
        print_list();
        return ret;
    }

    /* Has to be freed by the caller */
    ret = del->mnt_point;

    if (del->prev)
        del->prev->next = del->next;

    if (del->next)
        del->next->prev = del->prev;

    if (del == g_info)
        g_info = g_info->next;

    free(del->dev_node);
    free(del);

    pr_verbose("%s removed from list.\n", node);
    print_list();

    return ret;
}
