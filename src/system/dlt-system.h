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
 * \file dlt-system.h
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

#include <systemd/sd-journal.h>
#include <poll.h>

/* DLT related includes. */
#include "dlt.h"
#include "dlt_common.h"

/* Constants */
#define DEFAULT_CONF_FILE (CONFIGURATION_FILES_DIR "/dlt-system.conf")
#define DLT_SYSTEM_LOG_FILE_MAX 32
#define DLT_SYSTEM_LOG_DIRS_MAX 32
#define DLT_SYSTEM_LOG_PROCESSES_MAX 32

#define DLT_SYSTEM_MODE_OFF 0
#define DLT_SYSTEM_MODE_STARTUP 1
#define DLT_SYSTEM_MODE_REGULAR 2

#define MAX_LINE 1024

/** Total number of file descriptors needed for processing all features:
*   - Syslog file descriptor
*   - Timer file descriptor for processing LogFile and LogProcesses every second
*   - Inotify file descriptor for FileTransfer
*   - Timer file descriptor for Watchdog 
*/
#define MAX_FD_NUMBER   4

/* Macros */
#define MALLOC_ASSERT(x) if (x == NULL) { \
        fprintf(stderr, "Out of memory\n"); \
        abort(); }

/* enum for classification of FD */
enum fdType {
    fdType_syslog = 0,
    fdType_filetransfer,
    fdType_timer,
    fdType_watchdog,
};

/**
 * Configuration structures.
 * Please see dlt-system.conf for explanation of all the options.
 */

/* Command line options */
typedef struct {
    char *ConfigurationFileName;
    int freeConfigFileName;
    int Daemonize;
} DltSystemCliOptions;

/* Configuration shell options */
typedef struct {
    int Enable;
} ShellOptions;

/* Configuration syslog options */
typedef struct {
    int Enable;
    char ContextId[DLT_ID_SIZE];
    int Port;
} SyslogOptions;

/* Configuration journal options */
typedef struct {
    int Enable;
    char ContextId[DLT_ID_SIZE];
    int CurrentBoot;
    int Follow;
    int MapLogLevels;
    int UseOriginalTimestamp;
#ifdef DLT_SYSTEMD_WATCHDOG_ENFORCE_MSG_RX_ENABLE_DLT_SYSTEM
    int MessageReceived;
#endif
} JournalOptions;

typedef struct {
    int Enable;
    char ContextId[DLT_ID_SIZE];
    int TimeStartup;
    int TimeoutBetweenLogs;

    /* Variable number of file transfer dirs */
    int Count;
    int Compression[DLT_SYSTEM_LOG_DIRS_MAX];
    int CompressionLevel[DLT_SYSTEM_LOG_DIRS_MAX];
    char *Directory[DLT_SYSTEM_LOG_DIRS_MAX];
} FiletransferOptions;

typedef struct {
    int handle;
    int fd[DLT_SYSTEM_LOG_DIRS_MAX];
} s_ft_inotify;

typedef struct {
    int Enable;

    /* Variable number of files to transfer */
    int Count;
    char ContextId[DLT_SYSTEM_LOG_FILE_MAX][DLT_ID_SIZE];
    char *Filename[DLT_SYSTEM_LOG_FILE_MAX];
    int Mode[DLT_SYSTEM_LOG_FILE_MAX];
    int TimeDelay[DLT_SYSTEM_LOG_FILE_MAX];
} LogFileOptions;

typedef struct {
    int Enable;
    char ContextId[DLT_ID_SIZE];

    /* Variable number of processes */
    int Count;
    char *Name[DLT_SYSTEM_LOG_PROCESSES_MAX];
    char *Filename[DLT_SYSTEM_LOG_PROCESSES_MAX];
    int Mode[DLT_SYSTEM_LOG_PROCESSES_MAX];
    int TimeDelay[DLT_SYSTEM_LOG_PROCESSES_MAX];
} LogProcessOptions;

typedef struct {
    char ApplicationId[DLT_ID_SIZE];
    ShellOptions Shell;
    SyslogOptions Syslog;
    JournalOptions Journal;
    FiletransferOptions Filetransfer;
    LogFileOptions LogFile;
    LogProcessOptions LogProcesses;
} DltSystemConfiguration;

/**
 * Forward declarations for the whole application
 */

/* In dlt-system-options.c */
int read_command_line(DltSystemCliOptions *options, int argc, char *argv[]);
int read_configuration_file(DltSystemConfiguration *config, char *file_name);
void cleanup_config(DltSystemConfiguration *config, DltSystemCliOptions *options);

/* For dlt-process-handling.c */
int daemonize();
void init_shell();
void dlt_system_signal_handler(int sig);

/* Main function for creating/registering all needed file descriptors and starting the poll for all of them. */
void start_dlt_system_processes(DltSystemConfiguration *config);

/* Init process, create file descriptors and register them into main pollfd. */
int register_watchdog_fd(struct pollfd *pollfd, int fdcnt);
int init_filetransfer_dirs(DltSystemConfiguration *config);
void logfile_init(void *v_conf);
void logprocess_init(void *v_conf);
void register_journal_fd(sd_journal **j, struct pollfd *pollfd, int i,  DltSystemConfiguration *config);
int register_syslog_fd(struct pollfd *pollfd, int i, DltSystemConfiguration *config);

/* Routines that are called, when a fd event was raised. */
void logfile_fd_handler(void *v_conf);
void logprocess_fd_handler(void *v_conf);
void filetransfer_fd_handler(DltSystemConfiguration *config);
#if defined(DLT_SYSTEMD_WATCHDOG_ENFORCE_MSG_RX_ENABLE_DLT_SYSTEM) && defined(DLT_SYSTEMD_JOURNAL_ENABLE)
void watchdog_fd_handler(int fd, int* received_message_since_last_watchdog_interval);
#else
void watchdog_fd_handler(int fd);
#endif
void journal_fd_handler(sd_journal *j, DltSystemConfiguration *config);
struct journal_fd_params {
    volatile uint8_t* quit;
    struct pollfd* journalPollFd;
    sd_journal *j;
    DltSystemConfiguration *config;
};
void *journal_thread(void* journalParams);
void syslog_fd_handler(int syslogSock);

#endif /* DLT_SYSTEM_H_ */
