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
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> BMW 2011-2012
 *
 * \file dlt-daemon.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-daemon.h                                                  **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
**              Markus Klein                                                  **
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
**  aw          Alexander Wenzel           BMW                                **
**  mk          Markus Klein               Fraunhofer ESK                     **
*******************************************************************************/

/*******************************************************************************
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  aw          Alexander Wenzel           BMW                                **
**  mk          Markus Klein               Fraunhofer ESK                     **
*******************************************************************************/

#ifndef DLT_DAEMON_H
#define DLT_DAEMON_H

#include "dlt_daemon_common.h"
#include "dlt_user_shared.h"
#include "dlt_user_shared_cfg.h"

#include <dlt_offline_trace.h>

/**
 * The flags of a dlt daemon.
 */
typedef struct
{
    int aflag;      /**< (Boolean) Print DLT messages; payload as ASCII */
    int sflag;      /**< (Boolean) Print DLT messages; payload as hex */
    int xflag;      /**< (Boolean) Print DLT messages; only headers */
    int vflag;      /**< (Boolean) Verbose mode */
    int dflag;      /**< (Boolean) Daemonize */
    int lflag;      /**< (Boolean) Send DLT messages with serial header */
    int rflag;      /**< (Boolean) Send automatic get log info response during context registration */
    int mflag;      /**< (Boolean) Sync to serial header on serial connection */
    int nflag;      /**< (Boolean) Sync to serial header on all TCP connections */
    char evalue[256];   /**< (String: ECU ID) Set ECU ID (Default: ECU1) */
    char bvalue[256];   /**< (String: Baudrate) Serial device baudrate (Default: 115200) */
    char yvalue[256];   /**< (String: Devicename) Additional support for serial device */
    char ivalue[256];   /**< (String: Directory) Directory where to store the persistant configuration (Default: /tmp) */
    char cvalue[256];   /**< (String: Directory) Filename of DLT configuration file (Default: /etc/dlt.conf) */
    int  sharedMemorySize;	   /**< (int) Size of shared memory (Default: 100000) */
    int  sendMessageTime;	   /**< (Boolean) Send periodic Message Time if client is connected (Default: 0) */
    char offlineTraceDirectory[256]; /**< (String: Directory) Store DLT messages to local directory (Default: /etc/dlt.conf) */
    int  offlineTraceFileSize;	/**< (int) Maximum size in bytes of one trace file (Default: 1000000) */
    int  offlineTraceMaxSize;	/**< (int) Maximum size of all trace files (Default: 4000000) */
    int  loggingMode;	/**< (int) The logging console for internal logging of dlt-daemon (Default: 0) */
    int  loggingLevel;	/**< (int) The logging level for internal logging of dlt-daemon (Default: 6) */
    char loggingFilename[256]; /**< (String: Filename) The logging filename if internal logging mode is log to file (Default: /tmp/log) */
    int  sendECUSoftwareVersion;
    char pathToECUSoftwareVersion[256];
} DltDaemonFlags;

/**
 * The global parameters of a dlt daemon.
 */
typedef struct
{
    DltDaemonFlags flags;     /**< flags of the daemon */
    int fp;               /**< handle for own fifo */
    int sock;             /**< handle for tcp connection to client */
    int fdserial;         /**< handle for serial connection */
    int fdmax;            /**< highest number of used handles */
    fd_set master;            /**< master set of handles */
    fd_set read_fds;          /**< read set of handles */
    DltFile file;             /**< struct for file access */
    //int ohandle;          /**< handle to output file */
    DltMessage msg;           /**< one dlt message */
    DltReceiver receiver;     /**< receiver for fifo connection */
    DltReceiver receiverSock; /**< receiver for socket connection */
    DltReceiver receiverSerial; /**< receiver for serial connection */
    int client_connections;    /**< counter for nr. of client connections */
    size_t baudrate;          /**< Baudrate of serial connection */
#ifdef DLT_SHM_ENABLE
    DltShm dlt_shm;				/**< Shared memory handling */
#endif
    DltOfflineTrace offlineTrace; /**< Offline trace handling */
} DltDaemonLocal;

typedef struct
{
    int timer_fd;
    unsigned long long wakeups_missed;
} DltDaemonPeriodicData;

typedef struct
{
    DltDaemon *daemon;
    DltDaemonLocal *daemon_local;
} DltDaemonTimingPacketThreadData;

typedef DltDaemonTimingPacketThreadData DltDaemonECUVersionThreadData;

/* Function prototypes */
void dlt_daemon_local_cleanup(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
int dlt_daemon_local_init_p1(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
int dlt_daemon_local_init_p2(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
int dlt_daemon_local_connection_init(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);

void dlt_daemon_daemonize(int verbose);
void dlt_daemon_signal_handler(int sig);

int dlt_daemon_process_client_connect(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
int dlt_daemon_process_client_messages(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
int dlt_daemon_process_client_messages_serial(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
int dlt_daemon_process_user_messages(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);

int dlt_daemon_process_user_message_overflow(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
int dlt_daemon_process_user_message_register_application(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
int dlt_daemon_process_user_message_unregister_application(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
int dlt_daemon_process_user_message_register_context(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
int dlt_daemon_process_user_message_unregister_context(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
int dlt_daemon_process_user_message_log(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
#ifdef DLT_SHM_ENABLE
int dlt_daemon_process_user_message_log_shm(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
#endif
int dlt_daemon_process_user_message_set_app_ll_ts(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
int dlt_daemon_process_user_message_log_mode(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);

int dlt_daemon_send_ringbuffer_to_client(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
void dlt_daemon_timingpacket_thread(void *ptr);
void dlt_daemon_ecu_version_thread(void *ptr);
#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE)
	void dlt_daemon_systemd_watchdog_thread(void *ptr);
#endif
int dlt_daemon_make_periodic (unsigned int period, DltDaemonPeriodicData *info, int verbose);
void dlt_daemon_wait_period(DltDaemonPeriodicData *info, int verbose);

#endif /* DLT_DAEMON_H */

