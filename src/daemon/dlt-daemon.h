/*
 * @licence app begin@
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
 * @licence end@
 */

/*!
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-daemon.h
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

#include <limits.h> /* for NAME_MAX */

#include "dlt_daemon_common.h"
#include "dlt_user_shared.h"
#include "dlt_user_shared_cfg.h"
#include "dlt_daemon_event_handler_types.h"
#include "dlt_gateway_types.h"
#include <dlt_offline_trace.h>
#include <sys/time.h>

#define DLT_DAEMON_FLAG_MAX 256

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
    char evalue[NAME_MAX + 1];   /**< (String: ECU ID) Set ECU ID (Default: ECU1) */
    char bvalue[NAME_MAX + 1];   /**< (String: Baudrate) Serial device baudrate (Default: 115200) */
    char yvalue[NAME_MAX + 1];   /**< (String: Devicename) Additional support for serial device */
    char ivalue[NAME_MAX + 1];   /**< (String: Directory) Directory where to store the persistant configuration (Default: /tmp) */
    char cvalue[NAME_MAX + 1];   /**< (String: Directory) Filename of DLT configuration file (Default: /etc/dlt.conf) */
    int  sharedMemorySize;       /**< (int) Size of shared memory (Default: 100000) */
    int  sendMessageTime;       /**< (Boolean) Send periodic Message Time if client is connected (Default: 0) */
    char offlineTraceDirectory[DLT_DAEMON_FLAG_MAX]; /**< (String: Directory) Store DLT messages to local directory (Default: /etc/dlt.conf) */
    int  offlineTraceFileSize;    /**< (int) Maximum size in bytes of one trace file (Default: 1000000) */
    int  offlineTraceMaxSize;    /**< (int) Maximum size of all trace files (Default: 4000000) */
    int  offlineTraceFilenameTimestampBased; /**< (int) timestamp based or index based (Default: 1 Timestamp based) */
    int  loggingMode;    /**< (int) The logging console for internal logging of dlt-daemon (Default: 0) */
    int  loggingLevel;    /**< (int) The logging level for internal logging of dlt-daemon (Default: 6) */
    char loggingFilename[DLT_DAEMON_FLAG_MAX]; /**< (String: Filename) The logging filename if internal logging mode is log to file (Default: /tmp/log) */
    int  sendECUSoftwareVersion; /**< (Boolean) Send ECU software version perdiodically */
    char pathToECUSoftwareVersion[DLT_DAEMON_FLAG_MAX]; /**< (String: Filename) The file from which to read the ECU version from. */
    int  sendTimezone; /**< (Boolean) Send Timezone perdiodically */
    int  offlineLogstorageMaxDevices; /**< (int) Maximum devices to be used as offline logstorage devices */
    char offlineLogstorageDirPath[DLT_MOUNT_PATH_MAX]; /**< (String: Directory) DIR path to store offline logs  */
    int  offlineLogstorageTimestamp; /**< (int) Append timestamp in offline logstorage filename */
    char offlineLogstorageDelimiter; /**< (char) Append delimeter character in offline logstorage filename  */
    unsigned int offlineLogstorageMaxCounter; /**< (int) Maximum offline logstorage file counter index until wraparound  */
    unsigned int offlineLogstorageMaxCounterIdx; /**< (int) String len of  offlineLogstorageMaxCounter*/
    unsigned int offlineLogstorageCacheSize; /**< Max cache size offline logstorage cache */
    char userPipesDir[NAME_MAX + 1]; /**< (String: Directory) directory where dltpipes reside (Default: /tmp/dltpipes) */
    char daemonFifoName[NAME_MAX + 1]; /**< (String: Filename) name of local fifo (Default: /tmp/dlt) */
    unsigned int  port; /**< port number */
    char ctrlSockPath[DLT_DAEMON_FLAG_MAX]; /**< Path to Control socket */
    int gatewayMode; /**< (Boolean) Gateway Mode */
    int  contextLogLevel; /**< (int) log level sent to context if registered with default log-level or if enforced*/
    int  contextTraceStatus;  /**< (int) trace status sent to context if registered with default trace status  or if enforced*/
    int  enforceContextLLAndTS; /**< (Boolean) Enforce log-level, trace-status not to exceed contextLogLevel, contextTraceStatus */
} DltDaemonFlags;
/**
 * The global parameters of a dlt daemon.
 */
