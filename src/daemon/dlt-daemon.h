/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
 * Copyright (C) 2025, Minh Luu Quang
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
 * Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 * Markus Klein <Markus.Klein@esk.fraunhofer.de>
 * Minh Luu Quang <minh.luuquang@vn.bosch.com>
 *
 * \copyright Copyright © 2011-2015 BMW AG.
 * Copyright © 2025 Minh Luu Quang.
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-daemon.c
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-daemon.h                                                  **
**                                                                            **
**  TARGET    : linux, android, qnx                                           **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel <Alexander.AW.Wenzel@bmw.de>                 **
**              Markus Klein <Markus.Klein@esk.fraunhofer.de>                 **
**              Minh Luu Quang <minh.luuquang@vn.bosch.com>                   **
**  PURPOSE   :                                                               **
**                                                                            **
**  REMARKS   :                                                               **
**                                                                            **
**  PLATFORM DEPENDENCY [yes/no]: yes                                         **
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
**  ml          Minh Luu Quang             BOSCH                              **
*******************************************************************************/

#ifndef DLT_DAEMON_H
#define DLT_DAEMON_H

/* Standard Library Headers */
#include <stdarg.h>
#include <limits.h>
#include <sys/time.h>

/* Custom Daemon and Application Headers */
#include "dlt_gateway_types.h"
#include "dlt_offline_trace.h"
#include "dlt_user_shared.h"
#include "dlt_user_shared_cfg.h"
#include "dlt_daemon_common.h"
#include "dlt_daemon_event_handler_types.h"

/* DLT Macros */
#define DLT_DAEMON_FLAG_MAX              256
#define DLT_DAEMON_ERROR_OK              0
#define DLT_DAEMON_ERROR_UNKNOWN        -1
#define DLT_DAEMON_ERROR_BUFFER_FULL    -2
#define DLT_DAEMON_ERROR_SEND_FAILED    -3
#define DLT_DAEMON_ERROR_WRITE_FAILED   -4

/**
 * DLT Daemon Flags Structure
 *
 * Contains various flags and configuration parameters for controlling
 * the behavior of the DLT daemon. This includes settings for message
 * printing formats, logging configurations, socket communication,
 * offline trace storage, and other daemon-related options. Each flag
 * is a boolean or integer that enables or configures a specific feature.
 */
