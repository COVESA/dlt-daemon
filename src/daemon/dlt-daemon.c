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
 * Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 * Markus Klein <Markus.Klein@esk.fraunhofer.de>
 * Mikko Rapeli <mikko.rapeli@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-daemon.c
 */

#include <netdb.h>
#include <ctype.h>
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), (), and recv() */
#include <sys/un.h>
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() and access */
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <pthread.h>
#include <grp.h>

#ifdef linux
#   include <sys/timerfd.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#include <libgen.h>

#if defined(linux) && defined(__NR_statx)
#   include <linux/stat.h>
#endif

#ifdef DLT_DAEMON_VSOCK_IPC_ENABLE
#   ifdef linux
#       include <linux/vm_sockets.h>
#   endif
#   ifdef __QNX__
#       include <vm_sockets.h>
#   endif
#endif

#include "dlt_types.h"
#include "dlt-daemon.h"
#include "dlt-daemon_cfg.h"
#include "dlt_daemon_common_cfg.h"

#include "dlt_daemon_socket.h"
#include "dlt_daemon_unix_socket.h"
#include "dlt_daemon_serial.h"

#include "dlt_daemon_client.h"
#include "dlt_daemon_connection.h"
#include "dlt_daemon_event_handler.h"
#include "dlt_daemon_offline_logstorage.h"
#include "dlt_gateway.h"

#ifdef UDP_CONNECTION_SUPPORT
#   include "dlt_daemon_udp_socket.h"
#endif
#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE) || defined(DLT_SYSTEMD_ENABLE)
#   include "sd-daemon.h"
#endif

/**
 * \defgroup daemon DLT Daemon
 * \addtogroup daemon
 \{
 */

static int dlt_daemon_log_internal(DltDaemon *daemon, DltDaemonLocal *daemon_local, char *str, int verbose);

static int dlt_daemon_check_numeric_setting(char *token,
                                            char *value,
                                            unsigned long *data);

#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
static uint32_t watchdog_trigger_interval;  /* watchdog trigger interval in [s] */
#endif

/* used in main event loop and signal handler */
int g_exit = 0;

int g_signo = 0;

/* used for value from conf file */
static int value_length = 1024;

static char dlt_timer_conn_types[DLT_TIMER_UNKNOWN + 1] = {
    [DLT_TIMER_PACKET] = DLT_CONNECTION_ONE_S_TIMER,
    [DLT_TIMER_ECU] = DLT_CONNECTION_SIXTY_S_TIMER,
#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
    [DLT_TIMER_SYSTEMD] = DLT_CONNECTION_SYSTEMD_TIMER,
#endif
    [DLT_TIMER_GATEWAY] = DLT_CONNECTION_GATEWAY_TIMER,
    [DLT_TIMER_UNKNOWN] = DLT_CONNECTION_TYPE_MAX
};

static char dlt_timer_names[DLT_TIMER_UNKNOWN + 1][32] = {
    [DLT_TIMER_PACKET] = "Timing packet",
    [DLT_TIMER_ECU] = "ECU version",
#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
    [DLT_TIMER_SYSTEMD] = "Systemd watchdog",
#endif
    [DLT_TIMER_GATEWAY] = "Gateway",
    [DLT_TIMER_UNKNOWN] = "Unknown timer"
};

#ifdef __QNX__
static int dlt_timer_pipes[DLT_TIMER_UNKNOWN][2] = {
    /* [timer_id] = {read_pipe, write_pipe} */
    [DLT_TIMER_PACKET] = {DLT_FD_INIT, DLT_FD_INIT},
    [DLT_TIMER_ECU] = {DLT_FD_INIT, DLT_FD_INIT},
#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
    [DLT_TIMER_SYSTEMD] = {DLT_FD_INIT, DLT_FD_INIT},
#endif
    [DLT_TIMER_GATEWAY] = {DLT_FD_INIT, DLT_FD_INIT}
};

static pthread_t timer_threads[DLT_TIMER_UNKNOWN] = {
    [DLT_TIMER_PACKET] = 0,
    [DLT_TIMER_ECU] = 0,
#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
    [DLT_TIMER_SYSTEMD] = 0,
#endif
    [DLT_TIMER_GATEWAY] = 0
};

static DltDaemonPeriodicData *timer_data[DLT_TIMER_UNKNOWN] = {
    [DLT_TIMER_PACKET] = NULL,
    [DLT_TIMER_ECU] = NULL,
#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
    [DLT_TIMER_SYSTEMD] = NULL,
#endif
    [DLT_TIMER_GATEWAY] = NULL
};

void close_pipes(int fds[2])
{
    if (fds[0] > 0) {
        close(fds[0]);
        fds[0] = DLT_FD_INIT;
    }

    if (fds[1] > 0) {
        close(fds[1]);
        fds[1] = DLT_FD_INIT;
    }
}

#endif // __QNX__

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[DLT_DAEMON_TEXTBUFSIZE];
    dlt_get_version(version, DLT_DAEMON_TEXTBUFSIZE);

    /*printf("DLT logging daemon %s %s\n", _DLT_PACKAGE_VERSION, _DLT_PACKAGE_VERSION_STATE); */
    /*printf("Compile options: %s %s %s %s",_DLT_SYSTEMD_ENABLE, _DLT_SYSTEMD_WATCHDOG_ENABLE, _DLT_TEST_ENABLE, _DLT_SHM_ENABLE); */
    printf("%s", version);
    printf("Usage: dlt-daemon [options]\n");
    printf("Options:\n");
    printf("  -d            Daemonize\n");
    printf("  -h            Usage\n");
    printf("  -c filename   DLT daemon configuration file (Default: " CONFIGURATION_FILES_DIR "/dlt.conf)\n");

#ifdef DLT_DAEMON_USE_FIFO_IPC
    printf("  -t directory  Directory for local fifo and user-pipes (Default: /tmp)\n");
    printf("                (Applications wanting to connect to a daemon using a\n");
    printf("                custom directory need to be started with the environment \n");
    printf("                variable DLT_PIPE_DIR set appropriately)\n");
#endif

#ifdef DLT_SHM_ENABLE
    printf("  -s filename   The file name to create the share memory (Default: /dlt-shm)\n");
    printf("                (Applications wanting to connect to a daemon using a\n");
    printf("                custom shm name need to be started with the environment \n");
    printf("                variable DLT_SHM_NAME set appropriately)\n");
#endif
    printf("  -p port       port to monitor for incoming requests (Default: 3490)\n");
    printf("                (Applications wanting to connect to a daemon using a custom\n");
    printf("                port need to be started with the environment variable\n");
    printf("                DLT_DAEMON_TCP_PORT set appropriately)\n");
#ifdef DLT_LOG_LEVEL_APP_CONFIG
    printf("  -a filename   The filename for load default app id log levels (Default: " CONFIGURATION_FILES_DIR "/dlt-log-levels.conf)\n");
#endif

#
} /* usage() */

/**
 * Option handling
 */
int option_handling(DltDaemonLocal *daemon_local, int argc, char *argv[])
{
    int c;
    char options[255];
    memset(options, 0, sizeof options);
    const char *const default_options = "hdc:t:p:";
    strcpy(options, default_options);

    if (daemon_local == 0) {
        fprintf (stderr, "Invalid parameter passed to option_handling()\n");
        return -1;
    }

    /* Initialize flags */
    memset(daemon_local, 0, sizeof(DltDaemonLocal));

    /* default values */
    daemon_local->flags.port = DLT_DAEMON_TCP_PORT;

#ifdef DLT_DAEMON_USE_FIFO_IPC
    dlt_log_set_fifo_basedir(DLT_USER_IPC_PATH);
#endif

#ifdef DLT_SHM_ENABLE
    strncpy(dltShmName, "/dlt-shm", NAME_MAX);
#endif

    opterr = 0;

#ifdef DLT_SHM_ENABLE
    strcpy(options + strlen(options), "s:");
#endif
#ifdef DLT_LOG_LEVEL_APP_CONFIG
    strcpy(options + strlen(options), "a:");
#endif
    while ((c = getopt(argc, argv, options)) != -1)
        switch (c) {
        case 'd':
        {
            daemon_local->flags.dflag = 1;
            break;
        }
        case 'c':
        {
            strncpy(daemon_local->flags.cvalue, optarg, NAME_MAX);
            break;
        }
#ifdef DLT_LOG_LEVEL_APP_CONFIG
        case 'a':
        {
            strncpy(daemon_local->flags.avalue, optarg, NAME_MAX);
            break;
        }
#endif
#ifdef DLT_DAEMON_USE_FIFO_IPC
        case 't':
        {
            dlt_log_set_fifo_basedir(optarg);
            break;
        }
#endif

#ifdef DLT_SHM_ENABLE
        case 's':
        {
            strncpy(dltShmName, optarg, NAME_MAX);
            break;
        }
#endif
        case 'p':
        {
            daemon_local->flags.port = (unsigned int) atoi(optarg);

            if (daemon_local->flags.port == 0) {
                fprintf (stderr, "Invalid port `%s' specified.\n", optarg);
                return -1;
            }

            break;
        }
        case 'h':
        {
            usage();
            return -2; /* return no error */
        }
        case '?':
        {
            if ((optopt == 'c') || (optopt == 't') || (optopt == 'p')
    #ifdef DLT_LOG_LEVEL_APP_CONFIG
                  || (optopt == 'a')
    #endif
          )
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr, "Unknown option character `\\x%x'.\n", (uint32_t)optopt);

            /* unknown or wrong option used, show usage information and terminate */
            usage();
            return -1;
        }
        default:
        {
            fprintf (stderr, "Invalid option, this should never occur!\n");
            return -1;
        }
        }

    /* switch() */

#ifdef DLT_DAEMON_USE_FIFO_IPC
    snprintf(daemon_local->flags.userPipesDir, DLT_PATH_MAX,
             "%s/dltpipes", dltFifoBaseDir);
    snprintf(daemon_local->flags.daemonFifoName, DLT_PATH_MAX,
             "%s/dlt", dltFifoBaseDir);
#endif

#ifdef DLT_SHM_ENABLE
    strncpy(daemon_local->flags.dltShmName, dltShmName, NAME_MAX);
#endif

    return 0;

}  /* option_handling() */

/**
 * Option file parser
 */
