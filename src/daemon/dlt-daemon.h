/*
 * Dlt- Diagnostic Log and Trace daemon
 * @licence app begin@
 *
 * Copyright (C) 2011, BMW AG - Alexander Wenzel <alexander.wenzel@bmw.de>
 * 
 * This program is free software; you can redistribute it and/or modify it under the terms of the 
 * GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General 
 * Public License, version 2.1, for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along 
 * with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * 
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may 
 * also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * 
 * Linking DLT statically or dynamically with other modules is making a combined work based on DLT. You may 
 * license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to 
 * license your linked modules under the GNU Lesser General Public License, version 2.1, you 
 * may use the program under the following exception.
 * 
 * As a special exception, the copyright holders of DLT give you permission to combine DLT 
 * with software programs or libraries that are released under any license unless such a combination is not
 * permitted by the license of such a software program or library. You may copy and distribute such a 
 * system following the terms of the GNU Lesser General Public License, version 2.1, including this
 * special exception, for DLT and the licenses of the other code concerned.
 * 
 * Note that people who make modified versions of DLT are not obligated to grant this special exception 
 * for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, 
 * version 2.1, gives permission to release a modified version without this exception; this exception 
 * also makes it possible to release a modified version which carries forward this exception.
 *
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
    DltShm dlt_shm;				/**< Shared memory handling */
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
int dlt_daemon_process_user_message_set_app_ll_ts(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
int dlt_daemon_process_user_message_log_mode(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);

void dlt_daemon_timingpacket_thread(void *ptr);
int dlt_daemon_make_periodic (unsigned int period, DltDaemonPeriodicData *info, int verbose);
void dlt_daemon_wait_period(DltDaemonPeriodicData *info, int verbose);

#endif /* DLT_DAEMON_H */

