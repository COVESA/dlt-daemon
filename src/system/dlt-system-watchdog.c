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
 * Lassi Marttala <lassi.lm.marttala@partner.bmw.de>
 * Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 * Markus Klein <Markus.Klein@esk.fraunhofer.de>
 * Mikko Rapeli <mikko.rapeli@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-system-watchdog.c
 */

#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE)
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/timerfd.h>
#include "dlt.h"
#include "dlt-system.h"

#include "sd-daemon.h"


DLT_DECLARE_CONTEXT(watchdogContext)
DLT_IMPORT_CONTEXT(dltsystem)

extern DltSystemThreads threads;

typedef struct
{
    int timer_fd;
    unsigned long long wakeups_missed;
} PeriodicData;

void wait_period (PeriodicData *info)
{
    unsigned long long missed;

    if (read (info->timer_fd, &missed, sizeof (missed)) < 0)
        DLT_LOG(watchdogContext, DLT_LOG_ERROR,
                DLT_STRING("Could not read from timer file descriptor in watchdog.\n"));

    if (missed > 0)
        info->wakeups_missed += (missed - 1);
}

int make_periodic(unsigned int period, PeriodicData *info)
{
    unsigned int ns;
    unsigned int sec;
    int fd;
    struct itimerspec itval;

    if (info == 0) {
        DLT_LOG(watchdogContext, DLT_LOG_ERROR,
                DLT_STRING("Invalid function parameters used for function make_periodic.\n"));
        return -1;
    }

    /* Create the timer */
    fd = timerfd_create (CLOCK_MONOTONIC, 0);

    info->wakeups_missed = 0;
    info->timer_fd = fd;

    if (fd == -1) {
        DLT_LOG(watchdogContext, DLT_LOG_ERROR,
                DLT_STRING("Can't create timer filedescriptor.\n"));
        return -1;
    }

    /* Make the timer periodic */
    sec = period / 1000000;
    ns = (period - (sec * 1000000)) * 1000;
    itval.it_interval.tv_sec = sec;
    itval.it_interval.tv_nsec = ns;
    itval.it_value.tv_sec = sec;
    itval.it_value.tv_nsec = ns;

    return timerfd_settime (fd, 0, &itval, NULL);
}


void watchdog_thread(void *v_conf)
{
    char str[512];
    char *watchdogUSec;
    unsigned int watchdogTimeoutSeconds;
    unsigned int notifiyPeriodNSec;
    PeriodicData info;

    DLT_REGISTER_CONTEXT(watchdogContext, "DOG", "dlt system watchdog context.");

    sleep(1);

    DLT_LOG(watchdogContext, DLT_LOG_INFO, DLT_STRING("Watchdog thread started.\n"));

    if (v_conf == 0) {
        DLT_LOG(watchdogContext, DLT_LOG_ERROR,
                DLT_STRING("Invalid function parameters used for function watchdog_thread.\n"));
        return;
    }

    watchdogUSec = getenv("WATCHDOG_USEC");

    if (watchdogUSec) {
        DLT_LOG(watchdogContext, DLT_LOG_DEBUG, DLT_STRING("watchdogusec: "), DLT_STRING(watchdogUSec));

        watchdogTimeoutSeconds = atoi(watchdogUSec);

        if (watchdogTimeoutSeconds > 0) {

            /* Calculate half of WATCHDOG_USEC in ns for timer tick */
            notifiyPeriodNSec = watchdogTimeoutSeconds / 2;

            snprintf(str,
                     512,
                     "systemd watchdog timeout: %u nsec - timer will be initialized: %u nsec\n",
                     watchdogTimeoutSeconds,
                     notifiyPeriodNSec);
            DLT_LOG(watchdogContext, DLT_LOG_DEBUG, DLT_STRING(str));

            if (make_periodic (notifiyPeriodNSec, &info) < 0) {
                DLT_LOG(watchdogContext, DLT_LOG_ERROR, DLT_STRING("Could not initialize systemd watchdog timer\n"));
                return;
            }

            while (1) {
                if (sd_notify(0, "WATCHDOG=1") < 0)
                    DLT_LOG(watchdogContext, DLT_LOG_ERROR, DLT_STRING("Could not reset systemd watchdog\n"));

                DLT_LOG(watchdogContext, DLT_LOG_DEBUG, DLT_STRING("systemd watchdog waited periodic\n"));

                /* Wait for next period */
                wait_period(&info);
            }
        }
        else {
            snprintf(str, 512, "systemd watchdog timeout incorrect: %u\n", watchdogTimeoutSeconds);
            DLT_LOG(watchdogContext, DLT_LOG_DEBUG, DLT_STRING(str));
        }
    }
    else {
        DLT_LOG(watchdogContext, DLT_LOG_ERROR, DLT_STRING("systemd watchdog timeout (WATCHDOG_USEC) is null\n"));
    }
}
#endif