int option_file_parser(DltDaemonLocal *daemon_local)
{
    FILE *pFile;
    char line[value_length - 1];
    char token[value_length];
    char value[value_length];
    char *pch;
    const char *filename;
    ssize_t n;

    /* set default values for configuration */
    daemon_local->flags.sharedMemorySize = DLT_SHM_SIZE;
    daemon_local->flags.sendMessageTime = 0;
    daemon_local->flags.offlineTraceDirectory[0] = 0;
    daemon_local->flags.offlineTraceFileSize = 1000000;
    daemon_local->flags.offlineTraceMaxSize = 4000000;
    daemon_local->flags.offlineTraceFilenameTimestampBased = true;
    daemon_local->flags.loggingMode = DLT_LOG_TO_CONSOLE;
    daemon_local->flags.loggingLevel = LOG_INFO;

#ifdef DLT_DAEMON_USE_UNIX_SOCKET_IPC
    n = snprintf(daemon_local->flags.loggingFilename,
                 sizeof(daemon_local->flags.loggingFilename),
                 "%s/dlt.log", DLT_USER_IPC_PATH);
#else /* DLT_DAEMON_USE_FIFO_IPC */
    n = snprintf(daemon_local->flags.loggingFilename,
                 sizeof(daemon_local->flags.loggingFilename),
                 "%s/dlt.log", dltFifoBaseDir);
#endif

    if (n < 0 || (size_t)n > sizeof(daemon_local->flags.loggingFilename)) {
        dlt_vlog(LOG_WARNING, "%s: snprintf truncation/error(%ld) %s\n",
                __func__, n, daemon_local->flags.loggingFilename);
    }
    daemon_local->flags.enableLoggingFileLimit = false;
    daemon_local->flags.loggingFileSize = 250000;
    daemon_local->flags.loggingFileMaxSize = 1000000;

    daemon_local->timeoutOnSend = 4;
    daemon_local->RingbufferMinSize = DLT_DAEMON_RINGBUFFER_MIN_SIZE;
    daemon_local->RingbufferMaxSize = DLT_DAEMON_RINGBUFFER_MAX_SIZE;
    daemon_local->RingbufferStepSize = DLT_DAEMON_RINGBUFFER_STEP_SIZE;
    daemon_local->daemonFifoSize = 0;
    daemon_local->flags.sendECUSoftwareVersion = 0;
    memset(daemon_local->flags.pathToECUSoftwareVersion, 0, sizeof(daemon_local->flags.pathToECUSoftwareVersion));
    daemon_local->flags.sendTimezone = 0;
    daemon_local->flags.offlineLogstorageMaxDevices = 0;
    daemon_local->flags.offlineLogstorageDirPath[0] = 0;
    daemon_local->flags.offlineLogstorageTimestamp = 1;
    daemon_local->flags.offlineLogstorageDelimiter = '_';
    daemon_local->flags.offlineLogstorageMaxCounter = UINT_MAX;
    daemon_local->flags.offlineLogstorageMaxCounterIdx = 0;
    daemon_local->flags.offlineLogstorageCacheSize = 30000; /* 30MB */
    dlt_daemon_logstorage_set_logstorage_cache_size(
        daemon_local->flags.offlineLogstorageCacheSize);
    strncpy(daemon_local->flags.ctrlSockPath,
            DLT_DAEMON_DEFAULT_CTRL_SOCK_PATH,
            sizeof(daemon_local->flags.ctrlSockPath));
#ifdef DLT_DAEMON_USE_UNIX_SOCKET_IPC
    snprintf(daemon_local->flags.appSockPath, DLT_IPC_PATH_MAX, "%s/dlt", DLT_USER_IPC_PATH);

    if (strlen(DLT_USER_IPC_PATH) > DLT_IPC_PATH_MAX)
        fprintf(stderr, "Provided path too long...trimming it to path[%s]\n",
                daemon_local->flags.appSockPath);

#else /* DLT_DAEMON_USE_FIFO_IPC */
    memset(daemon_local->flags.daemonFifoGroup, 0, sizeof(daemon_local->flags.daemonFifoGroup));
#endif
    daemon_local->flags.gatewayMode = 0;
    strncpy(daemon_local->flags.gatewayConfigFile,
            DLT_GATEWAY_CONFIG_PATH,
            DLT_DAEMON_FLAG_MAX);
    daemon_local->flags.autoResponseGetLogInfoOption = 7;
    daemon_local->flags.contextLogLevel = DLT_LOG_INFO;
    daemon_local->flags.contextTraceStatus = DLT_TRACE_STATUS_OFF;
    daemon_local->flags.enforceContextLLAndTS = 0; /* default is off */
#ifdef UDP_CONNECTION_SUPPORT
    daemon_local->UDPConnectionSetup = MULTICAST_CONNECTION_ENABLED;
    strncpy(daemon_local->UDPMulticastIPAddress, MULTICASTIPADDRESS, MULTICASTIP_MAX_SIZE - 1);
    daemon_local->UDPMulticastIPPort = MULTICASTIPPORT;
#endif
    daemon_local->flags.ipNodes = NULL;
    daemon_local->flags.injectionMode = 1;

    /* open configuration file */
    if (daemon_local->flags.cvalue[0])
        filename = daemon_local->flags.cvalue;
    else
        filename = CONFIGURATION_FILES_DIR "/dlt.conf";

    /*printf("Load configuration from file: %s\n",filename); */
    pFile = fopen (filename, "r");

    if (pFile != NULL) {
        while (1) {
            /* fetch line from configuration file */
            if (fgets (line, value_length - 1, pFile) != NULL) {
                pch = strtok (line, " =\r\n");
                token[0] = 0;
                value[0] = 0;

                while (pch != NULL) {
                    if (strcmp(pch, "#") == 0)
                        break;

                    if (token[0] == 0) {
                        strncpy(token, pch, sizeof(token) - 1);
                        token[sizeof(token) - 1] = 0;
                    }
                    else {
                        strncpy(value, pch, sizeof(value) - 1);
                        value[sizeof(value) - 1] = 0;
                        break;
                    }

                    pch = strtok (NULL, " =\r\n");
                }

                if (token[0] && value[0]) {
                    /* parse arguments here */
                    if (strcmp(token, "Verbose") == 0) {
                        daemon_local->flags.vflag = atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "PrintASCII") == 0)
                    {
                        daemon_local->flags.aflag = atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "PrintHex") == 0)
                    {
                        daemon_local->flags.xflag = atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "PrintHeadersOnly") == 0)
                    {
                        daemon_local->flags.sflag = atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "SendSerialHeader") == 0)
                    {
                        daemon_local->flags.lflag = atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "SendContextRegistration") == 0)
                    {
                        daemon_local->flags.rflag = atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "SendContextRegistrationOption") == 0)
                    {
                        daemon_local->flags.autoResponseGetLogInfoOption = atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "SendMessageTime") == 0)
                    {
                        daemon_local->flags.sendMessageTime = atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "RS232SyncSerialHeader") == 0)
                    {
                        daemon_local->flags.mflag = atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "TCPSyncSerialHeader") == 0)
                    {
                        daemon_local->flags.nflag = atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "RS232DeviceName") == 0)
                    {
                        strncpy(daemon_local->flags.yvalue, value, NAME_MAX);
                        daemon_local->flags.yvalue[NAME_MAX] = 0;
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "RS232Baudrate") == 0)
                    {
                        strncpy(daemon_local->flags.bvalue, value, NAME_MAX);
                        daemon_local->flags.bvalue[NAME_MAX] = 0;
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "ECUId") == 0)
                    {
                        strncpy(daemon_local->flags.evalue, value, NAME_MAX);
                        daemon_local->flags.evalue[NAME_MAX] = 0;
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "PersistanceStoragePath") == 0)
                    {
                        strncpy(daemon_local->flags.ivalue, value, NAME_MAX);
                        daemon_local->flags.ivalue[NAME_MAX] = 0;
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "LoggingMode") == 0)
                    {
                        daemon_local->flags.loggingMode = (DltLoggingMode)atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "LoggingLevel") == 0)
                    {
                        daemon_local->flags.loggingLevel = atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "LoggingFilename") == 0)
                    {
                        strncpy(daemon_local->flags.loggingFilename,
                                value,
                                sizeof(daemon_local->flags.loggingFilename) - 1);
                        daemon_local->flags.loggingFilename[sizeof(daemon_local->flags.loggingFilename) - 1] = 0;
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "EnableLoggingFileLimit") == 0)
                    {
                        daemon_local->flags.enableLoggingFileLimit = (bool)atoi(value);
                    }
                    else if (strcmp(token, "LoggingFileSize") == 0)
                    {
                        daemon_local->flags.loggingFileSize = atoi(value);
                    }
                    else if (strcmp(token, "LoggingFileMaxSize") == 0)
                    {
                        daemon_local->flags.loggingFileMaxSize = atoi(value);
                    }
                    else if (strcmp(token, "TimeOutOnSend") == 0)
                    {
                        daemon_local->timeoutOnSend = atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "RingbufferMinSize") == 0)
                    {
                        if (dlt_daemon_check_numeric_setting(token,
                                value, &(daemon_local->RingbufferMinSize)) < 0)
                            return -1;
                    }
                    else if (strcmp(token, "RingbufferMaxSize") == 0)
                    {
                        if (dlt_daemon_check_numeric_setting(token,
                                value, &(daemon_local->RingbufferMaxSize)) < 0)
                            return -1;
                    }
                    else if (strcmp(token, "RingbufferStepSize") == 0)
                    {
                        if (dlt_daemon_check_numeric_setting(token,
                                value, &(daemon_local->RingbufferStepSize)) < 0)
                            return -1;
                    }
                    else if (strcmp(token, "SharedMemorySize") == 0)
                    {
                        daemon_local->flags.sharedMemorySize = atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "OfflineTraceDirectory") == 0)
                    {
                        strncpy(daemon_local->flags.offlineTraceDirectory, value,
                                sizeof(daemon_local->flags.offlineTraceDirectory) - 1);
                        daemon_local->flags.offlineTraceDirectory[sizeof(daemon_local->flags.offlineTraceDirectory) -
                                                                  1] = 0;
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "OfflineTraceFileSize") == 0)
                    {
                        daemon_local->flags.offlineTraceFileSize = atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "OfflineTraceMaxSize") == 0)
                    {
                        daemon_local->flags.offlineTraceMaxSize = atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "OfflineTraceFileNameTimestampBased") == 0)
                    {
                        daemon_local->flags.offlineTraceFilenameTimestampBased = (bool)atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "SendECUSoftwareVersion") == 0)
                    {
                        daemon_local->flags.sendECUSoftwareVersion = atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "PathToECUSoftwareVersion") == 0)
                    {
                        strncpy(daemon_local->flags.pathToECUSoftwareVersion, value,
                                sizeof(daemon_local->flags.pathToECUSoftwareVersion) - 1);
                        daemon_local->flags.pathToECUSoftwareVersion[sizeof(daemon_local->flags.pathToECUSoftwareVersion)
                                                                     - 1] = 0;
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "SendTimezone") == 0)
                    {
                        daemon_local->flags.sendTimezone = atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "OfflineLogstorageMaxDevices") == 0)
                    {
                        daemon_local->flags.offlineLogstorageMaxDevices = (uint32_t) atoi(value);
                    }
                    else if (strcmp(token, "OfflineLogstorageDirPath") == 0)
                    {
                        strncpy(daemon_local->flags.offlineLogstorageDirPath,
                                value,
                                sizeof(daemon_local->flags.offlineLogstorageDirPath) - 1);
                    }
                    else if (strcmp(token, "OfflineLogstorageTimestamp") == 0)
                    {
                        /* Check if set to 0, default otherwise */
                        if (atoi(value) == 0)
                            daemon_local->flags.offlineLogstorageTimestamp = 0;
                    }
                    else if (strcmp(token, "OfflineLogstorageDelimiter") == 0)
                    {
                        /* Check if valid punctuation, default otherwise*/
                        if (ispunct((char)value[0]))
                            daemon_local->flags.offlineLogstorageDelimiter = (char)value[0];
                    }
                    else if (strcmp(token, "OfflineLogstorageMaxCounter") == 0)
                    {
                        daemon_local->flags.offlineLogstorageMaxCounter = (unsigned int) atoi(value);
                        daemon_local->flags.offlineLogstorageMaxCounterIdx = (unsigned int) strlen(value);
                    }
                    else if (strcmp(token, "OfflineLogstorageCacheSize") == 0)
                    {
                        daemon_local->flags.offlineLogstorageCacheSize =
                            (unsigned int)atoi(value);
                        dlt_daemon_logstorage_set_logstorage_cache_size(
                            daemon_local->flags.offlineLogstorageCacheSize);
                    }
                    else if (strcmp(token, "ControlSocketPath") == 0)
                    {
                        memset(
                            daemon_local->flags.ctrlSockPath,
                            0,
                            DLT_DAEMON_FLAG_MAX);
                        strncpy(
                            daemon_local->flags.ctrlSockPath,
                            value,
                            DLT_DAEMON_FLAG_MAX - 1);
                    }
                    else if (strcmp(token, "GatewayMode") == 0)
                    {
                        daemon_local->flags.gatewayMode = atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "GatewayConfigFile") == 0)
                    {
                        memset(
                            daemon_local->flags.gatewayConfigFile,
                            0,
                            DLT_DAEMON_FLAG_MAX);
                        strncpy(
                            daemon_local->flags.gatewayConfigFile,
                            value,
                            DLT_DAEMON_FLAG_MAX - 1);
                    }
                    else if (strcmp(token, "ContextLogLevel") == 0)
                    {
                        int const intval = atoi(value);

                        if ((intval >= DLT_LOG_OFF) && (intval <= DLT_LOG_VERBOSE)) {
                            daemon_local->flags.contextLogLevel = intval;
                            printf("Option: %s=%s\n", token, value);
                        }
                        else {
                            fprintf(stderr,
                                    "Invalid value for ContextLogLevel: %i. Must be in range [%i..%i]\n",
                                    intval,
                                    DLT_LOG_OFF,
                                    DLT_LOG_VERBOSE);
                        }
                    }
                    else if (strcmp(token, "ContextTraceStatus") == 0)
                    {
                        int const intval = atoi(value);

                        if ((intval >= DLT_TRACE_STATUS_OFF) && (intval <= DLT_TRACE_STATUS_ON)) {
                            daemon_local->flags.contextTraceStatus = intval;
                            printf("Option: %s=%s\n", token, value);
                        }
                        else {
                            fprintf(stderr,
                                    "Invalid value for ContextTraceStatus: %i. Must be in range [%i..%i]\n",
                                    intval,
                                    DLT_TRACE_STATUS_OFF,
                                    DLT_TRACE_STATUS_ON);
                        }
                    }
                    else if (strcmp(token, "ForceContextLogLevelAndTraceStatus") == 0)
                    {
                        int const intval = atoi(value);

                        if ((intval >= 0) && (intval <= 1)) {
                            daemon_local->flags.enforceContextLLAndTS = intval;
                            printf("Option: %s=%s\n", token, value);
                        }
                        else {
                            fprintf(stderr,
                                    "Invalid value for ForceContextLogLevelAndTraceStatus: %i. Must be 0, 1\n",
                                    intval);
                        }
                    }

#ifdef DLT_DAEMON_USE_FIFO_IPC
                    else if (strcmp(token, "DaemonFIFOSize") == 0)
                    {
                        if (dlt_daemon_check_numeric_setting(token,
                                value, &(daemon_local->daemonFifoSize)) < 0)
                            return -1;
#ifndef __linux__
                            printf("Option DaemonFIFOSize is set but only supported on Linux. Ignored.\n");
#endif
                    }
                    else if (strcmp(token, "DaemonFifoGroup") == 0)
                    {
                        strncpy(daemon_local->flags.daemonFifoGroup, value, NAME_MAX);
                        daemon_local->flags.daemonFifoGroup[NAME_MAX] = 0;
                    }
#endif
#ifdef UDP_CONNECTION_SUPPORT
                    else if (strcmp(token, "UDPConnectionSetup") == 0)
                    {
                        const long longval = strtol(value, NULL, 10);

                        if ((longval == MULTICAST_CONNECTION_DISABLED)
                            || (longval == MULTICAST_CONNECTION_ENABLED)) {
                            daemon_local->UDPConnectionSetup = longval;
                            printf("Option: %s=%s\n", token, value);
                        }
                        else {
                            daemon_local->UDPConnectionSetup = MULTICAST_CONNECTION_DISABLED;
                            fprintf(stderr,
                                    "Invalid value for UDPConnectionSetup set to default %ld\n",
                                    longval);
                        }
                    }
                    else if (strcmp(token, "UDPMulticastIPAddress") == 0)
                    {
                        strncpy(daemon_local->UDPMulticastIPAddress, value,
                                MULTICASTIP_MAX_SIZE - 1);
                    }
                    else if (strcmp(token, "UDPMulticastIPPort") == 0)
                    {
                        daemon_local->UDPMulticastIPPort = strtol(value, NULL, 10);
                    }
#endif
                    else if (strcmp(token, "BindAddress") == 0)
                    {
                        DltBindAddress_t *newNode = NULL;
                        DltBindAddress_t *temp = NULL;

                        char *tok = strtok(value, ",;");

                        if (tok != NULL) {
                            daemon_local->flags.ipNodes = calloc(1, sizeof(DltBindAddress_t));

                            if (daemon_local->flags.ipNodes == NULL) {
                                dlt_vlog(LOG_ERR, "Could not allocate for IP list\n");
                                fclose(pFile);
                                return -1;
                            }
                            else {
                                strncpy(daemon_local->flags.ipNodes->ip,
                                        tok,
                                        sizeof(daemon_local->flags.ipNodes->ip) - 1);
                                daemon_local->flags.ipNodes->next = NULL;
                                temp = daemon_local->flags.ipNodes;

                                tok = strtok(NULL, ",;");

                                while (tok != NULL) {
                                    newNode = calloc(1, sizeof(DltBindAddress_t));

                                    if (newNode == NULL) {
                                        dlt_vlog(LOG_ERR, "Could not allocate for IP list\n");
                                        fclose(pFile);
                                        return -1;
                                    }
                                    else {
                                        strncpy(newNode->ip, tok, sizeof(newNode->ip) - 1);
                                    }

                                    temp->next = newNode;
                                    temp = temp->next;
                                    tok = strtok(NULL, ",;");
                                }
                            }
                        }
                        else {
                            dlt_vlog(LOG_WARNING, "BindAddress option is empty\n");
                        }
                    }
                    else if (strcmp(token, "InjectionMode") == 0) {
                        daemon_local->flags.injectionMode = atoi(value);
                    }
                    else {
                        fprintf(stderr, "Unknown option: %s=%s\n", token, value);
                    }
                }
            }
            else {
                break;
            }
        }

        fclose (pFile);
    }
    else {
        fprintf(stderr, "Cannot open configuration file: %s\n", filename);
    }

    return 0;
}

#ifdef DLT_LOG_LEVEL_APP_CONFIG
/**
 * Load configuration file parser
 */

static int compare_app_id_conf(const void *lhs, const void *rhs)
{
    return strncmp(((DltDaemonContextLogSettings *)lhs)->apid,
                   ((DltDaemonContextLogSettings *)rhs)->apid, DLT_ID_SIZE);
}

int app_id_default_log_level_config_parser(DltDaemon *daemon,
                                           DltDaemonLocal *daemon_local) {
    FILE *pFile;
    char line[value_length - 1];
    char app_id_value[value_length];
    char ctx_id_value[value_length];
    DltLogLevelType log_level_value;

    char *pch;
    const char *filename;

    /* open configuration file */
    filename = daemon_local->flags.avalue[0]
                   ? daemon_local->flags.avalue
                   : CONFIGURATION_FILES_DIR "/dlt-log-levels.conf";

    pFile = fopen(filename, "r");
    if (pFile == NULL) {
        dlt_vlog(LOG_WARNING, "Cannot open app log level configuration%s\n", filename);
        return -errno;
    }

    /* fetch lines from configuration file */
    while (fgets(line, value_length - 1, pFile) != NULL) {
        pch = strtok(line, " \t");
        memset(app_id_value, 0, value_length);
        memset(ctx_id_value, 0, value_length);
        log_level_value = DLT_LOG_MAX;

        /* ignore comments and new lines*/
        if (strncmp(pch, "#", 1) == 0 || strncmp(pch, "\n", 1) == 0
            || strlen(pch) < 1)
            continue;

        strncpy(app_id_value, pch, sizeof(app_id_value) - 1);
        app_id_value[sizeof(app_id_value) - 1] = 0;
        if (strlen(app_id_value) == 0 || strlen(app_id_value) > DLT_ID_SIZE) {
            if (app_id_value[strlen(app_id_value) - 1] == '\n') {
                dlt_vlog(LOG_WARNING, "Missing log level for apid %s in log settings\n",
                         app_id_value);
            } else {
                dlt_vlog(LOG_WARNING,
                         "Invalid apid for log settings settings: app id: %s\n",
                         app_id_value);
            }

            continue;
        }

        char *pch_next1 = strtok(NULL, " \t");
        char *pch_next2 = strtok(NULL, " \t");
        char *log_level;
        /* no context id given, log level is next token */
        if (pch_next2 == NULL || pch_next2[0] == '#') {
            log_level = pch_next1;
        } else {
            /* context id is given, log level is second to next token */
            log_level = pch_next2;

            /* next token is token id */
            strncpy(ctx_id_value, pch_next1, sizeof(ctx_id_value) - 1);
            if (strlen(ctx_id_value) == 0 || strlen(app_id_value) > DLT_ID_SIZE) {
                dlt_vlog(LOG_WARNING,
                         "Invalid ctxid for log settings: app id: %s "
                         "(skipping line)\n",
                         app_id_value);
                continue;
            }

            ctx_id_value[sizeof(ctx_id_value) - 1] = 0;
        }

        errno = 0;
        log_level_value = strtol(log_level, NULL, 10);
        if (errno != 0 || log_level_value >= DLT_LOG_MAX ||
            log_level_value <= DLT_LOG_DEFAULT) {
            dlt_vlog(LOG_WARNING,
                     "Invalid log level (%i), app id %s, conversion error: %s\n",
                     log_level_value, app_id_value, strerror(errno));
            continue;
        }

        DltDaemonContextLogSettings *settings =
            dlt_daemon_find_configured_app_id_ctx_id_settings(
                daemon, app_id_value, NULL);

        if (settings != NULL &&
            strncmp(settings->ctid, ctx_id_value, DLT_ID_SIZE) == 0) {
            if (strlen(ctx_id_value) > 0) {
                dlt_vlog(LOG_WARNING,
                         "Appid %s with ctxid %s is already configured, skipping "
                         "duplicated entry\n",
                         app_id_value, ctx_id_value);
            } else {
                dlt_vlog(LOG_WARNING,
                         "Appid %s is already configured, skipping duplicated entry\n",
                         app_id_value);
            }

            continue;
        }

        /* allocate one more element in the trace load settings */
        DltDaemonContextLogSettings *tmp =
            realloc(daemon->app_id_log_level_settings,
                    (++daemon->num_app_id_log_level_settings) *
                        sizeof(DltDaemonContextLogSettings));

        if (tmp == NULL) {
            dlt_log(LOG_CRIT, "Failed to allocate memory for app load settings\n");
            continue;
        }

        daemon->app_id_log_level_settings = tmp;

        /* update newly created entry */
        settings = &daemon->app_id_log_level_settings
                        [daemon->num_app_id_log_level_settings -1];

        memset(settings, 0, sizeof(DltDaemonContextLogSettings));
        memcpy(settings->apid, app_id_value, DLT_ID_SIZE);
        memcpy(settings->ctid, ctx_id_value, DLT_ID_SIZE);
        settings->log_level = log_level_value;

        /* make sure ids are 0 terminated for printing */
        char apid_buf[DLT_ID_SIZE + 1] = {0};
        char ctid_buf[DLT_ID_SIZE + 1] = {0};
        memcpy(apid_buf, app_id_value, DLT_ID_SIZE);
        memcpy(ctid_buf, ctx_id_value, DLT_ID_SIZE);

        /* log with or without context id */
        if (strlen(ctid_buf) > 0) {
            dlt_vlog(
                LOG_INFO,
                "Configured trace limits for app id %s, context id %s, level %u\n",
                apid_buf, ctid_buf, log_level_value);
        } else {
            dlt_vlog(LOG_INFO, "Configured trace limits for app id %s, level %u\n",
                     apid_buf, log_level_value);
        }

    } /* while */
    fclose(pFile);

    /* list must be sorted to speed up dlt_daemon_find_configured_app_id_ctx_id_settings */
    qsort(daemon->app_id_log_level_settings,
            daemon->num_app_id_log_level_settings,
            sizeof(DltDaemonContextLogSettings), compare_app_id_conf);

    return 0;
}
#endif

