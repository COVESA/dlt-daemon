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
#include <string.h>
#include <sys/timerfd.h>
#include "dlt.h"
#include "dlt-system.h"
#include <poll.h>
#include <systemd/sd-daemon.h>

DLT_DECLARE_CONTEXT(watchdogContext)
DLT_IMPORT_CONTEXT(dltsystem)

int calculate_period(struct itimerspec *itval)
{
    unsigned int ns;
    unsigned int sec;
    char str[512];
    char *watchdogUSec;
    unsigned int watchdogTimeoutSeconds;
    unsigned int notifiyPeriodNSec;

    watchdogUSec = getenv("WATCHDOG_USEC");

    if (watchdogUSec == NULL) {
        DLT_LOG(watchdogContext, DLT_LOG_ERROR, DLT_STRING("systemd watchdog timeout (WATCHDOG_USEC) is null\n"));
        return -1;
    }
    
    DLT_LOG(watchdogContext, DLT_LOG_DEBUG, DLT_STRING("watchdogusec: "), DLT_STRING(watchdogUSec));
    watchdogTimeoutSeconds = atoi(watchdogUSec);

    if (watchdogTimeoutSeconds <= 0) {
        snprintf(str, 512, "systemd watchdog timeout incorrect: %u\n", watchdogTimeoutSeconds);
        DLT_LOG(watchdogContext, DLT_LOG_ERROR, DLT_STRING(str));
        return -1;
    }

    /* Calculate half of WATCHDOG_USEC in ns for timer tick */
    notifiyPeriodNSec = watchdogTimeoutSeconds / 2;

    snprintf(str,
            512,
            "systemd watchdog timeout: %u nsec - timer will be initialized: %u nsec\n",
            watchdogTimeoutSeconds,
            notifiyPeriodNSec);
    DLT_LOG(watchdogContext, DLT_LOG_DEBUG, DLT_STRING(str));

    /* Make the timer periodic */
    sec = notifiyPeriodNSec / 1000000;
    ns = (notifiyPeriodNSec - (sec * 1000000)) * 1000;
    itval->it_interval.tv_sec = sec;
    itval->it_interval.tv_nsec = ns;
    itval->it_value.tv_sec = sec;
    itval->it_value.tv_nsec = ns;

    return 0;
}

int register_watchdog_fd(struct pollfd *pollfd, int fdcnt)
{
    DLT_REGISTER_CONTEXT(watchdogContext, "DOG", "dlt system watchdog context.");
    struct itimerspec timerValue;
    if (calculate_period(&timerValue) != 0)
        return -1;

    int timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timerfd < 0) {
        DLT_LOG(dltsystem, DLT_LOG_ERROR, DLT_STRING("Failed to create timer fd\n"));
        return -1;
    }
    pollfd[fdcnt].fd = timerfd;
    pollfd[fdcnt].events = POLLIN;

    if (timerfd_settime(timerfd, 0, &timerValue, NULL) < 0) {
        DLT_LOG(dltsystem, DLT_LOG_ERROR, DLT_STRING("Could not start timer\n"));
        return -1;
    }
    return 0;
}

#if defined(DLT_SYSTEMD_WATCHDOG_ENFORCE_MSG_RX_ENABLE_DLT_SYSTEM) && defined(DLT_SYSTEMD_JOURNAL_ENABLE)
void watchdog_fd_handler(int fd, int* received_message_since_last_watchdog_interval)
#else
void watchdog_fd_handler(int fd)
#endif
{
    uint64_t timersElapsed = 0ULL;
    int r = read(fd, &timersElapsed, 8U);    // only needed to reset fd event
    if(r < 0)
        DLT_LOG(watchdogContext, DLT_LOG_ERROR, DLT_STRING("Could not reset systemd watchdog. Exit with: "), 
            DLT_STRING(strerror(r)));

    #ifdef DLT_SYSTEMD_WATCHDOG_ENFORCE_MSG_RX_ENABLE_DLT_SYSTEM
    if (!*received_message_since_last_watchdog_interval) {
      dlt_log(LOG_WARNING, "No new messages received since last watchdog timer run\n");
      return;
    }
    *received_message_since_last_watchdog_interval = 0;
    #endif

    if (sd_notify(0, "WATCHDOG=1") < 0)
        DLT_LOG(watchdogContext, DLT_LOG_ERROR, DLT_STRING("Could not reset systemd watchdog\n"));

    DLT_LOG(watchdogContext, DLT_LOG_DEBUG, DLT_STRING("systemd watchdog waited periodic\n"));
}
#endif