typedef struct
{
    /* General Flags */
    int aflag;      /**< (Boolean) Print DLT messages; payload as ASCII */
    int sflag;      /**< (Boolean) Print DLT messages; payload as hex */
    int xflag;      /**< (Boolean) Print DLT messages; only headers */
    int vflag;      /**< (Boolean) Verbose mode */
    int dflag;      /**< (Boolean) Daemonize */
    int lflag;      /**< (Boolean) Send DLT messages with serial header */
    int rflag;      /**< (Boolean) Send automatic get log info response during context registration */
    int mflag;      /**< (Boolean) Sync to serial header on serial connection */
    int nflag;      /**< (Boolean) Sync to serial header on all TCP connections */

    /* String-based Flags */
    char evalue[NAME_MAX + 1];   /**< (String: ECU ID) Set ECU ID (Default: ECU1) */
    char bvalue[NAME_MAX + 1];   /**< (String: Baudrate) Serial device baudrate (Default: 115200) */
    char yvalue[NAME_MAX + 1];   /**< (String: Devicename) Additional support for serial device */
    char ivalue[NAME_MAX + 1];   /**< (String: Directory) Directory where to store the persistent configuration (Default: /tmp) */
    char cvalue[NAME_MAX + 1];   /**< (String: Directory) Filename of DLT configuration file (Default: /etc/dlt.conf) */
#ifdef DLT_LOG_LEVEL_APP_CONFIG
    char avalue[NAME_MAX + 1];   /**< (String: Directory) Filename of the app id default level config (Default: /etc/dlt-log-levels.conf) */
#endif
#ifdef DLT_TRACE_LOAD_CTRL_ENABLE
    char lvalue[NAME_MAX + 1];   /**< (String: Directory) Filename of DLT trace limit file (Default: /etc/dlt-trace_load.conf) */
#endif

    /* Memory and Trace Settings */
    int sharedMemorySize;                            /**< (Int) Size of shared memory (Default: 100000) */
    int sendMessageTime;                             /**< (Boolean) Send periodic Message Time if client is connected (Default: 0) */
    char offlineTraceDirectory[DLT_DAEMON_FLAG_MAX]; /**< (String: Directory) Store DLT messages to local directory (Default: /etc/dlt.conf) */
    int offlineTraceFileSize;                        /**< (Int) Maximum size in bytes of one trace file (Default: 1000000) */
    int offlineTraceMaxSize;                         /**< (Int) Maximum size of all trace files (Default: 4000000) */
    bool offlineTraceFilenameTimestampBased;         /**< (Boolean) Timestamp-based or index-based filenames (Default: true=Timestamp based) */

    /* Logging Settings */
    DltLoggingMode loggingMode;                /**< (Int) The logging console for internal logging of dlt-daemon (Default: 0) */
    int loggingLevel;                          /**< (Int) The logging level for internal logging of dlt-daemon (Default: 6) */
    char loggingFilename[DLT_DAEMON_FLAG_MAX]; /**< (String: Filename) The logging filename if internal logging mode is log to file (Default: /tmp/log) */
    bool enableLoggingFileLimit;               /**< (Boolean) Indicate whether size of logging file(s) is limited (Default: false) */
    int loggingFileSize;                       /**< (Int) Maximum size in bytes of one logging file (Default: 250000) */
    int loggingFileMaxSize;                    /**< (Int) Maximum size in bytes of all logging files (Default: 1000000) */

    /* ECU and Timezone Configuration */
    int sendECUSoftwareVersion;                            /**< (Boolean) Send ECU software version periodically */
    char pathToECUSoftwareVersion[DLT_DAEMON_FLAG_MAX];    /**< (String: Filename) The file from which to read the ECU version from. */
    char ecuSoftwareVersionFileField[DLT_DAEMON_FLAG_MAX]; /**< (String: Value) Reads a specific VALUE from a FIELD=VALUE ECU version file. */
    int sendTimezone;                                      /**< (Boolean) Send Timezone periodically */

    /* Offline Log Storage */
    int offlineLogstorageMaxDevices;                   /**< (Int) Maximum devices to be used as offline log storage devices */
    char offlineLogstorageDirPath[DLT_MOUNT_PATH_MAX]; /**< (String: Directory) DIR path to store offline logs */
    int offlineLogstorageTimestamp;                    /**< (Int) Append timestamp in offline log storage filename */
    char offlineLogstorageDelimiter;                   /**< (char) Append delimiter character in offline log storage filename */
    unsigned int offlineLogstorageMaxCounter;          /**< (Int) Maximum offline log storage file counter index until wraparound */
    unsigned int offlineLogstorageMaxCounterIdx;       /**< (Int) String length of offlineLogstorageMaxCounter */
    unsigned int offlineLogstorageCacheSize;           /**< (Int) Max cache size offline log storage cache */
    int offlineLogstorageOptionalCounter;              /**< (Boolean) Do not append index to filename if NOFiles=1 */
#ifdef DLT_DAEMON_USE_UNIX_SOCKET_IPC
    char appSockPath[DLT_DAEMON_FLAG_MAX];             /**< (String: Directory) Path to User socket */
#else /* DLT_DAEMON_USE_FIFO_IPC */
    char userPipesDir[DLT_PATH_MAX];                   /**< (String: Directory) directory where dltpipes reside (Default: /tmp/dltpipes) */
    char daemonFifoName[DLT_PATH_MAX];                 /**< (String: Filename) name of local fifo (Default: /tmp/dlt) */
    char daemonFifoGroup[DLT_PATH_MAX];                /**< (String: Group name) Owner group of local fifo (Default: Primary Group) */
#endif
#ifdef DLT_SHM_ENABLE
    char dltShmName[NAME_MAX + 1]; /**< Shared memory name */
#endif

    /* Networking and Gateway Configuration */
    unsigned int port;                           /**< (Int) Port number */
    char ctrlSockPath[DLT_DAEMON_FLAG_MAX];      /**< (String: Directory) Path to Control socket */
    int gatewayMode;                             /**< (Boolean) Gateway Mode */
    char gatewayConfigFile[DLT_DAEMON_FLAG_MAX]; /**< (String: Directory) Gateway config file path */

    /* Context Registration and Log Level Enforcement */
    int autoResponseGetLogInfoOption;   /**< (Int) The Option of automatic get log info response during context registration. (Default: 7) */
    int contextLogLevel;                /**< (Int) log level sent to context if registered with default log-level or if enforced */
    int contextTraceStatus;             /**< (Int) trace status sent to context if registered with default trace status or if enforced */
    int enforceContextLLAndTS;          /**< (Boolean) Enforce log-level, trace-status not to exceed contextLogLevel, contextTraceStatus */

    /* IP Binding and Injection Configuration */
    DltBindAddress_t* ipNodes; /**< (String: BindAddress) The daemon accepts connections only on this list of IP addresses */
    int injectionMode;         /**< (Boolean) Injection mode */
} DltDaemonFlags;