static int dlt_mkdir_recursive(const char *dir)
{
    int ret = 0;
    char tmp[PATH_MAX + 1];
    char *p = NULL;
    char *end = NULL;
    size_t len;

    strncpy(tmp, dir, PATH_MAX);
    len = strlen(tmp);

    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;

    end = tmp + len;

    for (p = tmp + 1; ((*p) && (ret == 0)) || ((ret == -1 && errno == EEXIST) && (p != end)); p++)
        if (*p == '/') {
            *p = 0;

            if (access(tmp, F_OK) != 0 && errno == ENOENT) {
                ret = mkdir(tmp,
                #ifdef DLT_DAEMON_USE_FIFO_IPC
                                S_IRWXU);
                #else
                    S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH  | S_IWOTH /*S_IRWXU*/);
                #endif
            }

            *p = '/';
        }



    if ((ret == 0) || ((ret == -1) && (errno == EEXIST)))
        ret = mkdir(tmp,
        #ifdef DLT_DAEMON_USE_FIFO_IPC
                    S_IRWXU);
        #else
                    S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH  | S_IWOTH /*S_IRWXU*/);
        #endif

    if ((ret == -1) && (errno == EEXIST))
        ret = 0;

    return ret;
}

#ifdef DLT_DAEMON_USE_FIFO_IPC
static DltReturnValue dlt_daemon_create_pipes_dir(char *dir)
{
    int ret = DLT_RETURN_OK;

    if (dir == NULL) {
        dlt_vlog(LOG_ERR, "%s: Invalid parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* create dlt pipes directory */
    ret = mkdir(dir,
                S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH | S_ISVTX);

    if ((ret == -1) && (errno != EEXIST)) {
        dlt_vlog(LOG_ERR,
                 "FIFO user dir %s cannot be created (%s)!\n",
                 dir,
                 strerror(errno));

        return DLT_RETURN_ERROR;
    }

    /* S_ISGID cannot be set by mkdir, let's reassign right bits */
    ret = chmod(dir,
                S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH | S_ISGID |
                S_ISVTX);

    if (ret == -1) {
        dlt_vlog(LOG_ERR,
                 "FIFO user dir %s cannot be chmoded (%s)!\n",
                 dir,
                 strerror(errno));

        return DLT_RETURN_ERROR;
    }

    return ret;
}
#endif

/**
 * Main function of tool.
 */
int main(int argc, char *argv[])
{
    char version[DLT_DAEMON_TEXTBUFSIZE];
    char local_str[DLT_DAEMON_TEXTBUFSIZE];
    DltDaemonLocal daemon_local;
    DltDaemon daemon;
    int back = 0;

    memset(&daemon_local, 0, sizeof(DltDaemonLocal));
    memset(&daemon, 0, sizeof(DltDaemon));

    /* Command line option handling */
    if ((back = option_handling(&daemon_local, argc, argv)) < 0) {
        if (back != -2)
            fprintf (stderr, "option_handling() failed!\n");

        return -1;
    }

    /* Configuration file option handling */
    if ((back = option_file_parser(&daemon_local)) < 0) {
        if (back != -2)
            fprintf (stderr, "option_file_parser() failed!\n");

        return -1;
    }

    /* Initialize internal logging facility */
    dlt_log_set_filename(daemon_local.flags.loggingFilename);
    dlt_log_set_level(daemon_local.flags.loggingLevel);
    DltReturnValue log_init_result =
            dlt_log_init_multiple_logfiles_support(daemon_local.flags.loggingMode,
                                                   daemon_local.flags.enableLoggingFileLimit,
                                                   daemon_local.flags.loggingFileSize,
                                                   daemon_local.flags.loggingFileMaxSize);

    if (log_init_result != DLT_RETURN_OK) {
      fprintf(stderr, "Failed to init internal logging\n");

#if WITH_DLT_FILE_LOGGING_SYSLOG_FALLBACK
        if (daemon_local.flags.loggingMode == DLT_LOG_TO_FILE) {
          fprintf(stderr, "Falling back to syslog mode\n");

          daemon_local.flags.loggingMode = DLT_LOG_TO_SYSLOG;
          log_init_result = dlt_log_init(daemon_local.flags.loggingMode);
          if (log_init_result != DLT_RETURN_OK) {
            fprintf(stderr, "Failed to setup syslog logging, internal logs will "
                            "not be available\n");
          }
      }
#endif
    }

    /* Print version information */
    dlt_get_version(version, DLT_DAEMON_TEXTBUFSIZE);

    dlt_vlog(LOG_NOTICE, "Starting DLT Daemon; %s\n", version);

    PRINT_FUNCTION_VERBOSE(daemon_local.flags.vflag);

/* Make sure the parent user directory is created */
#ifdef DLT_DAEMON_USE_FIFO_IPC

    if (dlt_mkdir_recursive(dltFifoBaseDir) != 0) {
        dlt_vlog(LOG_ERR, "Base dir %s cannot be created!\n", dltFifoBaseDir);
        return -1;
  }

#else
    if (dlt_mkdir_recursive(DLT_USER_IPC_PATH) != 0) {
        dlt_vlog(LOG_ERR, "Base dir %s cannot be created!\n", daemon_local.flags.appSockPath);
        return -1;
    }

#endif

    /* --- Daemon init phase 1 begin --- */
    if (dlt_daemon_local_init_p1(&daemon, &daemon_local, daemon_local.flags.vflag) == -1) {
        dlt_log(LOG_CRIT, "Initialization of phase 1 failed!\n");
        return -1;
    }

    /* --- Daemon init phase 1 end --- */

    if (dlt_daemon_prepare_event_handling(&daemon_local.pEvent)) {
        /* TODO: Perform clean-up */
        dlt_log(LOG_CRIT, "Initialization of event handling failed!\n");
        return -1;
    }

    /* --- Daemon connection init begin */
    if (dlt_daemon_local_connection_init(&daemon, &daemon_local, daemon_local.flags.vflag) == -1) {
        dlt_log(LOG_CRIT, "Initialization of local connections failed!\n");
        return -1;
    }

    /* --- Daemon connection init end */

    if (dlt_daemon_init_runtime_configuration(&daemon, daemon_local.flags.ivalue, daemon_local.flags.vflag) == -1) {
        dlt_log(LOG_ERR, "Could not load runtime config\n");
        return -1;
    }

    /*
     * Load dlt-runtime.cfg if available.
     * This must be loaded before offline setup
     */
    dlt_daemon_configuration_load(&daemon, daemon.runtime_configuration, daemon_local.flags.vflag);

    /* --- Daemon init phase 2 begin --- */
    if (dlt_daemon_local_init_p2(&daemon, &daemon_local, daemon_local.flags.vflag) == -1) {
        dlt_log(LOG_CRIT, "Initialization of phase 2 failed!\n");
        return -1;
    }

#ifdef DLT_LOG_LEVEL_APP_CONFIG
    /* Load control app id level configuration file without setting `back` to
     * prevent exit if file is missing */
    if (app_id_default_log_level_config_parser(&daemon, &daemon_local) < 0) {
        dlt_vlog(LOG_WARNING, "app_id_default_log_level_config_parser() failed, "
                           "no app specific log levels will be configured\n");
    }
#endif

    /* --- Daemon init phase 2 end --- */

    if (daemon_local.flags.offlineLogstorageDirPath[0])
        if (dlt_daemon_logstorage_setup_internal_storage(
                &daemon,
                &daemon_local,
                daemon_local.flags.offlineLogstorageDirPath,
                daemon_local.flags.vflag) == -1)
            dlt_log(LOG_INFO,
                    "Setting up internal offline log storage failed!\n");

    /* create fd for watchdog */
#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
    {
        char *watchdogUSec = getenv("WATCHDOG_USEC");
        int watchdogTimeoutSeconds = 0;

        dlt_log(LOG_DEBUG, "Systemd watchdog initialization\n");

        if (watchdogUSec)
            watchdogTimeoutSeconds = atoi(watchdogUSec) / 2000000;

        watchdog_trigger_interval = watchdogTimeoutSeconds;
        create_timer_fd(&daemon_local,
                        watchdogTimeoutSeconds,
                        watchdogTimeoutSeconds,
                        DLT_TIMER_SYSTEMD);
    }
#endif

    /* create fd for timer timing packets */
    create_timer_fd(&daemon_local, 1, 1, DLT_TIMER_PACKET);

    /* create fd for timer ecu version */
    if ((daemon_local.flags.sendECUSoftwareVersion > 0) ||
        (daemon_local.flags.sendTimezone > 0))
        create_timer_fd(&daemon_local, 60, 60, DLT_TIMER_ECU);

    /* initiate gateway */
    if (daemon_local.flags.gatewayMode == 1) {
        if (dlt_gateway_init(&daemon_local, daemon_local.flags.vflag) == -1) {
            dlt_log(LOG_CRIT, "Failed to create gateway\n");
            return -1;
        }

        /* create gateway timer */
        create_timer_fd(&daemon_local,
                        daemon_local.pGateway.interval,
                        daemon_local.pGateway.interval,
                        DLT_TIMER_GATEWAY);
    }

    /* For offline tracing we still can use the same states */
    /* as for socket sending. Using this trick we see the traces */
    /* In the offline trace AND in the socket stream. */
    if (daemon_local.flags.yvalue[0])
        dlt_daemon_change_state(&daemon, DLT_DAEMON_STATE_SEND_DIRECT);
    else
        dlt_daemon_change_state(&daemon, DLT_DAEMON_STATE_BUFFER);

    dlt_daemon_init_user_information(&daemon,
                                     &daemon_local.pGateway,
                                     daemon_local.flags.gatewayMode,
                                     daemon_local.flags.vflag);

    /*
     * Check for app and ctx runtime cfg.
     * These cfg must be loaded after ecuId and num_user_lists are available
     */
    if ((dlt_daemon_applications_load(&daemon, daemon.runtime_application_cfg,
                                      daemon_local.flags.vflag) == 0) &&
        (dlt_daemon_contexts_load(&daemon, daemon.runtime_context_cfg,
                                  daemon_local.flags.vflag) == 0))
        daemon.runtime_context_cfg_loaded = 1;

    dlt_daemon_log_internal(&daemon,
                            &daemon_local,
                            "Daemon launched. Starting to output traces...",
                            daemon_local.flags.vflag);

    /* Even handling loop. */
    while ((back >= 0) && (g_exit >= 0))
        back = dlt_daemon_handle_event(&daemon_local.pEvent,
                                       &daemon,
                                       &daemon_local);

    snprintf(local_str, DLT_DAEMON_TEXTBUFSIZE, "Exiting DLT daemon... [%d]",
             g_signo);
    dlt_daemon_log_internal(&daemon, &daemon_local, local_str,
                            daemon_local.flags.vflag);
    dlt_vlog(LOG_NOTICE, "%s%s", local_str, "\n");

    dlt_daemon_local_cleanup(&daemon, &daemon_local, daemon_local.flags.vflag);

#ifdef UDP_CONNECTION_SUPPORT
    dlt_daemon_udp_close_connection();
#endif

    dlt_gateway_deinit(&daemon_local.pGateway, daemon_local.flags.vflag);

    dlt_daemon_free(&daemon, daemon_local.flags.vflag);

    dlt_log(LOG_NOTICE, "Leaving DLT daemon\n");

    dlt_log_free();

    return 0;

} /* main() */

int dlt_daemon_local_init_p1(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);
    int ret = DLT_RETURN_OK;

    if ((daemon == 0) || (daemon_local == 0)) {
        dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_local_init_p1()\n");
        return -1;
    }

#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE) || defined(DLT_SYSTEMD_ENABLE)
    ret = sd_booted();

    if (ret == 0) {
        dlt_log(LOG_CRIT, "System not booted with systemd!\n");
    }
    else if (ret < 0)
    {
        dlt_log(LOG_CRIT, "sd_booted failed!\n");
        return -1;
    }
    else {
        dlt_log(LOG_INFO, "System booted with systemd\n");
    }

#endif

#ifdef DLT_DAEMON_USE_FIFO_IPC

    if (dlt_daemon_create_pipes_dir(daemon_local->flags.userPipesDir) == DLT_RETURN_ERROR)
        return DLT_RETURN_ERROR;

#endif

    /* Check for daemon mode */
    if (daemon_local->flags.dflag)
        dlt_daemon_daemonize(daemon_local->flags.vflag);

    /* initialise structure to use DLT file */
    ret = dlt_file_init(&(daemon_local->file), daemon_local->flags.vflag);

    if (ret == DLT_RETURN_ERROR) {
        dlt_log(LOG_ERR, "Could not initialize file structure\n");
        /* Return value ignored, dlt daemon will exit */
        dlt_file_free(&(daemon_local->file), daemon_local->flags.vflag);
        return ret;
    }

    signal(SIGPIPE, SIG_IGN);

    signal(SIGTERM, dlt_daemon_signal_handler); /* software termination signal from kill */
    signal(SIGHUP, dlt_daemon_signal_handler);  /* hangup signal */
    signal(SIGQUIT, dlt_daemon_signal_handler);
    signal(SIGINT, dlt_daemon_signal_handler);
#ifdef __QNX__
    signal(SIGUSR1, dlt_daemon_signal_handler); /* for timer threads */
#endif

    return DLT_RETURN_OK;
}

int dlt_daemon_local_init_p2(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == 0) || (daemon_local == 0)) {
        dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_local_init_p2()\n");
        return -1;
    }

    /* Daemon data */
    if (dlt_daemon_init(daemon, daemon_local->RingbufferMinSize, daemon_local->RingbufferMaxSize,
                        daemon_local->RingbufferStepSize, daemon_local->flags.ivalue,
                        daemon_local->flags.contextLogLevel,
                        daemon_local->flags.contextTraceStatus, daemon_local->flags.enforceContextLLAndTS,
                        daemon_local->flags.vflag) == -1) {
        dlt_log(LOG_ERR, "Could not initialize daemon data\n");
        return -1;
    }

    /* init offline trace */
    if (((daemon->mode == DLT_USER_MODE_INTERNAL) || (daemon->mode == DLT_USER_MODE_BOTH)) &&
        daemon_local->flags.offlineTraceDirectory[0]) {
        if (multiple_files_buffer_init(&(daemon_local->offlineTrace),
                                       daemon_local->flags.offlineTraceDirectory,
                                       daemon_local->flags.offlineTraceFileSize,
                                       daemon_local->flags.offlineTraceMaxSize,
                                       daemon_local->flags.offlineTraceFilenameTimestampBased,
                                       false,
                                       DLT_OFFLINETRACE_FILENAME_BASE,
                                       DLT_OFFLINETRACE_FILENAME_EXT) == -1) {
            dlt_log(LOG_ERR, "Could not initialize offline trace\n");
            return -1;
        }
    }

    /* Init offline logstorage for MAX devices */
    if (daemon_local->flags.offlineLogstorageMaxDevices > 0) {
        daemon->storage_handle = malloc(sizeof(DltLogStorage) * daemon_local->flags.offlineLogstorageMaxDevices);

        if (daemon->storage_handle == NULL) {
            dlt_log(LOG_ERR, "Could not initialize offline logstorage\n");
            return -1;
        }

        memset(daemon->storage_handle, 0, (sizeof(DltLogStorage) * daemon_local->flags.offlineLogstorageMaxDevices));
    }

    /* Set ECU id of daemon */
    if (daemon_local->flags.evalue[0])
        dlt_set_id(daemon->ecuid, daemon_local->flags.evalue);
    else
        dlt_set_id(daemon->ecuid, DLT_DAEMON_ECU_ID);

    /* Set flag for optional sending of serial header */
    daemon->sendserialheader = daemon_local->flags.lflag;

