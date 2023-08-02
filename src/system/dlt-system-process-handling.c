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
 * \author Lassi Marttala <lassi.lm.marttala@partner.bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-system-process-handling.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-systemprocess-handling.c                                                  **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Lassi Marttala <lassi.lm.marttala@partner.bmw.de>             **
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
#include "dlt.h"

#include "dlt-system.h"

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <limits.h>

#include <systemd/sd-journal.h>
#include <poll.h>
#include <sys/timerfd.h>
#include <time.h>
#include <errno.h>

DLT_IMPORT_CONTEXT(dltsystem)
DLT_IMPORT_CONTEXT(syslogContext)
DLT_IMPORT_CONTEXT(journalContext)
DLT_IMPORT_CONTEXT(watchdogContext)
DLT_IMPORT_CONTEXT(procContext)
DLT_IMPORT_CONTEXT(filetransferContext)
extern DltContext logfileContext[DLT_SYSTEM_LOG_FILE_MAX];

volatile uint8_t quit = 0;

#if defined(DLT_FILETRANSFER_ENABLE)
extern s_ft_inotify ino;
#endif

int daemonize()
{
    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-process-handling, daemonize"));

    /* Fork new process */
    int f = fork();

    if (f < 0)
        return f;

    if (f > 0)
        exit(0);

    /* Create a new process group */
    if (setsid() < 0)
        return -1;

    /**
     *  Close all file descriptors and point
     *  stdin, stdout and stderr to /dev/null
     */
    int i;

    for (i = getdtablesize(); i >= 0; i--)
        close(i);

    int fd = open("/dev/null", O_RDWR);

    if (fd < 0)
        return -1;

    if ((dup(fd) < 0) || (dup(fd) < 0)) {
        close(fd);
        return -1;
    }

    /**
     * Ignore signals related to child processes and
     * terminal handling.
     */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    /*no close(fd); - we just intentionally pointed stdx to null! tbd: set ignore for coverity */
    return 0;
}

/* Unregisters all DLT Contexts and closes all file descriptors */
void cleanup_processes(struct pollfd *pollfd, struct pollfd *journalPollFd, sd_journal *j, DltSystemConfiguration *config)
{
    //Syslog cleanup
    if (config->Syslog.Enable)
        DLT_UNREGISTER_CONTEXT(syslogContext);

    //Journal cleanup
#if defined(DLT_SYSTEMD_JOURNAL_ENABLE)
    if (config->Journal.Enable)
        DLT_UNREGISTER_CONTEXT(journalContext);
    if(j != NULL)
        sd_journal_close(j);

    if(journalPollFd->fd > 0)
        close(journalPollFd->fd);
#else
    // silence warnings
    (void)j;
    (void)journalPollFd->fd;
#endif

    //Logfile cleanup
    if (config->LogFile.Enable) {
        for (int i = 0; i < config->LogFile.Count; i++)
            DLT_UNREGISTER_CONTEXT(logfileContext[i]);
    }

    //LogProcess cleanup 
    if (config->LogProcesses.Enable) {
        DLT_UNREGISTER_CONTEXT(procContext);
    }

    //Watchdog cleanup
#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE)
    DLT_UNREGISTER_CONTEXT(watchdogContext);
#endif

    //FileTransfer cleanup
#if defined(DLT_FILETRANSFER_ENABLE)
    if (config->Filetransfer.Enable) {
        DLT_UNREGISTER_CONTEXT(filetransferContext);
    }
#endif

    for (int i = 0; i < MAX_FD_NUMBER; i++) {
        if(pollfd[i].fd > 0)
            close(pollfd[i].fd);
    }
}

/* Creates timer for LogFile and LogProcess, that need to be called every second. */
int register_timer_fd(struct pollfd *pollfd, int fdcnt)
{
    struct itimerspec timerValue;
    memset(&timerValue, '\0', sizeof(timerValue));
    timerValue.it_value.tv_sec = 1;
    timerValue.it_value.tv_nsec = 0;
    timerValue.it_interval.tv_sec = 1;
    timerValue.it_interval.tv_nsec = 0;

    int timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timerfd < 0) {
        DLT_LOG(dltsystem, DLT_LOG_ERROR, DLT_STRING("Failed to create timer fd"));
        return -1;
    }
    pollfd[fdcnt].fd = timerfd;
    pollfd[fdcnt].events = POLLIN;

    if (timerfd_settime(timerfd, 0, &timerValue, NULL) < 0) {   // init timer with 1 second
        DLT_LOG(dltsystem, DLT_LOG_ERROR, DLT_STRING("Could not start timer"));
        return -1;
    }
    return 0;
}

