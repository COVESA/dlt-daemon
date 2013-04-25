/**
 * @licence app begin@
 * Copyright (C) 2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt - Diagnostic Log and Trace console apps.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Lassi Marttala <lassi.lm.marttala@partner.bmw.de> BMW 2012
 *
 * \file dlt-system-process-handling.c
 * For further information see http://www.genivi.org/.
 * @licence end@
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
#include <pthread.h>
#include <limits.h>

volatile DltSystemThreads threads;

DLT_IMPORT_CONTEXT(dltsystem);

int daemonize()
{
	DLT_LOG(dltsystem,DLT_LOG_DEBUG,
			DLT_STRING("dlt-system-process-handling, daemonize"));

	// Fork new process
	int f = fork();

	if(f < 0)
		return f;
	if(f > 0)
		exit(0);

	// Create a new process group
	if(setsid() < 0)
		return -1;

	/**
	 *  Close all file descriptors and point
	 *  stdin, stdout and stderr to /dev/null
	 */
	int i;
    for(i = getdtablesize(); i >= 0; i--)
        close(i);

	int fd = open("/dev/null",O_RDWR);
    if(fd < 0)
    {
        return -1;
    }

    if(dup(fd) < 0 || dup(fd) < 0)
    {
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
    //no close(fd); - we just intentionally pointed stdx to null! tbd: set ignore for coverity
	return 0;
}

void start_threads(DltSystemConfiguration *config)
{
	DLT_LOG(dltsystem,DLT_LOG_DEBUG,
			DLT_STRING("dlt-system-process-handling, start threads"));

	int i;
	threads.count = 0;
	threads.shutdown = 0;
	for(i = 0;i < MAX_THREADS; i++)
	{
		threads.threads[i] = 0;
	}

#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE)
	start_systemd_watchdog(config);
#endif

	init_shell();

	if(config->LogFile.Enable)
		start_logfile(config);

	if(config->Filetransfer.Enable)
		start_filetransfer(config);

	if(config->LogProcesses.Enable)
		start_logprocess(config);

	if(config->Syslog.Enable)
		start_syslog(config);
}

/**
 * Wait for threads to exit.
 * There's not actually a condition currently
 * to bail out of file transfer without a signal.
 */
void join_threads()
{
	int i;
	DLT_LOG(dltsystem, DLT_LOG_DEBUG,
			DLT_STRING("dlt-system-process-handling, waiting for threads to exit."));

	if(threads.count < 1)
	{
		DLT_LOG(dltsystem, DLT_LOG_DEBUG,
				DLT_STRING("dlt-system-process-handling, no threads, waiting for signal."));
		sleep(UINT_MAX);
	}
	else
	{
		DLT_LOG(dltsystem,DLT_LOG_DEBUG,
				DLT_STRING("dlt-system-process-handling, thread count: "),
				DLT_INT(threads.count));

		for (i = 0; i < threads.count; i++)
		{
			pthread_join(threads.threads[i], NULL);
			DLT_LOG(dltsystem,DLT_LOG_DEBUG,
					DLT_STRING("dlt-system-process-handling, thread exit: "),
					DLT_INT(threads.threads[i]));
		}
	}
}

void dlt_system_signal_handler(int sig)
{
	DLT_LOG(dltsystem,DLT_LOG_DEBUG,
			DLT_STRING("dlt-system-process-handling, signal handler"));

    switch (sig)
    {
	case SIGHUP:
	case SIGTERM:
	case SIGINT:
	case SIGQUIT:
		DLT_LOG(dltsystem,DLT_LOG_DEBUG,
				DLT_STRING("dlt-system-process-handling, exit, signal: "),
				DLT_INT(sig));
		exit(0);
		break;
	default:
		DLT_LOG(dltsystem,DLT_LOG_WARN,
				DLT_STRING("dlt-system-process-handling, unknown signal!"));
		break;
	}
}