#ifdef DLT_SHM_ENABLE

    /* init shared memory */
    if (dlt_shm_init_server(&(daemon_local->dlt_shm), daemon_local->flags.dltShmName,
                            daemon_local->flags.sharedMemorySize) == DLT_RETURN_ERROR) {
        dlt_log(LOG_ERR, "Could not initialize shared memory\n");
        return -1;
    }

    daemon_local->recv_buf_shm = (unsigned char *)calloc(1, DLT_SHM_RCV_BUFFER_SIZE);

    if (NULL == daemon_local->recv_buf_shm) {
        dlt_log(LOG_ERR, "failed to allocated the buffer to receive shm data\n");
        return -1;
    }

#endif

    /* prepare main loop */
    if (dlt_message_init(&(daemon_local->msg), daemon_local->flags.vflag) == DLT_RETURN_ERROR) {
        dlt_log(LOG_ERR, "Could not initialize message\n");
        return -1;
    }

    /* configure sending timing packets */
    if (daemon_local->flags.sendMessageTime)
        daemon->timingpackets = 1;

    /* Get ECU version info from a file. If it fails, use dlt_version as fallback. */
    if (dlt_daemon_local_ecu_version_init(daemon, daemon_local, daemon_local->flags.vflag) < 0) {
        daemon->ECUVersionString = malloc(DLT_DAEMON_TEXTBUFSIZE);

        if (daemon->ECUVersionString == 0) {
            dlt_log(LOG_WARNING, "Could not allocate memory for version string\n");
            return -1;
        }

        dlt_get_version(daemon->ECUVersionString, DLT_DAEMON_TEXTBUFSIZE);
    }

    /* Set to allows to maintain logstorage loglevel as default */
    daemon->maintain_logstorage_loglevel = DLT_MAINTAIN_LOGSTORAGE_LOGLEVEL_ON;

    return 0;
}

static int dlt_daemon_init_serial(DltDaemonLocal *daemon_local)
{
    /* create and open serial connection from/to client */
    /* open serial connection */
    int fd = -1;

    if (daemon_local->flags.yvalue[0] == '\0')
        return 0;

    fd = open(daemon_local->flags.yvalue, O_RDWR);

    if (fd < 0) {
        dlt_vlog(LOG_ERR, "Failed to open serial device %s\n",
                 daemon_local->flags.yvalue);

        daemon_local->flags.yvalue[0] = 0;
        return -1;
    }

    if (isatty(fd)) {
        int speed = DLT_DAEMON_SERIAL_DEFAULT_BAUDRATE;

        if (daemon_local->flags.bvalue[0])
            speed = atoi(daemon_local->flags.bvalue);

        daemon_local->baudrate = dlt_convert_serial_speed(speed);

        if (dlt_setup_serial(fd, (speed_t) daemon_local->baudrate) < 0) {
            close(fd);
            daemon_local->flags.yvalue[0] = 0;

            dlt_vlog(LOG_ERR, "Failed to configure serial device %s (%s) \n",
                     daemon_local->flags.yvalue, strerror(errno));

            return -1;
        }

        if (daemon_local->flags.vflag)
            dlt_log(LOG_DEBUG, "Serial init done\n");
    }
    else {
        close(fd);
        fprintf(stderr,
                "Device is not a serial device, device = %s (%s) \n",
                daemon_local->flags.yvalue,
                strerror(errno));
        daemon_local->flags.yvalue[0] = 0;
        return -1;
    }

    return dlt_connection_create(daemon_local,
                                 &daemon_local->pEvent,
                                 fd,
                                 POLLIN,
                                 DLT_CONNECTION_CLIENT_MSG_SERIAL);
}

#ifdef DLT_DAEMON_USE_FIFO_IPC
static int dlt_daemon_init_fifo(DltDaemonLocal *daemon_local)
{
    int ret;
    int fd = -1;
    int fifo_size;

    /* open named pipe(FIFO) to receive DLT messages from users */
    umask(0);

    /* Try to delete existing pipe, ignore result of unlink */
    const char *tmpFifo = daemon_local->flags.daemonFifoName;
    unlink(tmpFifo);

    ret = mkfifo(tmpFifo, S_IRUSR | S_IWUSR | S_IWGRP);

    if (ret == -1) {
        dlt_vlog(LOG_WARNING, "FIFO user %s cannot be created (%s)!\n",
                 tmpFifo, strerror(errno));
        return -1;
    } /* if */

    /* Set group of daemon FIFO */
    if (daemon_local->flags.daemonFifoGroup[0] != 0) {
        errno = 0;
        struct group *group_dlt = getgrnam(daemon_local->flags.daemonFifoGroup);

        if (group_dlt) {
            ret = chown(tmpFifo, -1, group_dlt->gr_gid);

            if (ret == -1)
                dlt_vlog(LOG_ERR, "FIFO user %s cannot be chowned to group %s (%s)\n",
                         tmpFifo, daemon_local->flags.daemonFifoGroup,
                         strerror(errno));
        }
        else if ((errno == 0) || (errno == ENOENT) || (errno == EBADF) || (errno == EPERM))
        {
            dlt_vlog(LOG_ERR, "Group name %s is not found (%s)\n",
                     daemon_local->flags.daemonFifoGroup,
                     strerror(errno));
        }
        else {
            dlt_vlog(LOG_ERR, "Failed to get group id of %s (%s)\n",
                     daemon_local->flags.daemonFifoGroup,
                     strerror(errno));
        }
    }

    fd = open(tmpFifo, O_RDWR);

    if (fd == -1) {
        dlt_vlog(LOG_WARNING, "FIFO user %s cannot be opened (%s)!\n",
                 tmpFifo, strerror(errno));
        return -1;
    } /* if */

#ifdef __linux__
    /* F_SETPIPE_SZ and F_GETPIPE_SZ are only supported for Linux.
     * For other OSes it depends on its system e.g. pipe manager.
     */
    if (daemon_local->daemonFifoSize != 0) {
        /* Set Daemon FIFO size */
        if (fcntl(fd, F_SETPIPE_SZ, daemon_local->daemonFifoSize) == -1)
            dlt_vlog(LOG_ERR, "set FIFO size error: %s\n", strerror(errno));
    }

    /* Get Daemon FIFO size */
    if ((fifo_size = fcntl(fd, F_GETPIPE_SZ, 0)) == -1)
        dlt_vlog(LOG_ERR, "get FIFO size error: %s\n", strerror(errno));
    else
        dlt_vlog(LOG_INFO, "FIFO size: %d\n", fifo_size);
#endif

    /* Early init, to be able to catch client (app) connections
     * as soon as possible. This registration is automatically ignored
     * during next execution.
     */
    return dlt_connection_create(daemon_local,
                                 &daemon_local->pEvent,
                                 fd,
                                 POLLIN,
                                 DLT_CONNECTION_APP_MSG);
}
#endif

#ifdef DLT_DAEMON_VSOCK_IPC_ENABLE
static int dlt_daemon_init_vsock(DltDaemonLocal *daemon_local)
{
    int fd;
    struct sockaddr_vm addr;

    fd = socket(AF_VSOCK, SOCK_STREAM, 0);
    if (fd == -1) {
        dlt_vlog(LOG_ERR, "Failed to create VSOCK socket: %s\n", strerror(errno));
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.svm_family = AF_VSOCK;
    addr.svm_port = DLT_VSOCK_PORT;
    addr.svm_cid = VMADDR_CID_ANY;

    if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
        dlt_vlog(LOG_ERR, "Failed to bind VSOCK socket: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    if (listen(fd, 1) != 0) {
        dlt_vlog(LOG_ERR, "Failed to listen on VSOCK socket: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    return dlt_connection_create(daemon_local,
                                 &daemon_local->pEvent,
                                 fd,
                                 POLLIN,
                                 DLT_CONNECTION_APP_CONNECT);
}
#endif

#ifdef DLT_DAEMON_USE_UNIX_SOCKET_IPC
static DltReturnValue dlt_daemon_init_app_socket(DltDaemonLocal *daemon_local)
{
    /* socket access permission set to srw-rw-rw- (666) */
    int mask = S_IXUSR | S_IXGRP | S_IXOTH;
    DltReturnValue ret = DLT_RETURN_OK;
    int fd = -1;

    if (daemon_local == NULL) {
        dlt_vlog(LOG_ERR, "%s: Invalid function parameters\n", __func__);
        return DLT_RETURN_ERROR;
    }

#ifdef ANDROID
    /* on android if we want to use security contexts on Unix sockets,
     * they should be created by init (see dlt-daemon.rc in src/daemon)
     * and recovered through the below API */
    ret = dlt_daemon_unix_android_get_socket(&fd, daemon_local->flags.appSockPath);
    if (ret < DLT_RETURN_OK) {
        /* we failed to get app socket created by init.
         * To avoid blocking users to launch dlt-daemon only through
         * init on android (e.g: by hand for debugging purpose), try to
         * create app socket by ourselves */
        ret = dlt_daemon_unix_socket_open(&fd,
                                          daemon_local->flags.appSockPath,
                                          SOCK_STREAM,
                                          mask);
    }
#else
    ret = dlt_daemon_unix_socket_open(&fd,
                                      daemon_local->flags.appSockPath,
                                      SOCK_STREAM,
                                      mask);
#endif
    if (ret == DLT_RETURN_OK) {
        if (dlt_connection_create(daemon_local,
                                  &daemon_local->pEvent,
                                  fd,
                                  POLLIN,
                                  DLT_CONNECTION_APP_CONNECT)) {
            dlt_log(LOG_CRIT, "Could not create connection for app socket.\n");
            return DLT_RETURN_ERROR;
        }
    }
    else {
        dlt_log(LOG_CRIT, "Could not create and open app socket.\n");
        return DLT_RETURN_ERROR;
    }

    return ret;
}
#endif

static DltReturnValue dlt_daemon_initialize_control_socket(DltDaemonLocal *daemon_local)
{
    /* socket access permission set to srw-rw---- (660)  */
    int mask = S_IXUSR | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH;
    DltReturnValue ret = DLT_RETURN_OK;
    int fd = -1;

    if (daemon_local == NULL) {
        dlt_vlog(LOG_ERR, "%s: Invalid function parameters\n", __func__);
        return -1;
    }

#ifdef ANDROID
    /* on android if we want to use security contexts on Unix sockets,
     * they should be created by init (see dlt-daemon.rc in src/daemon)
     * and recovered through the below API */
    ret = dlt_daemon_unix_android_get_socket(&fd, daemon_local->flags.ctrlSockPath);
    if (ret < DLT_RETURN_OK) {
        /* we failed to get app socket created by init.
         * To avoid blocking users to launch dlt-daemon only through
         * init on android (e.g by hand for debugging purpose), try to
         * create app socket by ourselves */
        ret = dlt_daemon_unix_socket_open(&fd,
                                          daemon_local->flags.ctrlSockPath,
                                          SOCK_STREAM,
                                          mask);
    }
#else
    ret = dlt_daemon_unix_socket_open(&fd,
                                      daemon_local->flags.ctrlSockPath,
                                      SOCK_STREAM,
                                      mask);
#endif
    if (ret == DLT_RETURN_OK) {
        if (dlt_connection_create(daemon_local,
                                  &daemon_local->pEvent,
                                  fd,
                                  POLLIN,
                                  DLT_CONNECTION_CONTROL_CONNECT) < DLT_RETURN_OK) {
            dlt_log(LOG_ERR, "Could not initialize control socket.\n");
            ret = DLT_RETURN_ERROR;
        }
    }

    return ret;
}

int dlt_daemon_local_connection_init(DltDaemon *daemon,
                                     DltDaemonLocal *daemon_local,
                                     int verbose)
{
    int fd = -1;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL)) {
        dlt_vlog(LOG_ERR, "%s: Invalid function parameters\n", __func__);
        return -1;
    }

    DltBindAddress_t *head = daemon_local->flags.ipNodes;

#ifdef DLT_DAEMON_USE_UNIX_SOCKET_IPC
    /* create and open socket to receive incoming connections from user application */
    if (dlt_daemon_init_app_socket(daemon_local) < DLT_RETURN_OK) {
        dlt_log(LOG_ERR, "Unable to initialize app socket.\n");
        return DLT_RETURN_ERROR;
    }

#else /* DLT_DAEMON_USE_FIFO_IPC */

    if (dlt_daemon_init_fifo(daemon_local)) {
        dlt_log(LOG_ERR, "Unable to initialize fifo.\n");
        return DLT_RETURN_ERROR;
    }

#endif

#ifdef DLT_DAEMON_VSOCK_IPC_ENABLE
    if (dlt_daemon_init_vsock(daemon_local) != 0) {
        dlt_log(LOG_ERR, "Unable to initialize app VSOCK socket.\n");
        return DLT_RETURN_ERROR;
    }
#endif

    /* create and open socket to receive incoming connections from client */
    daemon_local->client_connections = 0;

    if (head == NULL) { /* no IP set in BindAddress option, will use "0.0.0.0" as default */

        if (dlt_daemon_socket_open(&fd, daemon_local->flags.port, "0.0.0.0") == DLT_RETURN_OK) {
            if (dlt_connection_create(daemon_local,
                                      &daemon_local->pEvent,
                                      fd,
                                      POLLIN,
                                      DLT_CONNECTION_CLIENT_CONNECT)) {
                dlt_log(LOG_ERR, "Could not initialize main socket.\n");
                return DLT_RETURN_ERROR;
            }
        }
        else {
            dlt_log(LOG_ERR, "Could not initialize main socket.\n");
            return DLT_RETURN_ERROR;
        }
    }
    else {
        bool any_open = false;
        while (head != NULL) { /* open socket for each IP in the bindAddress list */

            if (dlt_daemon_socket_open(&fd, daemon_local->flags.port, head->ip) == DLT_RETURN_OK) {
                if (dlt_connection_create(daemon_local,
                                          &daemon_local->pEvent,
                                          fd,
                                          POLLIN,
                                          DLT_CONNECTION_CLIENT_CONNECT)) {
                    dlt_vlog(LOG_ERR, "Could not create connection, for binding %s\n", head->ip);
                } else {
                    any_open = true;
                }
            }
            else {
                dlt_vlog(LOG_ERR, "Could not open main socket, for binding %s\n", head->ip);
            }

            head = head->next;
        }

        if (!any_open) {
            dlt_vlog(LOG_ERR, "Failed create main socket for any configured binding\n");
            return DLT_RETURN_ERROR;
        }
    }

#ifdef UDP_CONNECTION_SUPPORT

    if (daemon_local->UDPConnectionSetup == MULTICAST_CONNECTION_ENABLED) {
        if (dlt_daemon_udp_connection_setup(daemon_local) < 0) {
            dlt_log(LOG_ERR, "UDP fd creation failed\n");
            return DLT_RETURN_ERROR;
        }
        else {
            dlt_log(LOG_INFO, "UDP fd creation success\n");
        }
    }

#endif

    /* create and open unix socket to receive incoming connections from
     * control application */
    if (dlt_daemon_initialize_control_socket(daemon_local) < DLT_RETURN_OK) {
        dlt_log(LOG_ERR, "Could not initialize control socket.\n");
        return DLT_RETURN_ERROR;
    }

    /* Init serial */
    if (dlt_daemon_init_serial(daemon_local) < 0) {
        dlt_log(LOG_ERR, "Could not initialize daemon data\n");
        return DLT_RETURN_ERROR;
    }

    return 0;
}