/**
 * DLT Daemon Local Data Structure
 *
 * Stores runtime data for the DLT daemon, including communication settings,
 * file access, event handling, logging, and other configurations related
 * to the daemon’s operational state. This struct is used internally to
 * manage the state of the daemon in the context of handling client connections,
 * messages, offline traces, and the specific configuration of hardware or
 * network communication.
 */
typedef struct
{
    DltDaemonFlags flags;   /**< flags of the daemon */
    DltFile file;           /**< struct for file access */
    DltEventHandler pEvent; /**< struct for message producer event handling */
    DltGateway pGateway;    /**< struct for passive node connection handling */
    DltMessage msg;         /**< one DLT message */
    int client_connections; /**< counter for number of client connections */
    size_t baudrate;        /**< Baudrate of serial connection */

#ifdef DLT_SHM_ENABLE
    DltShm dlt_shm;              /**< Shared memory handling */
    unsigned char *recv_buf_shm; /**< buffer for receiving messages from shm */
#endif

    /* Offline Trace and Logging Handling */
    MultipleFilesRingBuffer offlineTrace; /**< Offline trace handling */
    MultipleFilesRingBuffer dltLogging;   /**< DLT logging handling */

    /* Timeout and Ringbuffer Configuration */
    int timeoutOnSend;                /**< Timeout on sending messages */
    unsigned long RingbufferMinSize;  /**< Minimum size of the ring buffer */
    unsigned long RingbufferMaxSize;  /**< Maximum size of the ring buffer */
    unsigned long RingbufferStepSize; /**< Step size for the ring buffer */

    /* Daemon FIFO Settings */
    unsigned long daemonFifoSize;                     /**< Size of the daemon FIFO */

#ifdef UDP_CONNECTION_SUPPORT
    int UDPConnectionSetup;                           /**< Enable/disable the UDP connection */
    char UDPMulticastIPAddress[MULTICASTIP_MAX_SIZE]; /**< Multicast IP address */
    int UDPMulticastIPPort;                           /**< Multicast port */
#endif
} DltDaemonLocal;

/**
 * DLT Daemon Periodic Data Structure
 *
 * Stores the periodic information about missed wakeups, timer period,
 * and the number of starts.
 */
typedef struct
{
    unsigned long long wakeups_missed; /**< Number of missed wakeups */
    int period_sec;                    /**< Period of the timer in seconds */
    int starts_in;                     /**< Number of times the timer will start */
    int timer_id;                      /**< Unique identifier for the timer */
} DltDaemonPeriodicData;

/**
 * DLT Daemon Timing Packet Thread Data Structure
 *
 * Holds references to the DLT daemon and its local instance,
 * which are used in the timing packet thread processing.
 */
typedef struct
{
    DltDaemon *daemon;            /**< Pointer to the DLT daemon instance */
    DltDaemonLocal *daemon_local; /**< Pointer to the DLT daemon local instance */
} DltDaemonTimingPacketThreadData;

/**
 * DLT Daemon ECU Version Thread Data
 *
 * This is a typedef for `DltDaemonTimingPacketThreadData`
 * that is specifically used for ECU version processing in a separate thread.
 */
typedef DltDaemonTimingPacketThreadData DltDaemonECUVersionThreadData;

