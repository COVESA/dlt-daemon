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
 * \author Christoph Lipka <clipka@jp.adit-jv.com> ADIT 2015
 * \author Frederic Berat <fberat@de.adit-jv.com> ADIT 2015
 *
 * \file dlt-logstorage-udev.c
 * For further information see http://www.covesa.org/.
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-logstorage-udev.c                                         **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Christoph Lipka clipka@jp.adit-jv.com                         **
**              Frederic Berat fberat@de.adit-jv.com                          **
**                                                                            **
**  PURPOSE   : For udev-based handling of any device                         **
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
**  cl          Christoph Lipka            ADIT                               **
**  fb          Frederic Berat             ADIT                               **
*******************************************************************************/

#define pr_fmt(fmt) "Udev control: "fmt

#include <libudev.h>
#include <errno.h>
#include <mntent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mount.h>

#include "dlt-control-common.h"
#include "dlt-logstorage-common.h"
#include "dlt-logstorage-list.h"
#include "dlt-logstorage-udev.h"

typedef struct {
    struct udev *udev;          /**< Udev instance */
    struct udev_monitor *mon;   /**< Udev monitor instance */
} LogstorageCtrlUdev;

/** @brief Get mount point of a device node
 *
 * This function search for the mount point in /proc/mounts
 * based on the device node.
 *
 * @param dev_node  Device node as string
 *
 * @return mount path or NULL on error
 */
static char *dlt_logstorage_udev_get_mount_point(char *dev_node)
{
    struct mntent *ent;
    FILE *f;
    char *mnt_point = NULL;

    if (dev_node == NULL)
        return NULL;

    f = setmntent("/proc/mounts", "r");

    if (f == NULL) {
        pr_error("Cannot read /proc/mounts\n");
        return NULL;
    }

    while (NULL != (ent = getmntent(f)))
        if (strncmp(ent->mnt_fsname, dev_node, strlen(ent->mnt_fsname)) == 0) {
            mnt_point = strdup(ent->mnt_dir);

            if (mnt_point == NULL) {
                pr_error("Cannot duplicate string.\n");
                return NULL;
            }

            /* Remounting rw */
            if (strlen(mnt_point))
                /* capabilities needed. Thus we don't really car on failure.
                 * Therefor we can ignore the return value.
                 */
                (void)mount(NULL, mnt_point, NULL, MS_REMOUNT, ent->mnt_opts);

            break;
        }

    endmntent(f);

    return mnt_point;
}

/** @brief Check if the daemon needs to be notified by the event
 *
 * On mount event:
 * If the device mount point contains the DLT configuration file,
 * the function will then send a message to the daemon.
 * On Unmounting event:
 * Check if the device was on the list, remove it and send the message
 * to the daemon.
 *
 * @param event The kind of event happening
 * @param part The device partition to be checked
 *
 * @return 0 on success, -1 if an error occured.
 */
static int check_mountpoint_from_partition(int event, struct udev_device *part)
{
    int logstorage_dev = 0;
    char *mnt_point = NULL;
    char *dev_node = NULL;
    int ret = 0;

    if (!part) {
        pr_verbose("No partition structure given.\n");
        return -1;
    }

    pr_verbose("Checking mount point.\n");

    if (!udev_device_get_devnode(part)) {
        pr_verbose("Skipping as no devnode.\n");
        return 0;
    }

    dev_node = strdup(udev_device_get_devnode(part));

    if (dev_node == NULL) {
        pr_error("Cannot allocate memory for to store string\n");
        return -1;
    }

    if (event == EVENT_MOUNTED) {
        mnt_point = dlt_logstorage_udev_get_mount_point(dev_node);
        logstorage_dev = dlt_logstorage_check_config_file(mnt_point);

        if (logstorage_dev) { /* Configuration file available, add node to internal list */
            logstorage_store_dev_info(dev_node, mnt_point);
        }
        else {
            free(mnt_point);
            mnt_point = NULL;
        }
    }
    else {
        /* remove device information */
        mnt_point = logstorage_delete_dev_info(dev_node);
    }

    if (mnt_point) {
        ret = dlt_logstorage_send_event(event, mnt_point);

        if (ret)
            pr_error("Can't send event for %s to DLT.\n", mnt_point);
    }

    free(dev_node);
    free(mnt_point);

    return 0;
}

/** @brief Handles the udev events
 *
 * On event, it finds the corresponding action, and calls
 * check_mountpoint_from_partition with the corresponding event.
 *
 * @return 0 on success, -1 on error.
 */
static int logstorage_udev_udevd_callback(void)
{
    const char *action;
    int ret = 0;
    DltLogstorageCtrl *lctrl = get_logstorage_control();
    LogstorageCtrlUdev *prvt = NULL;
    struct udev_device *partition = NULL;
    struct timespec ts;

    if (!lctrl) {
        pr_error("Not able to get logstorage control instance.\n");
        return -1;
    }

    prvt = (LogstorageCtrlUdev *)lctrl->prvt;

    if ((!prvt) || (!prvt->mon)) {
        pr_error("Not able to get private data.\n");
        return -1;
    }

    partition = udev_monitor_receive_device(prvt->mon);

    if (!partition) {
        pr_error("Not able to get partition.\n");
        return -1;
    }

    action = udev_device_get_action(partition);

    if (!action) {
        pr_error("Not able to get action.\n");
        udev_device_unref(partition);
        return -1;
    }

    pr_verbose("%s action received from udev for %s.\n",
               action,
               udev_device_get_devnode(partition));

    if (strncmp(action, "add", sizeof("add")) == 0) {
        /*TODO: This can be replaced by polling on /proc/mount.
         * we could get event on modification, and keep track on a list
         * of mounted devices. New devices could be check that way.
         * That also would solve the unmount event issue.
         * Then, udev is only interesting to simplify the check on new devices,
         * and/or for hot unplug (without unmount).
         */
        ts.tv_sec = 0;
        ts.tv_nsec = 500 * NANOSEC_PER_MILLISEC;
        nanosleep(&ts, NULL);
        ret = check_mountpoint_from_partition(EVENT_MOUNTED, partition);
    }
    else if (strncmp(action, "remove", sizeof("remove")) == 0)
    {
        ret = check_mountpoint_from_partition(EVENT_UNMOUNTING, partition);
    }

    udev_device_unref(partition);

    return ret;
}