/* Routine for executing LogProcess and LogFile, when timer expires */
void timer_fd_handler(int fd, DltSystemConfiguration *config)
{
    uint64_t timersElapsed = 0ULL;
    int r = read(fd, &timersElapsed, 8U);    // only needed to reset fd event
    if (r < 0) 
        DLT_LOG(dltsystem, DLT_LOG_ERROR, DLT_STRING("Error while reading timer fd: "), 
            DLT_STRING(strerror(r)));

    if(config->LogProcesses.Enable)
        logprocess_fd_handler(config);
    if(config->LogFile.Enable)
        logfile_fd_handler(config);
}

void start_dlt_system_processes(DltSystemConfiguration *config)
{
    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-process-handling, start threads"));

    if (config->Shell.Enable)
        init_shell();

    int fdcnt = 0;

    /* Init FDs for all activated processes*/
    struct pollfd pollfd[MAX_FD_NUMBER];    // Holds all FDs and events
    uint8_t fdType[MAX_FD_NUMBER];          // Holds corresponding enum for process identification

    for(int cnt = 0 ; cnt < MAX_FD_NUMBER ; cnt++) {
        pollfd[cnt].fd = 0;
        pollfd[cnt].events = 0;
    }

    //init FD for LogFile and LogProcesses
    if (config->LogProcesses.Enable || config->LogFile.Enable) {
        fdType[fdcnt] = fdType_timer;
        if (register_timer_fd(pollfd, fdcnt) == 0) {
            if(config->LogProcesses.Enable)
                logprocess_init(config);
            if(config->LogFile.Enable)
                logfile_init(config);
            fdcnt++;
        }
    }

    //init FD for Syslog
    int syslogSock = 0;
    if (config->Syslog.Enable) {
        fdType[fdcnt] = fdType_syslog;
        syslogSock = register_syslog_fd(pollfd, fdcnt, config);
        fdcnt++;
    }

    //init FD for Journal
    sd_journal *j = NULL;
    struct pollfd journalPollFd;
#if defined(DLT_SYSTEMD_JOURNAL_ENABLE)
    pthread_t journalThreadHandle;
    if (config->Journal.Enable) {
        register_journal_fd(&j, &journalPollFd, 0, config);
        struct journal_fd_params params = {
                &quit,
                &journalPollFd,
                j,
                config
        };
        if (pthread_create(&journalThreadHandle, NULL, &journal_thread, (void*)&params) != 0) {
            DLT_LOG(dltsystem, DLT_LOG_ERROR, DLT_STRING("Failed to create journal_thread thread "),
                    DLT_STRING(strerror(errno)));
        }
    }
#endif

    //init FD for FileTransfer
#if defined(DLT_FILETRANSFER_ENABLE)
    if (config->Filetransfer.Enable) {
        init_filetransfer_dirs(config);
        pollfd[fdcnt].fd = ino.handle;
        pollfd[fdcnt].events = POLLIN;
        fdType[fdcnt] = fdType_filetransfer;
        fdcnt++;
    }
#endif

    //init FD for Watchdog
#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE)
    fdType[fdcnt] = fdType_watchdog;
    register_watchdog_fd(pollfd, fdcnt);
#endif

    while (quit == 0)
    {
        int ready;
        ready = poll(pollfd, MAX_FD_NUMBER, -1);
        if (ready == -1 && quit == 0)
            DLT_LOG(dltsystem, DLT_LOG_ERROR, DLT_STRING("Error while poll. Exit with: "), 
                DLT_STRING(strerror(ready)));

        for (int i = 0; i < MAX_FD_NUMBER; i++) {
            if(pollfd[i].revents & POLLIN){
                if (fdType[i] == fdType_syslog && syslogSock > 0) {
                    syslog_fd_handler(syslogSock);
                }
                else if (fdType[i] == fdType_timer) {
                    timer_fd_handler(pollfd[i].fd, config);
                }
                #if defined(DLT_FILETRANSFER_ENABLE)
                else if (fdType[i] == fdType_filetransfer) {
                    filetransfer_fd_handler(config);
                }
                #endif
                #if defined(DLT_SYSTEMD_WATCHDOG_ENABLE)
                else if (fdType[i] == fdType_watchdog) {
                    #if defined(DLT_SYSTEMD_WATCHDOG_ENFORCE_MSG_RX_ENABLE_DLT_SYSTEM) && defined(DLT_SYSTEMD_JOURNAL_ENABLE)
                    watchdog_fd_handler(pollfd[i].fd, &config->Journal.MessageReceived);
                    #else
                    watchdog_fd_handler(pollfd[i].fd);
                    #endif
                }
                #endif
            }
        }
    }

#if defined(DLT_SYSTEMD_JOURNAL_ENABLE)
    if (config->Journal.Enable) {
        pthread_join(journalThreadHandle, NULL);
    }
#endif
    cleanup_processes(pollfd, &journalPollFd, j, config);
}

void dlt_system_signal_handler(int sig)
{
    switch (sig) {
    case SIGHUP:
    case SIGTERM:
    case SIGINT:
    case SIGQUIT:
        quit = 1;
        break;
    default:
        break;
    }
}