int dlt_daemon_local_ecu_version_init(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    char *version = NULL;
    FILE *f = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    /* By default, version string is null. */
    daemon->ECUVersionString = NULL;

    /* Open the file. Bail out if error occurs */
    f = fopen(daemon_local->flags.pathToECUSoftwareVersion, "r");

    if (f == NULL) {
        /* Error level notice, because this might be deliberate choice */
        dlt_log(LOG_NOTICE, "Failed to open ECU Software version file.\n");
        return -1;
    }

    /* Get the file size. Bail out if stat fails. */
    int fd = fileno(f);
    struct stat s_buf;

    if (fstat(fd, &s_buf) < 0) {
        dlt_log(LOG_WARNING, "Failed to stat ECU Software version file.\n");
        fclose(f);
        return -1;
    }

    /* Bail out if file is too large. Use DLT_DAEMON_TEXTBUFSIZE max.
     * Reserve one byte for trailing '\0' */
    off_t size = s_buf.st_size;

    if (size >= DLT_DAEMON_TEXTBUFSIZE) {
        dlt_log(LOG_WARNING, "Too large file for ECU version.\n");
        fclose(f);
        return -1;
    }

    /* Allocate permanent buffer for version info */
    version = malloc((size_t) (size + 1));

    if (version == 0) {
        dlt_log(LOG_WARNING, "Cannot allocate memory for ECU version.\n");
        fclose(f);
        return -1;
    }

    off_t offset = 0;

    while (!feof(f)) {
        offset += (off_t) fread(version + offset, 1, (size_t) size, f);

        if (ferror(f)) {
            dlt_log(LOG_WARNING, "Failed to read ECU Software version file.\n");
            free(version);
            fclose(f);
            return -1;
        }

        if (offset > size) {
            dlt_log(LOG_WARNING, "Too long file for ECU Software version info.\n");
            free(version);
            fclose(f);
            return -1;
        }
    }

    version[offset] = '\0';/*append null termination at end of version string */
    daemon->ECUVersionString = version;
    fclose(f);
    return 0;
}

void dlt_daemon_local_cleanup(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == 0) || (daemon_local == 0)) {
        dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_local_cleanup()\n");
        return;
    }

    /* Don't receive event anymore */
    dlt_event_handler_cleanup_connections(&daemon_local->pEvent);

    dlt_message_free(&(daemon_local->msg), daemon_local->flags.vflag);

    /* free shared memory */
    if (daemon_local->flags.offlineTraceDirectory[0])
        multiple_files_buffer_free(&(daemon_local->offlineTrace));

    /* Ignore result */
    dlt_file_free(&(daemon_local->file), daemon_local->flags.vflag);

#ifdef DLT_DAEMON_USE_FIFO_IPC
    /* Try to delete existing pipe, ignore result of unlink() */
    unlink(daemon_local->flags.daemonFifoName);
#else /* DLT_DAEMON_USE_UNIX_SOCKET_IPC */
    /* Try to delete existing pipe, ignore result of unlink() */
    unlink(daemon_local->flags.appSockPath);
#endif

#ifdef DLT_SHM_ENABLE
    /* free shared memory */
    dlt_shm_free_server(&(daemon_local->dlt_shm), daemon_local->flags.dltShmName);
    free(daemon_local->recv_buf_shm);
    daemon_local->recv_buf_shm = NULL;
#endif

    if (daemon_local->flags.offlineLogstorageMaxDevices > 0) {
        /* disconnect all logstorage devices */
        dlt_daemon_logstorage_cleanup(daemon,
                                      daemon_local,
                                      daemon_local->flags.vflag);

        free(daemon->storage_handle);
    }

    if (daemon->ECUVersionString != NULL)
        free(daemon->ECUVersionString);

    unlink(daemon_local->flags.ctrlSockPath);

    /* free IP list */
    free(daemon_local->flags.ipNodes);
}

void dlt_daemon_exit_trigger()
{
    /* stop event loop */
    g_exit = -1;

#ifdef DLT_DAEMON_USE_FIFO_IPC
    char tmp[DLT_PATH_MAX] = { 0 };

    ssize_t n;
    n = snprintf(tmp, DLT_PATH_MAX, "%s/dlt", dltFifoBaseDir);
    if (n < 0 || (size_t)n > DLT_PATH_MAX) {
        dlt_vlog(LOG_WARNING, "%s: snprintf truncation/error(%ld) %s\n",
                __func__, n, tmp);
    }

    (void)unlink(tmp);
#endif

#ifdef __QNX__
    dlt_daemon_cleanup_timers();
#endif

}

void dlt_daemon_signal_handler(int sig)
{
    g_signo = sig;

    switch (sig) {
        case SIGHUP:
        case SIGTERM:
        case SIGINT:
        case SIGQUIT:
        {
            /* finalize the server */
            dlt_vlog(LOG_NOTICE, "Exiting DLT daemon due to signal: %s\n",
                     strsignal(sig));
            dlt_daemon_exit_trigger();
            break;
        }
        default:
        {
            /* This case should never happen! */
            break;
        }
    } /* switch */

} /* dlt_daemon_signal_handler() */

#ifdef __QNX__
void dlt_daemon_cleanup_timers()
{
    int i = 0;
    while (i < DLT_TIMER_UNKNOWN) {
        /* Remove FIFO of every timer and kill timer thread */
        if (0 != timer_threads[i]) {
            pthread_kill(timer_threads[i], SIGUSR1);
            pthread_join(timer_threads[i], NULL);
            timer_threads[i] = 0;

            close_pipes(dlt_timer_pipes[i]);

            /* Free data of every timer */
            if (NULL != timer_data[i]) {
                free(timer_data[i]);
                timer_data[i] = NULL;
            }
        }
        i++;
    }
}
#endif

void dlt_daemon_daemonize(int verbose)
{
    int i;
    int fd;

    PRINT_FUNCTION_VERBOSE(verbose);

    dlt_log(LOG_NOTICE, "Daemon mode\n");

    /* Daemonize */
    i = fork();

    if (i < 0) {
        dlt_log(LOG_CRIT, "Unable to fork(), exiting DLT daemon\n");
        exit(-1); /* fork error */
    }

    if (i > 0)
        exit(0); /* parent exits */

    /* child (daemon) continues */

    /* Process independency */

    /* obtain a new process group */
    if (setsid() == -1) {
        dlt_log(LOG_CRIT, "setsid() failed, exiting DLT daemon\n");
        exit(-1); /* fork error */
    }

    /* Open standard descriptors stdin, stdout, stderr */
    fd = open("/dev/null", O_RDWR);

    if (fd != -1) {
        /* Redirect STDOUT to /dev/null */
        if (dup2(fd, STDOUT_FILENO) < 0)
            dlt_vlog(LOG_WARNING, "Failed to direct stdout to /dev/null. Error: %s\n", strerror(errno));

        /* Redirect STDERR to /dev/null */
        if (dup2(fd, STDERR_FILENO) < 0)
            dlt_vlog(LOG_WARNING, "Failed to direct stderr to /dev/null. Error: %s\n", strerror(errno));

        close(fd);
    }
    else {
        dlt_log(LOG_CRIT, "Error opening /dev/null, exiting DLT daemon\n");
        exit(-1); /* fork error */
    }

    /* Set umask */
    umask(DLT_DAEMON_UMASK);

    /* Change to root directory */
    if (chdir("/") < 0)
        dlt_log(LOG_WARNING, "Failed to chdir to root\n");

    /* Catch signals */
    signal(SIGCHLD, SIG_IGN); /* ignore child */
    signal(SIGTSTP, SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

} /* dlt_daemon_daemonize() */

/* This function logs str to the configured output sink (socket, serial, offline trace).
 * To avoid recursion this function must be called only from DLT highlevel functions.
 * E. g. calling it to output a failure when the open of the offline trace file fails
 * would cause an endless loop because dlt_daemon_log_internal() would itself again try
 * to open the offline trace file.
 * This is a dlt-daemon only function. The libdlt has no equivalent function available. */
int dlt_daemon_log_internal(DltDaemon *daemon, DltDaemonLocal *daemon_local, char *str, int verbose)
{
    DltMessage msg = { 0 };
    static uint8_t uiMsgCount = 0;
    DltStandardHeaderExtra *pStandardExtra = NULL;
    uint32_t uiType;
    uint16_t uiSize;
    uint32_t uiExtraSize;

    PRINT_FUNCTION_VERBOSE(verbose);

    /* Set storageheader */
    msg.storageheader = (DltStorageHeader *)(msg.headerbuffer);
    dlt_set_storageheader(msg.storageheader, daemon->ecuid);

    /* Set standardheader */
    msg.standardheader = (DltStandardHeader *)(msg.headerbuffer + sizeof(DltStorageHeader));
    msg.standardheader->htyp = DLT_HTYP_UEH | DLT_HTYP_WEID | DLT_HTYP_WSID | DLT_HTYP_WTMS |
        DLT_HTYP_PROTOCOL_VERSION1;
    msg.standardheader->mcnt = uiMsgCount++;

    uiExtraSize = (uint32_t) (DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp) +
        (DLT_IS_HTYP_UEH(msg.standardheader->htyp) ? sizeof(DltExtendedHeader) : 0));
    msg.headersize = (uint32_t) sizeof(DltStorageHeader) + (uint32_t) sizeof(DltStandardHeader) + uiExtraSize;

    /* Set extraheader */
    pStandardExtra =
        (DltStandardHeaderExtra *)(msg.headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader));
    dlt_set_id(pStandardExtra->ecu, daemon->ecuid);
    pStandardExtra->tmsp = DLT_HTOBE_32(dlt_uptime());
    pStandardExtra->seid = (unsigned int) DLT_HTOBE_32(getpid());

    /* Set extendedheader */
    msg.extendedheader =
        (DltExtendedHeader *)(msg.headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader) +
                              DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp));
    msg.extendedheader->msin = DLT_MSIN_VERB | (DLT_TYPE_LOG << DLT_MSIN_MSTP_SHIFT) |
        ((DLT_LOG_INFO << DLT_MSIN_MTIN_SHIFT) & DLT_MSIN_MTIN);
    msg.extendedheader->noar = 1;
    dlt_set_id(msg.extendedheader->apid, "DLTD");
    dlt_set_id(msg.extendedheader->ctid, "INTM");

    /* Set payload data... */
    uiType = DLT_TYPE_INFO_STRG;
    uiSize = (uint16_t) (strlen(str) + 1);
    msg.datasize = (uint32_t) (sizeof(uint32_t) + sizeof(uint16_t) + uiSize);

    msg.databuffer = (uint8_t *)malloc((size_t) msg.datasize);
    msg.databuffersize = msg.datasize;

    if (msg.databuffer == 0) {
        dlt_log(LOG_WARNING, "Can't allocate buffer for get log info message\n");
        return -1;
    }

    msg.datasize = 0;
    memcpy((uint8_t *)(msg.databuffer + msg.datasize), (uint8_t *)(&uiType), sizeof(uint32_t));
    msg.datasize += (uint32_t) sizeof(uint32_t);
    memcpy((uint8_t *)(msg.databuffer + msg.datasize), (uint8_t *)(&uiSize), sizeof(uint16_t));
    msg.datasize += (uint32_t) sizeof(uint16_t);
    memcpy((uint8_t *)(msg.databuffer + msg.datasize), str, uiSize);
    msg.datasize += uiSize;

    /* Calc length */
    msg.standardheader->len = DLT_HTOBE_16(msg.headersize - sizeof(DltStorageHeader) + msg.datasize);

    dlt_daemon_client_send(DLT_DAEMON_SEND_TO_ALL, daemon,daemon_local,
                           msg.headerbuffer, sizeof(DltStorageHeader),
                           msg.headerbuffer + sizeof(DltStorageHeader),
                           (int) (msg.headersize - sizeof(DltStorageHeader)),
                           msg.databuffer, (int) msg.datasize, verbose);

    free(msg.databuffer);

    return 0;
}

int dlt_daemon_check_numeric_setting(char *token,
                                    char *value,
                                    unsigned long *data)
{
    char value_check[value_length];
    value_check[0] = 0;
    sscanf(value, "%lu%s", data, value_check);
    if (value_check[0] || !isdigit(value[0])) {
        fprintf(stderr, "Invalid input [%s] detected in option %s\n",
                value,
                token);
        return -1;
    }
    return 0;
}

int dlt_daemon_process_client_connect(DltDaemon *daemon,
                                      DltDaemonLocal *daemon_local,
                                      DltReceiver *receiver,
                                      int verbose)
{
    socklen_t cli_size;
    struct sockaddr_un cli;

    int in_sock = -1;
    char local_str[DLT_DAEMON_TEXTBUFSIZE] = { '\0' };

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (receiver == NULL)) {
        dlt_log(LOG_ERR,
                "Invalid function parameters used for function "
                "dlt_daemon_process_client_connect()\n");
        return -1;
    }

    /* event from TCP server socket, new connection */
    cli_size = sizeof(cli);

    if ((in_sock = accept(receiver->fd, (struct sockaddr *)&cli, &cli_size)) < 0) {
        if (errno == ECONNABORTED) // Caused by nmap -v -p 3490 -Pn <IP of dlt-daemon>
            return 0;
        dlt_vlog(LOG_ERR, "accept() for socket %d failed: %s\n", receiver->fd, strerror(errno));
        return -1;
    }

    /* check if file file descriptor was already used, and make it invalid if it
     * is reused. */
    /* This prevents sending messages to wrong file descriptor */
    dlt_daemon_applications_invalidate_fd(daemon, daemon->ecuid, in_sock, verbose);
    dlt_daemon_contexts_invalidate_fd(daemon, daemon->ecuid, in_sock, verbose);

    /* Set socket timeout in reception */
    struct timeval timeout_send;
    timeout_send.tv_sec = daemon_local->timeoutOnSend;
    timeout_send.tv_usec = 0;

    if (setsockopt (in_sock,
                    SOL_SOCKET,
                    SO_SNDTIMEO,
                    (char *)&timeout_send,
                    sizeof(timeout_send)) < 0)
        dlt_log(LOG_WARNING, "setsockopt failed\n");

    if (dlt_connection_create(daemon_local,
                              &daemon_local->pEvent,
                              in_sock,
                              POLLIN,
                              DLT_CONNECTION_CLIENT_MSG_TCP)) {
        dlt_log(LOG_ERR, "Failed to register new client. \n");
        /* TODO: Perform clean-up */
        return -1;
    }

    /* send connection info about connected */
    dlt_daemon_control_message_connection_info(in_sock,
                                               daemon,
                                               daemon_local,
                                               DLT_CONNECTION_STATUS_CONNECTED,
                                               "",
                                               verbose);

    /* send ecu version string */
    if (daemon_local->flags.sendECUSoftwareVersion > 0) {
        if (daemon_local->flags.sendECUSoftwareVersion > 0)
            dlt_daemon_control_get_software_version(DLT_DAEMON_SEND_TO_ALL,
                                                    daemon,
                                                    daemon_local,
                                                    daemon_local->flags.vflag);

        if (daemon_local->flags.sendTimezone > 0)
            dlt_daemon_control_message_timezone(DLT_DAEMON_SEND_TO_ALL,
                                                daemon,
                                                daemon_local,
                                                daemon_local->flags.vflag);
    }

    snprintf(local_str, DLT_DAEMON_TEXTBUFSIZE,
             "New client connection #%d established, Total Clients : %d",
             in_sock, daemon_local->client_connections);

    dlt_daemon_log_internal(daemon, daemon_local, local_str,
                            daemon_local->flags.vflag);
    dlt_vlog(LOG_DEBUG, "%s%s", local_str, "\n");

    if (daemon_local->client_connections == 1) {
        if (daemon_local->flags.vflag)
            dlt_log(LOG_DEBUG, "Send ring-buffer to client\n");

        dlt_daemon_change_state(daemon, DLT_DAEMON_STATE_SEND_BUFFER);

        if (dlt_daemon_send_ringbuffer_to_client(daemon, daemon_local, verbose) == -1) {
            dlt_log(LOG_WARNING, "Can't send contents of ringbuffer to clients\n");
            close(in_sock);
            in_sock = -1;
            return -1;
        }

        /* send new log state to all applications */
        daemon->connectionState = 1;
        dlt_daemon_user_send_all_log_state(daemon, verbose);
    }

    return 0;
}