/* ---------- DLT Function Prototypes for Initialization and Cleanup ---------- */
/** Cleanup the DLT Daemon and its local data */
void dlt_daemon_local_cleanup(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
/** Initialize the DLT Daemon Local (Phase 1) */
int dlt_daemon_local_init_p1(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
/** Initialize the DLT Daemon Local (Phase 2) */
int dlt_daemon_local_init_p2(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
/** Initialize connection for the DLT Daemon */
int dlt_daemon_local_connection_init(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
/** Initialize ECU version for the DLT Daemon */
int dlt_daemon_local_ecu_version_init(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);

/* ---------- DLT Function Prototypes for Daemon Control and Signal Handling ---------- */
/** Daemonize the DLT Daemon (run in the background) */
void dlt_daemon_daemonize(int verbose);
/** Trigger the DLT Daemon exit process */
void dlt_daemon_exit_trigger();
/** Handle signals for the DLT Daemon */
void dlt_daemon_signal_handler(int sig);

/* ---------- DLT Function Prototypes for Client and Message Processing ---------- */
/** Process a new client connection */
int dlt_daemon_process_client_connect(DltDaemon *daemon,
                                      DltDaemonLocal *daemon_local,
                                      DltReceiver *recv,
                                      int verbose);

/** Process messages from a connected client */
int dlt_daemon_process_client_messages(DltDaemon *daemon,
                                       DltDaemonLocal *daemon_local,
                                       DltReceiver *recv,
                                       int verbose);

/** Process messages from a client using serial communication */
int dlt_daemon_process_client_messages_serial(DltDaemon *daemon,
                                              DltDaemonLocal *daemon_local,
                                              DltReceiver *recv,
                                              int verbose);

/** Process user messages received by the DLT Daemon */
int dlt_daemon_process_user_messages(DltDaemon *daemon,
                                     DltDaemonLocal *daemon_local,
                                     DltReceiver *recv,
                                     int verbose);

/** Process a 1-second timer event */
int dlt_daemon_process_one_s_timer(DltDaemon *daemon,
                                   DltDaemonLocal *daemon_local,
                                   DltReceiver *recv,
                                   int verbose);

/** Process a 60-second timer event */
int dlt_daemon_process_sixty_s_timer(DltDaemon *daemon,
                                     DltDaemonLocal *daemon_local,
                                     DltReceiver *recv,
                                     int verbose);

/** Process systemd timer events */
int dlt_daemon_process_systemd_timer(DltDaemon *daemon,
                                     DltDaemonLocal *daemon_local,
                                     DltReceiver *recv,
                                     int verbose);

/* ---------- Function Prototypes for Control and IPC Connections ---------- */
/** Process a control connection from a client */
int dlt_daemon_process_control_connect(DltDaemon *daemon,
                                       DltDaemonLocal *daemon_local,
                                       DltReceiver *recv,
                                       int verbose);

/** Process an application connection (for IPC) */
#if defined DLT_DAEMON_USE_UNIX_SOCKET_IPC || defined DLT_DAEMON_VSOCK_IPC_ENABLE
int dlt_daemon_process_app_connect(DltDaemon *daemon,
                                   DltDaemonLocal *daemon_local,
                                   DltReceiver *recv,
                                   int verbose);
#endif

/** Process control messages received by the DLT Daemon */
int dlt_daemon_process_control_messages(DltDaemon *daemon,
                                        DltDaemonLocal *daemon_local,
                                        DltReceiver *recv,
                                        int verbose);

/* Function Prototypes for User Message Processing */

/** Function pointer for processing user messages */
typedef int (*dlt_daemon_process_user_message_func)(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *recv, int verbose);

/** Handle overflow in user messages */
int dlt_daemon_process_user_message_overflow(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *recv, int verbose);

/** Send overflow messages when the message buffer is full */
int dlt_daemon_send_message_overflow(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);

/** Process a register application user message */
int dlt_daemon_process_user_message_register_application(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *recv, int verbose);

/** Process an unregister application user message */
int dlt_daemon_process_user_message_unregister_application(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *recv, int verbose);

/** Process a register context user message */
int dlt_daemon_process_user_message_register_context(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *recv, int verbose);

/** Process an unregister context user message */
int dlt_daemon_process_user_message_unregister_context(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *recv, int verbose);

/** Process a log message user message */
int dlt_daemon_process_user_message_log(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltReceiver *recv, int verbose);

bool enforce_context_ll_and_ts_keep_message(DltDaemonLocal *daemon_local
#ifdef DLT_LOG_LEVEL_APP_CONFIG
                                            ,DltDaemonApplication *app
#endif
                                            );

int dlt_daemon_process_user_message_set_app_ll_ts(DltDaemon *daemon,
                                                  DltDaemonLocal *daemon_local,
                                                  DltReceiver *rec,
                                                  int verbose);
int dlt_daemon_process_user_message_marker(DltDaemon *daemon,
                                           DltDaemonLocal *daemon_local,
                                           DltReceiver *rec,
                                           int verbose);

int dlt_daemon_send_ringbuffer_to_client(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
void dlt_daemon_timingpacket_thread(void *ptr);
void dlt_daemon_ecu_version_thread(void *ptr);
#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE)
void dlt_daemon_systemd_watchdog_thread(void *ptr);
#endif

int create_timer_fd(DltDaemonLocal *daemon_local, int period_sec, int starts_in, DltTimers timer);

int dlt_daemon_close_socket(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);

#ifdef DLT_TRACE_LOAD_CTRL_ENABLE
bool trace_load_keep_message(
    DltDaemonApplication *app, int size, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
// Functions that are only exposed for testing and should not be public
// for normal builds
#ifdef DLT_UNIT_TESTS
int trace_load_config_file_parser(DltDaemon *daemon, DltDaemonLocal *daemon_local);
#endif
#endif

#endif /* DLT_DAEMON_H */