typedef struct
{
    DltDaemonFlags flags;     /**< flags of the daemon */
    DltFile file;             /**< struct for file access */
    DltEventHandler pEvent; /**< struct for message producer event handling */
    DltGateway pGateway; /**< struct for passive node connection handling */
    DltMessage msg;           /**< one dlt message */
    int client_connections;    /**< counter for nr. of client connections */
    size_t baudrate;          /**< Baudrate of serial connection */
#ifdef DLT_SHM_ENABLE
    DltShm dlt_shm;                /**< Shared memory handling */
#endif
    DltOfflineTrace offlineTrace; /**< Offline trace handling */
    int timeoutOnSend;
    unsigned long RingbufferMinSize;
    unsigned long RingbufferMaxSize;
    unsigned long RingbufferStepSize;
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

#define DLT_DAEMON_ERROR_OK             0
#define DLT_DAEMON_ERROR_UNKNOWN         -1
#define DLT_DAEMON_ERROR_BUFFER_FULL     -2
#define DLT_DAEMON_ERROR_SEND_FAILED     -3
#define DLT_DAEMON_ERROR_WRITE_FAILED     -4

/* Function prototypes */
void dlt_daemon_local_cleanup(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
int dlt_daemon_local_init_p1(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
int dlt_daemon_local_init_p2(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
int dlt_daemon_local_connection_init(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
int dlt_daemon_local_ecu_version_init(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);

void dlt_daemon_daemonize(int verbose);
void dlt_daemon_signal_handler(int sig);
int dlt_daemon_process_client_connect(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *recv, int verbose);
int dlt_daemon_process_client_messages(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *revc, int verbose);
int dlt_daemon_process_client_messages_serial(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *recv, int verbose);
int dlt_daemon_process_user_messages(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *recv, int verbose);
int dlt_daemon_process_one_s_timer(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *recv, int verbose);
int dlt_daemon_process_sixty_s_timer(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *recv, int verbose);
int dlt_daemon_process_systemd_timer(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *recv, int verbose);

int dlt_daemon_process_control_connect(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *recv, int verbose);
int dlt_daemon_process_control_messages(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *recv, int verbose);

typedef int (*dlt_daemon_process_user_message_func)(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *rec, int verbose);

int dlt_daemon_process_user_message_overflow(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *rec, int verbose);
int dlt_daemon_send_message_overflow(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
int dlt_daemon_process_user_message_register_application(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *rec, int verbose);
int dlt_daemon_process_user_message_unregister_application(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *rec, int verbose);
int dlt_daemon_process_user_message_register_context(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *rec, int verbose);
int dlt_daemon_process_user_message_unregister_context(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *rec, int verbose);
int dlt_daemon_process_user_message_log(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *rec, int verbose);
#ifdef DLT_SHM_ENABLE
int dlt_daemon_process_user_message_log_shm(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *rec, int verbose);
#endif
int dlt_daemon_process_user_message_set_app_ll_ts(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *rec, int verbose);
int dlt_daemon_process_user_message_marker(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *rec, int verbose);

int dlt_daemon_send_ringbuffer_to_client(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
void dlt_daemon_timingpacket_thread(void *ptr);
void dlt_daemon_ecu_version_thread(void *ptr);
#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE)
    void dlt_daemon_systemd_watchdog_thread(void *ptr);
#endif

int create_timer_fd(DltDaemonLocal *daemon_local, int period_sec, int starts_in, DltTimers timer);

int dlt_daemon_close_socket(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);

#endif /* DLT_DAEMON_H */