int dlt_daemon_process_client_messages(DltDaemon *daemon,
                                       DltDaemonLocal *daemon_local,
                                       DltReceiver *receiver,
                                       int verbose)
{
    int bytes_to_be_removed = 0;
    int must_close_socket = -1;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (receiver == NULL)) {
        dlt_log(LOG_ERR,
                "Invalid function parameters used for function "
                "dlt_daemon_process_client_messages()\n");
        return -1;
    }

    must_close_socket = dlt_receiver_receive(receiver);

    if (must_close_socket < 0) {
        dlt_daemon_close_socket(receiver->fd,
                                daemon,
                                daemon_local,
                                verbose);
        return -1;
    }

    /* Process all received messages */
    while (dlt_message_read(&(daemon_local->msg),
                            (uint8_t *)receiver->buf,
                            (unsigned int) receiver->bytesRcvd,
                            daemon_local->flags.nflag,
                            daemon_local->flags.vflag) == DLT_MESSAGE_ERROR_OK) {
        /* Check for control message */
        if ((0 < receiver->fd) &&
            DLT_MSG_IS_CONTROL_REQUEST(&(daemon_local->msg)))
            dlt_daemon_client_process_control(receiver->fd,
                                              daemon,
                                              daemon_local,
                                              &(daemon_local->msg),
                                              daemon_local->flags.vflag);

        bytes_to_be_removed = (int) (daemon_local->msg.headersize +
            daemon_local->msg.datasize -
            sizeof(DltStorageHeader));

        if (daemon_local->msg.found_serialheader)
            bytes_to_be_removed += (int) sizeof(dltSerialHeader);

        if (daemon_local->msg.resync_offset)
            bytes_to_be_removed += daemon_local->msg.resync_offset;

        if (dlt_receiver_remove(receiver, bytes_to_be_removed) == -1) {
            dlt_log(LOG_WARNING,
                    "Can't remove bytes from receiver for sockets\n");
            return -1;
        }
    } /* while */

    if (dlt_receiver_move_to_begin(receiver) == -1) {
        dlt_log(LOG_WARNING,
                "Can't move bytes to beginning of receiver buffer for sockets\n");
        return -1;
    }

    if (must_close_socket == 0)
        /* FIXME: Why the hell do we need to close the socket
         * on control message reception ??
         */
        dlt_daemon_close_socket(receiver->fd,
                                daemon,
                                daemon_local,
                                verbose);

    return 0;
}

int dlt_daemon_process_client_messages_serial(DltDaemon *daemon,
                                              DltDaemonLocal *daemon_local,
                                              DltReceiver *receiver,
                                              int verbose)
{
    int bytes_to_be_removed = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (receiver == NULL)) {
        dlt_log(LOG_ERR,
                "Invalid function parameters used for function "
                "dlt_daemon_process_client_messages_serial()\n");
        return -1;
    }

    if (dlt_receiver_receive(receiver) <= 0) {
        dlt_log(LOG_WARNING,
                "dlt_receiver_receive_fd() for messages from serial interface "
                "failed!\n");
        return -1;
    }

    /* Process all received messages */
    while (dlt_message_read(&(daemon_local->msg),
                            (uint8_t *)receiver->buf,
                            (unsigned int) receiver->bytesRcvd,
                            daemon_local->flags.mflag,
                            daemon_local->flags.vflag) == DLT_MESSAGE_ERROR_OK) {
        /* Check for control message */
        if (DLT_MSG_IS_CONTROL_REQUEST(&(daemon_local->msg))) {
            if (dlt_daemon_client_process_control(receiver->fd,
                                                  daemon,
                                                  daemon_local,
                                                  &(daemon_local->msg),
                                                  daemon_local->flags.vflag)
                == -1) {
                dlt_log(LOG_WARNING, "Can't process control messages\n");
                return -1;
            }
        }

        bytes_to_be_removed = (int) (daemon_local->msg.headersize +
            daemon_local->msg.datasize -
            sizeof(DltStorageHeader));

        if (daemon_local->msg.found_serialheader)
            bytes_to_be_removed += (int) sizeof(dltSerialHeader);

        if (daemon_local->msg.resync_offset)
            bytes_to_be_removed += daemon_local->msg.resync_offset;

        if (dlt_receiver_remove(receiver, bytes_to_be_removed) == -1) {
            dlt_log(LOG_WARNING,
                    "Can't remove bytes from receiver for serial connection\n");
            return -1;
        }
    } /* while */

    if (dlt_receiver_move_to_begin(receiver) == -1) {
        dlt_log(LOG_WARNING,
                "Can't move bytes to beginning of receiver buffer for serial "
                "connection\n");
        return -1;
    }

    return 0;
}

int dlt_daemon_process_control_connect(
    DltDaemon *daemon,
    DltDaemonLocal *daemon_local,
    DltReceiver *receiver,
    int verbose)
{
    socklen_t ctrl_size;
    struct sockaddr_un ctrl;
    int in_sock = -1;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (receiver == NULL)) {
        dlt_log(LOG_ERR,
                "Invalid function parameters used for function "
                "dlt_daemon_process_control_connect()\n");
        return -1;
    }

    /* event from UNIX server socket, new connection */
    ctrl_size = sizeof(ctrl);

    if ((in_sock = accept(receiver->fd, (struct sockaddr *)&ctrl, &ctrl_size)) < 0) {
        dlt_vlog(LOG_ERR, "accept() on UNIX control socket %d failed: %s\n", receiver->fd, strerror(errno));
        return -1;
    }

    /* check if file file descriptor was already used, and make it invalid if it
     *  is reused */
    /* This prevents sending messages to wrong file descriptor */
    dlt_daemon_applications_invalidate_fd(daemon, daemon->ecuid, in_sock, verbose);
    dlt_daemon_contexts_invalidate_fd(daemon, daemon->ecuid, in_sock, verbose);

    if (dlt_connection_create(daemon_local,
                              &daemon_local->pEvent,
                              in_sock,
                              POLLIN,
                              DLT_CONNECTION_CONTROL_MSG)) {
        dlt_log(LOG_ERR, "Failed to register new client. \n");
        /* TODO: Perform clean-up */
        return -1;
    }

    if (verbose)
        dlt_vlog(LOG_INFO, "New connection to control client established\n");

    return 0;
}

#if defined DLT_DAEMON_USE_UNIX_SOCKET_IPC || defined DLT_DAEMON_VSOCK_IPC_ENABLE
int dlt_daemon_process_app_connect(
    DltDaemon *daemon,
    DltDaemonLocal *daemon_local,
    DltReceiver *receiver,
    int verbose)
{
    int in_sock = -1;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (receiver == NULL)) {
        dlt_vlog(LOG_ERR,
                 "%s: Invalid parameters\n",
                 __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* event from server socket, new connection */

    if ((in_sock = accept(receiver->fd, NULL, NULL)) < 0) {
        dlt_vlog(LOG_ERR, "accept() on UNIX socket %d failed: %s\n", receiver->fd, strerror(errno));
        return -1;
    }

    /* check if file file descriptor was already used, and make it invalid if it
     * is reused. This prevents sending messages to wrong file descriptor */
    dlt_daemon_applications_invalidate_fd(daemon, daemon->ecuid, in_sock, verbose);
    dlt_daemon_contexts_invalidate_fd(daemon, daemon->ecuid, in_sock, verbose);

    if (dlt_connection_create(daemon_local,
                              &daemon_local->pEvent,
                              in_sock,
                              POLLIN,
                              DLT_CONNECTION_APP_MSG)) {
        dlt_log(LOG_ERR, "Failed to register new application. \n");
        close(in_sock);
        return -1;
    }

    if (verbose)
        dlt_vlog(LOG_INFO, "New connection to application established\n");

    return 0;
}
#endif

int dlt_daemon_process_control_messages(
    DltDaemon *daemon,
    DltDaemonLocal *daemon_local,
    DltReceiver *receiver,
    int verbose)
{
    int bytes_to_be_removed = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (receiver == NULL)) {
        dlt_log(LOG_ERR,
                "Invalid function parameters used for function "
                "dlt_daemon_process_control_messages()\n");
        return -1;
    }

    if (dlt_receiver_receive(receiver) <= 0) {
        dlt_daemon_close_socket(receiver->fd,
                                daemon,
                                daemon_local,
                                verbose);
        /* FIXME: Why the hell do we need to close the socket
         * on control message reception ??
         */
        return 0;
    }

    /* Process all received messages */
    while (dlt_message_read(
               &(daemon_local->msg),
               (uint8_t *)receiver->buf,
               (unsigned int) receiver->bytesRcvd,
               daemon_local->flags.nflag,
               daemon_local->flags.vflag) == DLT_MESSAGE_ERROR_OK) {
        /* Check for control message */
        if ((receiver->fd > 0) &&
            DLT_MSG_IS_CONTROL_REQUEST(&(daemon_local->msg)))
            dlt_daemon_client_process_control(receiver->fd,
                                              daemon, daemon_local,
                                              &(daemon_local->msg),
                                              daemon_local->flags.vflag);

        bytes_to_be_removed = (int) (daemon_local->msg.headersize +
            daemon_local->msg.datasize -
            sizeof(DltStorageHeader));

        if (daemon_local->msg.found_serialheader)
            bytes_to_be_removed += (int) sizeof(dltSerialHeader);

        if (daemon_local->msg.resync_offset)
            bytes_to_be_removed += daemon_local->msg.resync_offset;

        if (dlt_receiver_remove(receiver, bytes_to_be_removed) == -1) {
            dlt_log(LOG_WARNING,
                    "Can't remove bytes from receiver for sockets\n");
            return -1;
        }
    } /* while */

    if (dlt_receiver_move_to_begin(receiver) == -1) {
        dlt_log(LOG_WARNING, "Can't move bytes to beginning of receiver buffer for sockets\n");
        return -1;
    }

    return 0;
}

static int dlt_daemon_process_user_message_not_sup(DltDaemon *daemon,
                                                   DltDaemonLocal *daemon_local,
                                                   DltReceiver *receiver,
                                                   int verbose)
{
    DltUserHeader *userheader = (DltUserHeader *)(receiver->buf);
    (void)daemon;
    (void)daemon_local;

    PRINT_FUNCTION_VERBOSE(verbose);

    dlt_vlog(LOG_ERR, "Invalid user message type received: %u!\n",
             userheader->message);

    /* remove user header */
    if (dlt_receiver_remove(receiver, sizeof(DltUserHeader)) == -1)
        dlt_log(LOG_WARNING,
                "Can't remove bytes from receiver for user messages\n");

    return -1;
}

static dlt_daemon_process_user_message_func process_user_func[DLT_USER_MESSAGE_NOT_SUPPORTED] = {
    dlt_daemon_process_user_message_not_sup,
    dlt_daemon_process_user_message_log,
    dlt_daemon_process_user_message_register_application,
    dlt_daemon_process_user_message_unregister_application,
    dlt_daemon_process_user_message_register_context,
    dlt_daemon_process_user_message_unregister_context,
    dlt_daemon_process_user_message_not_sup,
    dlt_daemon_process_user_message_not_sup,
    dlt_daemon_process_user_message_overflow,
    dlt_daemon_process_user_message_set_app_ll_ts,
    dlt_daemon_process_user_message_not_sup,
    dlt_daemon_process_user_message_not_sup,
    dlt_daemon_process_user_message_not_sup,
    dlt_daemon_process_user_message_marker,
    dlt_daemon_process_user_message_not_sup,
    dlt_daemon_process_user_message_not_sup
};

int dlt_daemon_process_user_messages(DltDaemon *daemon,
                                     DltDaemonLocal *daemon_local,
                                     DltReceiver *receiver,
                                     int verbose)
{
    int offset = 0;
    int run_loop = 1;
    int32_t min_size = (int32_t) sizeof(DltUserHeader);
    DltUserHeader *userheader;
    int recv;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (receiver == NULL)) {
        dlt_log(LOG_ERR,
                "Invalid function parameters used for function "
                "dlt_daemon_process_user_messages()\n");
        return -1;
    }

    recv = dlt_receiver_receive(receiver);

    if (recv <= 0 && receiver->type == DLT_RECEIVE_SOCKET) {
        dlt_daemon_close_socket(receiver->fd,
                                daemon,
                                daemon_local,
                                verbose);
        return 0;
    }
    else if (recv < 0) {
        dlt_log(LOG_WARNING,
                "dlt_receiver_receive_fd() for user messages failed!\n");
        return -1;
    }

    /* look through buffer as long as data is in there */
    while ((receiver->bytesRcvd >= min_size) && run_loop) {
        dlt_daemon_process_user_message_func func = NULL;

        offset = 0;
        userheader = (DltUserHeader *)(receiver->buf + offset);

        while (!dlt_user_check_userheader(userheader) &&
               (offset + min_size <= receiver->bytesRcvd)) {
            /* resync if necessary */
            offset++;
            userheader = (DltUserHeader *)(receiver->buf + offset);
        }

        /* Check for user header pattern */
        if (!dlt_user_check_userheader(userheader))
            break;

        /* Set new start offset */
        if (offset > 0)
            dlt_receiver_remove(receiver, offset);

        if (userheader->message >= DLT_USER_MESSAGE_NOT_SUPPORTED)
            func = dlt_daemon_process_user_message_not_sup;
        else
            func = process_user_func[userheader->message];

        if (func(daemon,
                 daemon_local,
                 receiver,
                 daemon_local->flags.vflag) == -1)
            run_loop = 0;
    }

    /* keep not read data in buffer */
    if (dlt_receiver_move_to_begin(receiver) == -1) {
        dlt_log(LOG_WARNING,
                "Can't move bytes to beginning of receiver buffer for user "
                "messages\n");
        return -1;
    }

    return 0;
}

int dlt_daemon_process_user_message_overflow(DltDaemon *daemon,
                                             DltDaemonLocal *daemon_local,
                                             DltReceiver *rec,
                                             int verbose)
{
    uint32_t len = sizeof(DltUserControlMsgBufferOverflow);
    DltUserControlMsgBufferOverflow userpayload;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (rec == NULL)) {
        dlt_vlog(LOG_ERR, "Invalid function parameters used for %s\n",
                 __func__);
        return -1;
    }

    if (dlt_receiver_check_and_get(rec,
                                   &userpayload,
                                   len,
                                   DLT_RCV_SKIP_HEADER | DLT_RCV_REMOVE) < 0)
        /* Not enough bytes received */
        return -1;

    /* Store in daemon, that a message buffer overflow has occured */
    /* look if TCP connection to client is available or it least message can be put into buffer */
    if (dlt_daemon_control_message_buffer_overflow(DLT_DAEMON_SEND_TO_ALL,
                                                   daemon,
                                                   daemon_local,
                                                   userpayload.overflow_counter,
                                                   userpayload.apid,
                                                   verbose))
        /* there was an error when storing message */
        /* add the counter of lost messages to the daemon counter */
        daemon->overflow_counter += userpayload.overflow_counter;

    return 0;
}

int dlt_daemon_send_message_overflow(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    int ret;
    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == 0) || (daemon_local == 0)) {
        dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_user_message_overflow()\n");
        return DLT_DAEMON_ERROR_UNKNOWN;
    }

    /* Store in daemon, that a message buffer overflow has occured */
    if ((ret =
             dlt_daemon_control_message_buffer_overflow(DLT_DAEMON_SEND_TO_ALL, daemon, daemon_local,
                                                        daemon->overflow_counter,
                                                        "", verbose)))
        return ret;

    return DLT_DAEMON_ERROR_OK;
}