/** @brief Check all partitions on the system to find configuration file
 *
 * The function looks for block devices that are of "partition" type.
 * Then, it gets the node, and call check_mountpoint_from_partition with it.
 *
 * @param udev The udev device used to find all the nodes
 *
 * @return 0 on success, -1 otherwise.
 */
static int dlt_logstorage_udev_check_mounted(struct udev *udev)
{
    if (udev == NULL) {
        pr_error("%s: udev structure is NULL\n", __func__);
        return -1;
    }

    /* Create a list of the devices in the 'partition' subsystem. */
    struct udev_enumerate *enumerate = udev_enumerate_new(udev);
    struct udev_list_entry *devices = NULL;
    struct udev_list_entry *dev_list_entry = NULL;

    if (!enumerate) {
        pr_error("Can't enumerate devices.\n");
        return -1;
    }

    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_add_match_property(enumerate, "DEVTYPE", "partition");
    udev_enumerate_scan_devices(enumerate);

    devices = udev_enumerate_get_list_entry(enumerate);

    /* For each list entry, get the corresponding device */
    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char *path;
        struct udev_device *partition = NULL;

        /* Get the filename of the /sys entry for the device
         * and create a udev_device object representing it
         */
        path = udev_list_entry_get_name(dev_list_entry);
        partition = udev_device_new_from_syspath(udev, path);

        if (!partition)
            continue;

        pr_verbose("Found device %s %s %s.\n",
                   path,
                   udev_device_get_devnode(partition),
                   udev_device_get_devtype(partition));

        /* Check the device and clean-up */
        check_mountpoint_from_partition(EVENT_MOUNTED, partition);
        udev_device_unref(partition);
    }

    /* Free the enumerator object */
    udev_enumerate_unref(enumerate);

    return 0;
}

/** @brief Clean-up the udev data
 *
 * That will destroy all the private data.
 *
 * @return 0 on success, -1 otherwise.
 */
int dlt_logstorage_udev_deinit(void)
{
    DltLogstorageCtrl *lctrl = get_logstorage_control();
    LogstorageCtrlUdev *prvt = NULL;

    if (!lctrl)
        return -1;

    prvt = (LogstorageCtrlUdev *)lctrl->prvt;

    if (prvt == NULL)
        return -1;

    if (prvt->mon)
        udev_monitor_unref(prvt->mon);

    if (prvt->udev)
        udev_unref(prvt->udev);

    free(prvt);
    lctrl->prvt = NULL;

    return 0;
}

/** @brief Initialize the private data
 *
 * That function will create the udev device and monitor.
 *
 * @return 0 on success, -1 otherwise.
 */
int dlt_logstorage_udev_init(void)
{
    int ret = 0;

    DltLogstorageCtrl *lctrl = get_logstorage_control();
    LogstorageCtrlUdev *prvt = NULL;

    pr_verbose("Initializing.\n");

    if (!lctrl) {
        pr_error("Not able to get logstorage control instance.\n");
        return -1;
    }

    lctrl->prvt = calloc(1, sizeof(LogstorageCtrlUdev));

    if (!lctrl->prvt) {
        pr_error("No memory to allocate private data.\n");
        return -1;
    }

    prvt = (LogstorageCtrlUdev *)lctrl->prvt;

    /* Initialize udev object */
    prvt->udev = udev_new();

    if (!prvt->udev) {
        pr_error("Cannot initialize udev object\n");
        dlt_logstorage_udev_deinit();
        return -1;
    }

    /* setup udev monitor which will report events when
     * devices attached to the system change. Events include
     * "add", "remove", "change", etc */
    prvt->mon = udev_monitor_new_from_netlink(prvt->udev, "udev");

    if (!prvt->mon) {
        pr_error("Cannot initialize udev monitor\n");
        dlt_logstorage_udev_deinit();
        return -1;
    }

    ret = udev_monitor_filter_add_match_subsystem_devtype(prvt->mon,
                                                          "block",
                                                          NULL);

    if (ret) {
        pr_error("Cannot attach filter to monitor: %s.\n", strerror(-ret));
        dlt_logstorage_udev_deinit();
        return -1;
    }

    ret = udev_monitor_enable_receiving(prvt->mon);

    if (ret < 0) {
        pr_error("Cannot start receiving: %s.\n", strerror(-ret));
        dlt_logstorage_udev_deinit();
        return -1;
    }

    /* get file descriptor */
    lctrl->fd = udev_monitor_get_fd(prvt->mon);
    /* set callback function */
    lctrl->callback = &logstorage_udev_udevd_callback;

    /* check if there is something already mounted */
    return dlt_logstorage_udev_check_mounted(prvt->udev);
}
