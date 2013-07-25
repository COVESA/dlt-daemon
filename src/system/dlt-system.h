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
 * \file dlt-system.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-system.h                                                  **
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

/*******************************************************************************
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  lm          Lassi Marttala             BMW                                **
*******************************************************************************/

#ifndef DLT_SYSTEM_H_
#define DLT_SYSTEM_H_

// DLT related includes.
#include "dlt.h"
#include "dlt_common.h"

// Constants
#define DEFAULT_CONF_FILE "/etc/dlt-system.conf"
#define DLT_SYSTEM_LOG_FILE_MAX 32
#define DLT_SYSTEM_LOG_DIRS_MAX 32
#define DLT_SYSTEM_LOG_PROCESSES_MAX 32

#define DLT_SYSTEM_MODE_OFF 0
#define DLT_SYSTEM_MODE_STARTUP 1
#define DLT_SYSTEM_MODE_REGULAR 2

#define MAX_LINE 1024

#define MAX_THREADS 8

// Macros
#define MALLOC_ASSERT(x) if(x == NULL) {\
	fprintf(stderr, "Out of memory\n");\
	abort();}

/**
 * Configuration structures.
 * Please see dlt-system.conf for explanation of all the options.
 */

// Command line options
typedef struct {
	char 	*ConfigurationFileName;
	int 	Daemonize;
} DltSystemCliOptions;

// Configuration shell options
typedef struct {
	int  	Enable;
} ShellOptions;

// Configuration syslog options
typedef struct {
	int  	Enable;
	char 	*ContextId;
	int  	Port;
} SyslogOptions;

// Configuration journal options
typedef struct {
	int  	Enable;
	char 	*ContextId;
	int 	CurrentBoot;
	int		Follow;
	int		MapLogLevels;
} JournalOptions;

typedef struct {
	int  Enable;
	char *ContextId;
	int  TimeStartup;
	int  TimeDelay;
	int  TimeoutBetweenLogs;
	char *TempDir;

	// Variable number of file transfer dirs
	int  Count;
	int  Compression[DLT_SYSTEM_LOG_DIRS_MAX];
	int  CompressionLevel[DLT_SYSTEM_LOG_DIRS_MAX];
	char *Directory[DLT_SYSTEM_LOG_DIRS_MAX];
} FiletransferOptions;

typedef struct {
	int  Enable;

	// Variable number of files to transfer
	int  Count;
	char *ContextId[DLT_SYSTEM_LOG_FILE_MAX];
	char *Filename[DLT_SYSTEM_LOG_FILE_MAX];
	int  Mode[DLT_SYSTEM_LOG_FILE_MAX];
	int  TimeDelay[DLT_SYSTEM_LOG_FILE_MAX];
} LogFileOptions;

typedef struct {
	int  Enable;
	char *ContextId;

	// Variable number of processes
	int  Count;
	char *Name[DLT_SYSTEM_LOG_PROCESSES_MAX];
	char *Filename[DLT_SYSTEM_LOG_PROCESSES_MAX];
	int  Mode[DLT_SYSTEM_LOG_PROCESSES_MAX];
	int  TimeDelay[DLT_SYSTEM_LOG_PROCESSES_MAX];
} LogProcessOptions;

typedef struct {
	char *ApplicationId;
	ShellOptions 			Shell;
	SyslogOptions 			Syslog;
	JournalOptions			Journal;
	FiletransferOptions 	Filetransfer;
	LogFileOptions 			LogFile;
	LogProcessOptions		LogProcesses;
} DltSystemConfiguration;

typedef struct {
	pthread_t threads[MAX_THREADS];
	int count;
	int shutdown;
} DltSystemThreads;

/**
 * Forward declarations for the whole application
 */

// In dlt-system-options.c
int read_command_line(DltSystemCliOptions *options, int argc, char *argv[]);
int read_configuration_file(DltSystemConfiguration *config, char *file_name);

// In dlt-process-handling.c
int daemonize();
void start_threads(DltSystemConfiguration *config);
void join_threads();
void dlt_system_signal_handler(int sig);
void register_with_dlt(DltSystemConfiguration *config);

// Thread initiators:
void init_shell();
void start_syslog();
void start_filetransfer(DltSystemConfiguration *conf);
void start_logfile(DltSystemConfiguration *conf);
void start_logprocess(DltSystemConfiguration *conf);

#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE)
void start_systemd_watchdog(DltSystemConfiguration *conf);
#endif

#if defined(DLT_SYSTEMD_JOURNAL_ENABLE)
void start_systemd_journal(DltSystemConfiguration *conf);
#endif

#endif /* DLT_SYSTEM_H_ */