int dlt_daemon_process_user_message_register_application(DltDaemon *daemon,
                                                         DltDaemonLocal *daemon_local,
                                                         DltReceiver *rec,
                                                         int verbose)
{
    uint32_t len = sizeof(DltUserControlMsgRegisterApplication);
    uint32_t to_remove = 0;
    DltDaemonApplication *application = NULL;
    DltDaemonApplication *old_application = NULL;
    pid_t old_pid = 0;
    char description[DLT_DAEMON_DESCSIZE + 1] = { '\0' };
    DltUserControlMsgRegisterApplication userapp;
    char *origin;
    int fd = -1;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (rec == NULL)) {
        dlt_vlog(LOG_ERR, "Invalid function parameters used for %s\n",
                 __func__);
        return -1;
    }

    memset(&userapp, 0, sizeof(DltUserControlMsgRegisterApplication));
    origin = rec->buf;

    /* Adding temp variable to check the return value */
    int temp = 0;

    /* We shall not remove data before checking that everything is there. */
    temp = dlt_receiver_check_and_get(rec,
                                           &userapp,
                                           len,
                                           DLT_RCV_SKIP_HEADER);

    if (temp < 0)
        /* Not enough bytes received */
        return -1;
    else {
        to_remove = (uint32_t) temp;
    }

    len = userapp.description_length;

    if (len > DLT_DAEMON_DESCSIZE) {
        len = DLT_DAEMON_DESCSIZE;
        dlt_log(LOG_WARNING, "Application description exceeds limit\n");
    }

    /* adjust buffer pointer */
    rec->buf += to_remove + sizeof(DltUserHeader);

    if (dlt_receiver_check_and_get(rec, description, len, DLT_RCV_NONE) < 0) {
        dlt_log(LOG_ERR, "Unable to get application description\n");
        /* in case description was not readable, set dummy description */
        memcpy(description, "Unknown", sizeof("Unknown"));

        /* unknown len of original description, set to 0 to not remove in next
         * step. Because message buffer is re-adjusted the corrupted description
         * is ignored. */
        len = 0;
    }

    /* adjust to_remove */
    to_remove += (uint32_t) sizeof(DltUserHeader) + len;
    /* point to begin of message */
    rec->buf = origin;

    /* We can now remove data. */
    if (dlt_receiver_remove(rec, (int) to_remove) != DLT_RETURN_OK) {
        dlt_log(LOG_WARNING, "Can't remove bytes from receiver\n");
        return -1;
    }

    old_application = dlt_daemon_application_find(daemon, userapp.apid, daemon->ecuid, verbose);

    if (old_application != NULL)
        old_pid = old_application->pid;

    if (rec->type == DLT_RECEIVE_SOCKET)
        fd = rec->fd; /* For sockets, an app specific fd has already been created with accept(). */

    application = dlt_daemon_application_add(daemon,
                                             userapp.apid,
                                             userapp.pid,
                                             description,
                                             fd,
                                             daemon->ecuid,
                                             verbose);

    /* send log state to new application */
    dlt_daemon_user_send_log_state(daemon, application, verbose);

    if (application == NULL) {
        dlt_vlog(LOG_WARNING, "Can't add ApplicationID '%.4s' for PID %d\n",
                 userapp.apid, userapp.pid);
        return -1;
    }
    else if (old_pid != application->pid)
    {
        char local_str[DLT_DAEMON_TEXTBUFSIZE] = { '\0' };

        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "ApplicationID '%.4s' registered for PID %d, Description=%s",
                 application->apid,
                 application->pid,
                 application->application_description);
        dlt_daemon_log_internal(daemon,
                                daemon_local,
                                local_str,
                                daemon_local->flags.vflag);
        dlt_vlog(LOG_DEBUG, "%s%s", local_str, "\n");
    }

    return 0;
}

int dlt_daemon_process_user_message_register_context(DltDaemon *daemon,
                                                     DltDaemonLocal *daemon_local,
                                                     DltReceiver *rec,
                                                     int verbose)
{
    uint32_t to_remove = 0;
    uint32_t len = sizeof(DltUserControlMsgRegisterContext);
    DltUserControlMsgRegisterContext userctxt;
    char description[DLT_DAEMON_DESCSIZE + 1] = { '\0' };
    DltDaemonApplication *application = NULL;
    DltDaemonContext *context = NULL;
    DltServiceGetLogInfoRequest *req = NULL;
    char *origin;

    DltMessage msg;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (rec == NULL)) {
        dlt_vlog(LOG_ERR, "Invalid function parameters used for %s\n",
                 __func__);
        return -1;
    }

    memset(&userctxt, 0, sizeof(DltUserControlMsgRegisterContext));
    origin = rec->buf;

    /* Adding temp variable to check the return value */
    int temp = 0;

    temp = dlt_receiver_check_and_get(rec,
                                           &userctxt,
                                           len,
                                           DLT_RCV_SKIP_HEADER);

    if (temp < 0)
        /* Not enough bytes received */
        return -1;
    else {
        to_remove = (uint32_t) temp;
    }

    len = userctxt.description_length;

    if (len > DLT_DAEMON_DESCSIZE) {
        dlt_vlog(LOG_WARNING, "Context description exceeds limit: %u\n", len);
        len = DLT_DAEMON_DESCSIZE;
    }

    /* adjust buffer pointer */
    rec->buf += to_remove + sizeof(DltUserHeader);

    if (dlt_receiver_check_and_get(rec, description, len, DLT_RCV_NONE) < 0) {
        dlt_log(LOG_ERR, "Unable to get context description\n");
        /* in case description was not readable, set dummy description */
        memcpy(description, "Unknown", sizeof("Unknown"));

        /* unknown len of original description, set to 0 to not remove in next
         * step. Because message buffer is re-adjusted the corrupted description
         * is ignored. */
        len = 0;
    }

    /* adjust to_remove */
    to_remove += (uint32_t) sizeof(DltUserHeader) + len;
    /* point to begin of message */
    rec->buf = origin;

    /* We can now remove data. */
    if (dlt_receiver_remove(rec, (int) to_remove) != DLT_RETURN_OK) {
        dlt_log(LOG_WARNING, "Can't remove bytes from receiver\n");
        return -1;
    }

    application = dlt_daemon_application_find(daemon,
                                              userctxt.apid,
                                              daemon->ecuid,
                                              verbose);

    if (application == 0) {
        dlt_vlog(LOG_WARNING,
                 "ApID '%.4s' not found for new ContextID '%.4s' in %s\n",
                 userctxt.apid,
                 userctxt.ctid,
                 __func__);

        return 0;
    }

    /* Set log level */
    if (userctxt.log_level == DLT_USER_LOG_LEVEL_NOT_SET) {
        userctxt.log_level = DLT_LOG_DEFAULT;
    } else {
        /* Plausibility check */
        if ((userctxt.log_level < DLT_LOG_DEFAULT) ||
                (userctxt.log_level > DLT_LOG_VERBOSE)) {
            return -1;
        }
    }

    /* Set trace status */
    if (userctxt.trace_status == DLT_USER_TRACE_STATUS_NOT_SET) {
        userctxt.trace_status = DLT_TRACE_STATUS_DEFAULT;
    } else {
        /* Plausibility check */
        if ((userctxt.trace_status < DLT_TRACE_STATUS_DEFAULT) ||
                (userctxt.trace_status > DLT_TRACE_STATUS_ON)) {
            return -1;
        }
    }

    context = dlt_daemon_context_add(daemon,
                                     userctxt.apid,
                                     userctxt.ctid,
                                     userctxt.log_level,
                                     userctxt.trace_status,
                                     userctxt.log_level_pos,
                                     application->user_handle,
                                     description,
                                     daemon->ecuid,
                                     verbose);

    if (context == 0) {
        dlt_vlog(LOG_WARNING,
                 "Can't add ContextID '%.4s' for ApID '%.4s'\n in %s",
                 userctxt.ctid, userctxt.apid, __func__);
        return -1;
    }
    else {
        char local_str[DLT_DAEMON_TEXTBUFSIZE] = { '\0' };

        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "ContextID '%.4s' registered for ApID '%.4s', Description=%s",
                 context->ctid,
                 context->apid,
                 context->context_description);

        if (verbose)
            dlt_daemon_log_internal(daemon, daemon_local, local_str, verbose);

        dlt_vlog(LOG_DEBUG, "%s%s", local_str, "\n");
    }

    if (daemon_local->flags.offlineLogstorageMaxDevices)
        /* Store log level set for offline logstorage into context structure*/
        context->storage_log_level =
            (int8_t) dlt_daemon_logstorage_get_loglevel(daemon,
                                               (int8_t) daemon_local->flags.offlineLogstorageMaxDevices,
                                               userctxt.apid,
                                               userctxt.ctid);
    else
        context->storage_log_level = DLT_LOG_DEFAULT;

    /* Create automatic get log info response for registered context */
    if (daemon_local->flags.rflag) {
        /* Prepare request for get log info with one application and one context */
        if (dlt_message_init(&msg, verbose) == -1) {
            dlt_log(LOG_WARNING, "Can't initialize message");
            return -1;
        }

        msg.datasize = sizeof(DltServiceGetLogInfoRequest);

        if (msg.databuffer && (msg.databuffersize < msg.datasize)) {
            free(msg.databuffer);
            msg.databuffer = 0;
        }

        if (msg.databuffer == 0) {
            msg.databuffer = (uint8_t *)malloc(msg.datasize);
            msg.databuffersize = msg.datasize;
        }

        if (msg.databuffer == 0) {
            dlt_log(LOG_WARNING, "Can't allocate buffer for get log info message\n");
            return -1;
        }

        req = (DltServiceGetLogInfoRequest *)msg.databuffer;

        req->service_id = DLT_SERVICE_ID_GET_LOG_INFO;
        req->options = (uint8_t) daemon_local->flags.autoResponseGetLogInfoOption;
        dlt_set_id(req->apid, userctxt.apid);
        dlt_set_id(req->ctid, userctxt.ctid);
        dlt_set_id(req->com, "remo");

        dlt_daemon_control_get_log_info(DLT_DAEMON_SEND_TO_ALL, daemon, daemon_local, &msg, verbose);

        dlt_message_free(&msg, verbose);
    }

    if (context->user_handle >= DLT_FD_MINIMUM) {
        if ((userctxt.log_level == DLT_LOG_DEFAULT) || (userctxt.trace_status == DLT_TRACE_STATUS_DEFAULT)) {
            /* This call also replaces the default values with the values defined for default */
            if (dlt_daemon_user_send_log_level(daemon, context, verbose) == -1) {
                dlt_vlog(LOG_WARNING, "Can't send current log level as response to %s for (%.4s;%.4s)\n",
                         __func__,
                         context->apid,
                         context->ctid);
                return -1;
            }
        }
    }

    return 0;
}

int dlt_daemon_process_user_message_unregister_application(DltDaemon *daemon,
                                                           DltDaemonLocal *daemon_local,
                                                           DltReceiver *rec,
                                                           int verbose)
{
    uint32_t len = sizeof(DltUserControlMsgUnregisterApplication);
    DltUserControlMsgUnregisterApplication userapp;
    DltDaemonApplication *application = NULL;
    DltDaemonContext *context;
    int i, offset_base;
    DltDaemonRegisteredUsers *user_list = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (rec == NULL)) {
        dlt_vlog(LOG_ERR,
                 "Invalid function parameters used for %s\n",
                 __func__);
        return -1;
    }

    if (dlt_receiver_check_and_get(rec,
                                   &userapp,
                                   len,
                                   DLT_RCV_SKIP_HEADER | DLT_RCV_REMOVE) < 0)
        /* Not enough bytes received */
        return -1;

    user_list = dlt_daemon_find_users_list(daemon, daemon->ecuid, verbose);

    if (user_list == NULL)
        return -1;

    if (user_list->num_applications > 0) {
        /* Delete this application and all corresponding contexts
         * for this application from internal table.
         */
        application = dlt_daemon_application_find(daemon,
                                                  userapp.apid,
                                                  daemon->ecuid,
                                                  verbose);

        if (application) {
            /* Calculate start offset within contexts[] */
            offset_base = 0;

            for (i = 0; i < (application - (user_list->applications)); i++)
                offset_base += user_list->applications[i].num_contexts;

            for (i = (application->num_contexts) - 1; i >= 0; i--) {
                context = &(user_list->contexts[offset_base + i]);

                if (context) {
                    /* Delete context */
                    if (dlt_daemon_context_del(daemon,
                                               context,
                                               daemon->ecuid,
                                               verbose) == -1) {
                        dlt_vlog(LOG_WARNING,
                                 "Can't delete CtID '%.4s' for ApID '%.4s' in %s\n",
                                 context->ctid,
                                 context->apid,
                                 __func__);
                        return -1;
                    }
                }
            }

            /* Delete this application entry from internal table*/
            if (dlt_daemon_application_del(daemon,
                                           application,
                                           daemon->ecuid,
                                           verbose) == -1) {
                dlt_vlog(LOG_WARNING,
                         "Can't delete ApID '%.4s' in %s\n",
                         application->apid,
                         __func__);
                return -1;
            }
            else {
                char local_str[DLT_DAEMON_TEXTBUFSIZE] = { '\0' };

                snprintf(local_str,
                         DLT_DAEMON_TEXTBUFSIZE,
                         "Unregistered ApID '%.4s'",
                         userapp.apid);
                dlt_daemon_log_internal(daemon,
                                        daemon_local,
                                        local_str,
                                        verbose);
                dlt_vlog(LOG_DEBUG, "%s%s", local_str, "\n");
            }
        }
    }

    return 0;
}

int dlt_daemon_process_user_message_unregister_context(DltDaemon *daemon,
                                                       DltDaemonLocal *daemon_local,
                                                       DltReceiver *rec,
                                                       int verbose)
{
    uint32_t len = sizeof(DltUserControlMsgUnregisterContext);
    DltUserControlMsgUnregisterContext userctxt;
    DltDaemonContext *context;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (rec == NULL)) {
        dlt_vlog(LOG_ERR,
                 "Invalid function parameters used for %s\n",
                 __func__);

        return -1;
    }

    if (dlt_receiver_check_and_get(rec,
                                   &userctxt,
                                   len,
                                   DLT_RCV_SKIP_HEADER | DLT_RCV_REMOVE) < 0)
        /* Not enough bytes received */
        return -1;

    context = dlt_daemon_context_find(daemon,
                                      userctxt.apid,
                                      userctxt.ctid,
                                      daemon->ecuid,
                                      verbose);

    /* In case the daemon is loaded with predefined contexts and its context
     * unregisters, the context information will not be deleted from daemon's
     * table until its parent application is unregistered.
     */
    if (context && (context->predefined == false)) {
        /* Delete this connection entry from internal table*/
        if (dlt_daemon_context_del(daemon, context, daemon->ecuid, verbose) == -1) {
            dlt_vlog(LOG_WARNING,
                     "Can't delete CtID '%.4s' for ApID '%.4s' in %s\n",
                     userctxt.ctid,
                     userctxt.apid,
                     __func__);
            return -1;
        }
        else {
            char local_str[DLT_DAEMON_TEXTBUFSIZE] = { '\0' };

            snprintf(local_str,
                     DLT_DAEMON_TEXTBUFSIZE,
                     "Unregistered CtID '%.4s' for ApID '%.4s'",
                     userctxt.ctid,
                     userctxt.apid);

            if (verbose)
                dlt_daemon_log_internal(daemon,
                                        daemon_local,
                                        local_str,
                                        verbose);

            dlt_vlog(LOG_DEBUG, "%s%s", local_str, "\n");
        }
    }

    /* Create automatic unregister context response for unregistered context */
    if (daemon_local->flags.rflag)
        dlt_daemon_control_message_unregister_context(DLT_DAEMON_SEND_TO_ALL,
                                                      daemon,
                                                      daemon_local,
                                                      userctxt.apid,
                                                      userctxt.ctid,
                                                      "remo",
                                                      verbose);

    return 0;
}

int dlt_daemon_process_user_message_log(DltDaemon *daemon,
                                        DltDaemonLocal *daemon_local,
                                        DltReceiver *rec,
                                        int verbose)
{
    int ret = 0;
    int size = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (rec == NULL)) {
        dlt_vlog(LOG_ERR, "%s: invalid function parameters.\n", __func__);
        return DLT_DAEMON_ERROR_UNKNOWN;
    }

#ifdef DLT_SYSTEMD_WATCHDOG_ENFORCE_MSG_RX_ENABLE
    daemon->received_message_since_last_watchdog_interval = 1;
#endif
#ifdef DLT_SHM_ENABLE

    /** In case of SHM, the header still received via fifo/unix_socket receiver,
     * so we need to remove header from the receiver.
     */
    if (dlt_receiver_remove(rec, sizeof(DltUserHeader)) < 0)
        /* Not enough bytes received to remove*/
        return DLT_DAEMON_ERROR_UNKNOWN;

    while (1) {
        /* get log message from SHM then store into receiver buffer */
        size = dlt_shm_pull(&(daemon_local->dlt_shm),
                            daemon_local->recv_buf_shm,
                            DLT_SHM_RCV_BUFFER_SIZE);

        if (size <= 0)
            break;

        ret = dlt_message_read(&(daemon_local->msg),
                               daemon_local->recv_buf_shm, size, 0, verbose);

        if (DLT_MESSAGE_ERROR_OK != ret) {
            dlt_shm_remove(&(daemon_local->dlt_shm));
            dlt_log(LOG_WARNING, "failed to read messages from shm.\n");
            return DLT_DAEMON_ERROR_UNKNOWN;
        }

#ifdef DLT_LOG_LEVEL_APP_CONFIG
        DltDaemonApplication *app = dlt_daemon_application_find(
            daemon, daemon_local->msg.extendedheader->apid, daemon->ecuid, verbose);
#endif

        /* discard non-allowed levels if enforcement is on */
        bool keep_message = enforce_context_ll_and_ts_keep_message(
            daemon_local
#ifdef DLT_LOG_LEVEL_APP_CONFIG
            , app
#endif
        );

        if (keep_message)
          dlt_daemon_client_send_message_to_all_client(daemon, daemon_local, verbose);

        if (DLT_DAEMON_ERROR_OK != ret)
            dlt_log(LOG_ERR, "failed to send message to client.\n");
    }

#else
    ret = dlt_message_read(&(daemon_local->msg),
                           (unsigned char *)rec->buf + sizeof(DltUserHeader),
                           (unsigned int) ((unsigned int) rec->bytesRcvd - sizeof(DltUserHeader)),
                           0,
                           verbose);

    if (ret != DLT_MESSAGE_ERROR_OK) {
        if (ret != DLT_MESSAGE_ERROR_SIZE)
            /* This is a normal usecase: The daemon reads the data in 10kb chunks.
             * Thus the last trace in this chunk is probably not complete and will be completed
             * with the next chunk read. This happens always when the FIFO is filled with more than 10kb before
             * the daemon is able to read from the FIFO.
             * Thus the loglevel of this message is set to DEBUG.
             * A cleaner solution would be to check more in detail whether the message is not complete (normal usecase)
             * or the headers are corrupted (error case). */
            dlt_log(LOG_DEBUG, "Can't read messages from receiver\n");

        if (dlt_receiver_remove(rec, rec->bytesRcvd) != DLT_RETURN_OK) {
            /* In certain rare scenarios where only a partial message has been received
             * (Eg: kernel IPC buffer memory being full), we want to discard the message
             * and not broadcast it forward to connected clients. Since the DLT library
             * checks return value of the writev() call against the sent total message
             * length, the partial message will be buffered and retransmitted again.
             * This implicitly ensures that no message loss occurs.
             */
            dlt_log(LOG_WARNING, "failed to remove required bytes from receiver.\n");
        }

        return DLT_DAEMON_ERROR_UNKNOWN;
    }

#ifdef DLT_LOG_LEVEL_APP_CONFIG
    DltDaemonApplication *app = dlt_daemon_application_find(
        daemon, daemon_local->msg.extendedheader->apid, daemon->ecuid, verbose);
#endif

    /* discard non-allowed levels if enforcement is on */
    bool keep_message = enforce_context_ll_and_ts_keep_message(
        daemon_local
#ifdef DLT_LOG_LEVEL_APP_CONFIG
        , app
#endif
    );

    if (keep_message)
      dlt_daemon_client_send_message_to_all_client(daemon, daemon_local, verbose);

    /* keep not read data in buffer */
    size = (int) (daemon_local->msg.headersize +
        daemon_local->msg.datasize - sizeof(DltStorageHeader) +
        sizeof(DltUserHeader));

    if (daemon_local->msg.found_serialheader)
        size += (int) sizeof(dltSerialHeader);

    if (dlt_receiver_remove(rec, size) != DLT_RETURN_OK) {
        dlt_log(LOG_WARNING, "failed to remove bytes from receiver.\n");
        return DLT_DAEMON_ERROR_UNKNOWN;
    }

#endif

    return DLT_DAEMON_ERROR_OK;
}

bool enforce_context_ll_and_ts_keep_message(DltDaemonLocal *daemon_local
#ifdef DLT_LOG_LEVEL_APP_CONFIG
                                            , DltDaemonApplication *app
#endif
)
{
    if (!daemon_local->flags.enforceContextLLAndTS ||
        !daemon_local->msg.extendedheader) {
        return true;
    }

    const int mtin = DLT_GET_MSIN_MTIN(daemon_local->msg.extendedheader->msin);
#ifdef DLT_LOG_LEVEL_APP_CONFIG
    if (app->num_context_log_level_settings > 0) {
        DltDaemonContextLogSettings *log_settings =
            dlt_daemon_find_app_log_level_config(app, daemon_local->msg.extendedheader->ctid);

        if (log_settings != NULL) {
            return mtin <= log_settings->log_level;
        }
    }
#endif
    return mtin <= daemon_local->flags.contextLogLevel;
}

int dlt_daemon_process_user_message_set_app_ll_ts(DltDaemon *daemon,
                                                  DltDaemonLocal *daemon_local,
                                                  DltReceiver *rec,
                                                  int verbose)
{
    uint32_t len = sizeof(DltUserControlMsgAppLogLevelTraceStatus);
    DltUserControlMsgAppLogLevelTraceStatus userctxt;
    DltDaemonApplication *application;
    DltDaemonContext *context;
    int i, offset_base;
    int8_t old_log_level, old_trace_status;
    DltDaemonRegisteredUsers *user_list = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (rec == NULL)) {
        dlt_vlog(LOG_ERR,
                 "Invalid function parameters used for %s\n",
                 __func__);
        return DLT_RETURN_ERROR;
    }

    user_list = dlt_daemon_find_users_list(daemon, daemon->ecuid, verbose);

    if (user_list == NULL)
        return DLT_RETURN_ERROR;

    memset(&userctxt, 0, len);

    if (dlt_receiver_check_and_get(rec,
                                   &userctxt,
                                   len,
                                   DLT_RCV_SKIP_HEADER | DLT_RCV_REMOVE) < 0)
        /* Not enough bytes received */
        return DLT_RETURN_ERROR;

    if (user_list->num_applications > 0) {
        /* Get all contexts with application id matching the received application id */
        application = dlt_daemon_application_find(daemon,
                                                  userctxt.apid,
                                                  daemon->ecuid,
                                                  verbose);

        if (application) {
            /* Calculate start offset within contexts[] */
            offset_base = 0;

            for (i = 0; i < (application - (user_list->applications)); i++)
                offset_base += user_list->applications[i].num_contexts;

            for (i = 0; i < application->num_contexts; i++) {
                context = &(user_list->contexts[offset_base + i]);

                if (context) {
                    old_log_level = context->log_level;
                    context->log_level = (int8_t) userctxt.log_level; /* No endianess conversion necessary*/

                    old_trace_status = context->trace_status;
                    context->trace_status = (int8_t) userctxt.trace_status;   /* No endianess conversion necessary */

                    /* The following function sends also the trace status */
                    if ((context->user_handle >= DLT_FD_MINIMUM) &&
                        (dlt_daemon_user_send_log_level(daemon,
                                                        context,
                                                        verbose) != 0)) {
                        context->log_level = old_log_level;
                        context->trace_status = old_trace_status;
                    }
                }
            }
        }
    }

    return DLT_RETURN_OK;
}

int dlt_daemon_process_user_message_log_mode(DltDaemon *daemon,
                                             DltDaemonLocal *daemon_local,
                                             DltReceiver *rec,
                                             int verbose)
{
    DltUserControlMsgLogMode userctxt;
    uint32_t len = sizeof(DltUserControlMsgLogMode);

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == 0) || (daemon_local == 0)) {
        dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_log_mode()\n");
        return -1;
    }

    memset(&userctxt, 0, len);

    if (dlt_receiver_check_and_get(rec,
                                   &userctxt,
                                   len,
                                   DLT_RCV_SKIP_HEADER | DLT_RCV_REMOVE) < 0)
        /* Not enough bytes received */
        return -1;

    /* set the new log mode */
    daemon->mode = userctxt.log_mode;

    /* write configuration persistantly */
    dlt_daemon_configuration_save(daemon, daemon->runtime_configuration, verbose);

    return 0;
}

int dlt_daemon_process_user_message_marker(DltDaemon *daemon,
                                           DltDaemonLocal *daemon_local,
                                           DltReceiver *rec,
                                           int verbose)
{
    uint32_t len = sizeof(DltUserControlMsgLogMode);
    DltUserControlMsgLogMode userctxt;
    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (rec == NULL)) {
        dlt_vlog(LOG_ERR, "Invalid function parameters used for %s\n",
                 __func__);
        return -1;
    }

    memset(&userctxt, 0, len);

    if (dlt_receiver_check_and_get(rec,
                                   &userctxt,
                                   len,
                                   DLT_RCV_SKIP_HEADER | DLT_RCV_REMOVE) < 0)
        /* Not enough bytes received */
        return -1;

    /* Create automatic unregister context response for unregistered context */
    dlt_daemon_control_message_marker(DLT_DAEMON_SEND_TO_ALL, daemon, daemon_local, verbose);

    return 0;
}

int dlt_daemon_send_ringbuffer_to_client(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    int ret;
    static uint8_t data[DLT_DAEMON_RCVBUFSIZE];
    int length;
#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
    uint32_t curr_time;
#endif

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == 0) || (daemon_local == 0)) {
        dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_send_ringbuffer_to_client()\n");
        return DLT_DAEMON_ERROR_UNKNOWN;
    }

    if (dlt_buffer_get_message_count(&(daemon->client_ringbuffer)) <= 0) {
        dlt_daemon_change_state(daemon, DLT_DAEMON_STATE_SEND_DIRECT);
        return DLT_DAEMON_ERROR_OK;
    }

#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE

    if (sd_notify(0, "WATCHDOG=1") < 0)
        dlt_vlog(LOG_WARNING, "Could not reset systemd watchdog: %s\n", strerror(errno));

    curr_time = dlt_uptime();
#endif

    while ((length = dlt_buffer_copy(&(daemon->client_ringbuffer), data, sizeof(data))) > 0) {
#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE

        if ((dlt_uptime() - curr_time) / 10000 >= watchdog_trigger_interval) {
            if (sd_notify(0, "WATCHDOG=1") < 0)
                dlt_log(LOG_WARNING, "Could not reset systemd watchdog\n");

            curr_time = dlt_uptime();
        }

#endif

        if ((ret =
                 dlt_daemon_client_send(DLT_DAEMON_SEND_FORCE, daemon, daemon_local, 0, 0, data, length, 0, 0,
                                        verbose)))
            return ret;

        dlt_buffer_remove(&(daemon->client_ringbuffer));

        if (daemon->state != DLT_DAEMON_STATE_SEND_BUFFER)
            dlt_daemon_change_state(daemon, DLT_DAEMON_STATE_SEND_BUFFER);

        if (dlt_buffer_get_message_count(&(daemon->client_ringbuffer)) <= 0) {
            dlt_daemon_change_state(daemon, DLT_DAEMON_STATE_SEND_DIRECT);
            return DLT_DAEMON_ERROR_OK;
        }
    }

    return DLT_DAEMON_ERROR_OK;
}

#ifdef __QNX__
static void *timer_thread(void *data)
{
    int pexit = 0;
    unsigned int sleep_ret = 0;

    DltDaemonPeriodicData* timer_thread_data = (DltDaemonPeriodicData*) data;

    /* Timer will start in starts_in sec*/
    if ((sleep_ret = sleep(timer_thread_data->starts_in))) {
        dlt_vlog(LOG_NOTICE, "Sleep remains [%u] for starting!"
                "Stop thread of timer [%d]\n",
                sleep_ret, timer_thread_data->timer_id);
         close_pipes(dlt_timer_pipes[timer_thread_data->timer_id]);
         return NULL;
    }

    while (1) {
        if ((dlt_timer_pipes[timer_thread_data->timer_id][1] > 0) &&
                (0 > write(dlt_timer_pipes[timer_thread_data->timer_id][1], "1", 1))) {
            dlt_vlog(LOG_ERR, "Failed to send notification for timer [%s]!\n",
                    dlt_timer_names[timer_thread_data->timer_id]);
            pexit = 1;
        }

        if (pexit || g_exit) {
            dlt_vlog(LOG_NOTICE, "Received signal!"
                    "Stop thread of timer [%d]\n",
                    timer_thread_data->timer_id);
            close_pipes(dlt_timer_pipes[timer_thread_data->timer_id]);
            return NULL;
        }

        if ((sleep_ret = sleep(timer_thread_data->period_sec))) {
            dlt_vlog(LOG_NOTICE, "Sleep remains [%u] for interval!"
                    "Stop thread of timer [%d]\n",
                    sleep_ret, timer_thread_data->timer_id);
             close_pipes(dlt_timer_pipes[timer_thread_data->timer_id]);
             return NULL;
        }
    }
}
#endif

int create_timer_fd(DltDaemonLocal *daemon_local,
                    int period_sec,
                    int starts_in,
                    DltTimers timer_id)
{
    int local_fd = DLT_FD_INIT;
    char *timer_name = NULL;

    if (timer_id >= DLT_TIMER_UNKNOWN) {
        dlt_log(DLT_LOG_ERROR, "Unknown timer.");
        return -1;
    }

    timer_name = dlt_timer_names[timer_id];

    if (daemon_local == NULL) {
        dlt_log(DLT_LOG_ERROR, "Daemon local structure is NULL");
        return -1;
    }

    if ((period_sec <= 0) || (starts_in <= 0)) {
        /* timer not activated via the service file */
        dlt_vlog(LOG_INFO, "<%s> not set: period=0\n", timer_name);
        local_fd = DLT_FD_INIT;
    }
    else {
#ifdef linux
        struct itimerspec l_timer_spec;
        local_fd = timerfd_create(CLOCK_MONOTONIC, 0);

        if (local_fd < 0)
            dlt_vlog(LOG_WARNING, "<%s> timerfd_create failed: %s\n",
                     timer_name, strerror(errno));

        l_timer_spec.it_interval.tv_sec = period_sec;
        l_timer_spec.it_interval.tv_nsec = 0;
        l_timer_spec.it_value.tv_sec = starts_in;
        l_timer_spec.it_value.tv_nsec = 0;

        if (timerfd_settime(local_fd, 0, &l_timer_spec, NULL) < 0) {
            dlt_vlog(LOG_WARNING, "<%s> timerfd_settime failed: %s\n",
                     timer_name, strerror(errno));
            local_fd = DLT_FD_INIT;
        }
#elif __QNX__
        /*
         * Since timerfd is not valid in QNX, new threads are introduced
         * to manage timers and communicate with main thread when timer expires.
         */
        if(0 != pipe(dlt_timer_pipes[timer_id])) {
            dlt_vlog(LOG_ERR, "Failed to create pipe for timer [%s]",
                    dlt_timer_names[timer_id]);
            return -1;
        }
        if (NULL == timer_data[timer_id]) {
            timer_data[timer_id] = calloc(1, sizeof(DltDaemonPeriodicData));
            if (NULL == timer_data[timer_id]) {
                dlt_vlog(LOG_ERR, "Failed to allocate memory for timer_data [%s]!\n",
                         dlt_timer_names[timer_id]);
                close_pipes(dlt_timer_pipes[timer_id]);
                return -1;
            }
        }

        timer_data[timer_id]->timer_id = timer_id;
        timer_data[timer_id]->period_sec = period_sec;
        timer_data[timer_id]->starts_in = starts_in;
        timer_data[timer_id]->wakeups_missed = 0;

        if (0 != pthread_create(&timer_threads[timer_id], NULL,
                            &timer_thread, (void*)timer_data[timer_id])) {
            dlt_vlog(LOG_ERR, "Failed to create new thread for timer [%s]!\n",
                                     dlt_timer_names[timer_id]);
            /* Clean up timer before returning */
            close_pipes(dlt_timer_pipes[timer_id]);
            free(timer_data[timer_id]);
            timer_data[timer_id] = NULL;

            return -1;
        }
        local_fd = dlt_timer_pipes[timer_id][0];
#endif
    }

    return dlt_connection_create(daemon_local,
                                 &daemon_local->pEvent,
                                 local_fd,
                                 POLLIN,
                                 dlt_timer_conn_types[timer_id]);
}

/* Close connection function */
int dlt_daemon_close_socket(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    char local_str[DLT_DAEMON_TEXTBUFSIZE] = { '\0' };

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon_local == NULL) || (daemon == NULL)) {
        dlt_log(LOG_ERR, "dlt_daemon_close_socket: Invalid input parmeters\n");
        return -1;
    }

    /* Closure is done while unregistering has for any connection */
    dlt_event_handler_unregister_connection(&daemon_local->pEvent,
                                            daemon_local,
                                            sock);

    if (daemon_local->client_connections == 0) {
        /* send new log state to all applications */
        daemon->connectionState = 0;
        dlt_daemon_user_send_all_log_state(daemon, verbose);

        /* For offline tracing we still can use the same states */
        /* as for socket sending. Using this trick we see the traces */
        /* In the offline trace AND in the socket stream. */
        if (daemon_local->flags.yvalue[0] == 0)
            dlt_daemon_change_state(daemon, DLT_DAEMON_STATE_BUFFER);
    }

    dlt_daemon_control_message_connection_info(DLT_DAEMON_SEND_TO_ALL,
                                               daemon,
                                               daemon_local,
                                               DLT_CONNECTION_STATUS_DISCONNECTED,
                                               "",
                                               verbose);

    snprintf(local_str, DLT_DAEMON_TEXTBUFSIZE,
             "Client connection #%d closed. Total Clients : %d",
             sock,
             daemon_local->client_connections);
    dlt_daemon_log_internal(daemon, daemon_local, local_str, daemon_local->flags.vflag);
    dlt_vlog(LOG_DEBUG, "%s%s", local_str, "\n");

    return 0;
}

/**
 \}
 */
