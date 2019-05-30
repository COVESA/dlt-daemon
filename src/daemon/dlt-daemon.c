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
#include <unistd.h>     /* for close() */
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <pthread.h>

#ifdef linux
#   include <sys/timerfd.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#if defined(linux) && defined(__NR_statx)
#   include <linux/stat.h>
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

#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE) || defined(DLT_SYSTEMD_ENABLE)
#   include "sd-daemon.h"
#endif

/**
 * \defgroup daemon DLT Daemon
 * \addtogroup daemon
 \{
 */

/** Global text output buffer, mainly used for creation of error/warning strings */
static char str[DLT_DAEMON_TEXTBUFSIZE];

static int dlt_daemon_log_internal(DltDaemon *daemon, DltDaemonLocal *daemon_local, char *str, int verbose);

#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
static uint32_t watchdog_trigger_interval;  /* watchdog trigger interval in [s] */
#endif

/* used in main event loop and signal handler */
int g_exit = 0;

int g_signo = 0;

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
    printf("  -t directory  Directory for local fifo and user-pipes (Default: /tmp)\n");
    printf("                (Applications wanting to connect to a daemon using a\n");
    printf("                custom directory need to be started with the environment \n");
    printf("                variable DLT_PIPE_DIR set appropriately)\n");
    printf("  -p port       port to monitor for incoming requests (Default: 3490)\n");
    printf("                (Applications wanting to connect to a daemon using a custom\n");
    printf("                port need to be started with the environment variable\n");
    printf("                DLT_DAEMON_TCP_PORT set appropriately)\n");
} /* usage() */

/**
 * Option handling
 */
int option_handling(DltDaemonLocal *daemon_local, int argc, char *argv[])
{
    int c;

    if (daemon_local == 0) {
        fprintf (stderr, "Invalid parameter passed to option_handling()\n");
        return -1;
    }

    /* Initialize flags */
    memset(daemon_local, 0, sizeof(DltDaemonLocal));

    /* default values */
    daemon_local->flags.port = DLT_DAEMON_TCP_PORT;
    strncpy(dltFifoBaseDir, DLT_USER_IPC_PATH, sizeof(DLT_USER_IPC_PATH));

    opterr = 0;

    while ((c = getopt (argc, argv, "hdc:t:p:")) != -1)
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
        case 't':
        {
            strncpy(dltFifoBaseDir, optarg, NAME_MAX);
            break;
        }
        case 'p':
        {
            daemon_local->flags.port = atoi(optarg);

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
            if ((optopt == 'c') || (optopt == 't') || (optopt == 'p'))
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);

            /* unknown or wrong option used, show usage information and terminate */
            usage();
            return -1;
        }
        default:
        {
            fprintf (stderr, "Invalid option, this should never occur!\n");
            return -1;
        }
        } /* switch() */


#ifndef DLT_USE_UNIX_SOCKET_IPC
    snprintf(daemon_local->flags.userPipesDir, NAME_MAX + 1, "%s/dltpipes", dltFifoBaseDir);
#endif
    snprintf(daemon_local->flags.daemonFifoName, NAME_MAX + 1, "%s/dlt", dltFifoBaseDir);

    return 0;

}  /* option_handling() */

/**
 * Option file parser
 */
int option_file_parser(DltDaemonLocal *daemon_local)
{
    FILE *pFile;
    int value_length = 1024;
    char line[value_length - 1];
    char token[value_length];
    char value[value_length];
    char *pch;
    const char *filename;

    /* set default values for configuration */
    daemon_local->flags.sharedMemorySize = DLT_SHM_SIZE;
    daemon_local->flags.sendMessageTime = 0;
    daemon_local->flags.offlineTraceDirectory[0] = 0;
    daemon_local->flags.offlineTraceFileSize = 1000000;
    daemon_local->flags.offlineTraceMaxSize = 0;
    daemon_local->flags.offlineTraceFilenameTimestampBased = 1;
    daemon_local->flags.loggingMode = DLT_LOG_TO_CONSOLE;
    daemon_local->flags.loggingLevel = LOG_INFO;
    snprintf(daemon_local->flags.loggingFilename,
             sizeof(daemon_local->flags.loggingFilename) - 1,
             "%s/dlt.log",
             dltFifoBaseDir);
    daemon_local->flags.loggingFilename[sizeof(daemon_local->flags.loggingFilename) - 1] = 0;
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
    daemon_local->flags.offlineLogstorageMaxDevices = 0;
    daemon_local->flags.offlineLogstorageTimestamp = 1;
    daemon_local->flags.offlineLogstorageDelimiter = '_';
    daemon_local->flags.offlineLogstorageMaxCounter = UINT_MAX;
    daemon_local->flags.offlineLogstorageMaxCounterIdx = 0;
    daemon_local->flags.offlineLogstorageCacheSize = 30000; /* 30MB */
    dlt_daemon_logstorage_set_logstorage_cache_size(
        daemon_local->flags.offlineLogstorageCacheSize);
    strncpy(daemon_local->flags.ctrlSockPath,
            DLT_DAEMON_DEFAULT_CTRL_SOCK_PATH,
            sizeof(daemon_local->flags.ctrlSockPath) - 1);
#ifdef DLT_USE_UNIX_SOCKET_IPC
    snprintf(daemon_local->flags.appSockPath, DLT_IPC_PATH_MAX, "%s/dlt", DLT_USER_IPC_PATH);

    if (strlen(DLT_USER_IPC_PATH) > DLT_IPC_PATH_MAX)
        fprintf(stderr, "Provided path too long...trimming it to path[%s]\n",
                daemon_local->flags.appSockPath);

#endif
    daemon_local->flags.gatewayMode = 0;
    strncpy(daemon_local->flags.gatewayConfigFile,
            DLT_GATEWAY_CONFIG_PATH,
            DLT_DAEMON_FLAG_MAX - 1);
    daemon_local->flags.autoResponseGetLogInfoOption = 7;
    daemon_local->flags.contextLogLevel = DLT_LOG_INFO;
    daemon_local->flags.contextTraceStatus = DLT_TRACE_STATUS_OFF;
    daemon_local->flags.enforceContextLLAndTS = 0; /* default is off */

    daemon_local->flags.ipNodes = malloc(sizeof(DltBindAddress_t));
    if (daemon_local->flags.ipNodes == NULL) {
        dlt_log(LOG_ERR, "Could not allocate default IP node\n");
        return -1;
    }
    else {
        strncpy(daemon_local->flags.ipNodes->ip, "0.0.0.0", sizeof(daemon_local->flags.ipNodes->ip) - 1);
        daemon_local->flags.ipNodes->next = NULL;
    }

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
                        daemon_local->flags.loggingMode = atoi(value);
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
                    else if (strcmp(token, "TimeOutOnSend") == 0)
                    {
                        daemon_local->timeoutOnSend = atoi(value);
                        /*printf("Option: %s=%s\n",token,value); */
                    }
                    else if (strcmp(token, "RingbufferMinSize") == 0)
                    {
                        sscanf(value, "%lu", &(daemon_local->RingbufferMinSize));
                    }
                    else if (strcmp(token, "RingbufferMaxSize") == 0)
                    {
                        sscanf(value, "%lu", &(daemon_local->RingbufferMaxSize));
                    }
                    else if (strcmp(token, "RingbufferStepSize") == 0)
                    {
                        sscanf(value, "%lu", &(daemon_local->RingbufferStepSize));
                    }
                    else if (strcmp(token, "DaemonFIFOSize") == 0)
                    {
                        sscanf(value, "%lu", &(daemon_local->daemonFifoSize));
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
                        daemon_local->flags.offlineTraceFilenameTimestampBased = atoi(value);
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
                        daemon_local->flags.offlineLogstorageMaxDevices = atoi(value);
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
                        daemon_local->flags.offlineLogstorageMaxCounter = atoi(value);
                        daemon_local->flags.offlineLogstorageMaxCounterIdx = strlen(value);
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
                    else if (strcmp(token, "BindAddress") == 0)
                    {
                        DltBindAddress_t *head = daemon_local->flags.ipNodes;
                        DltBindAddress_t *newNode = NULL;
                        DltBindAddress_t *temp = NULL;

                        /* update first IP node with address from configuration file */
                        char *tok = strtok(value, ",;");
                        if (tok != NULL) {
                            // set first IP address
                            strncpy(head->ip, tok, sizeof(head->ip) - 1);
                            temp = head; // head->next is by default NULL, no need to set it
                        }
                        tok = strtok(NULL, ",;");
                        while (tok != NULL ) {
                            newNode = malloc(sizeof(DltBindAddress_t));
                            if (newNode == NULL) {
                                dlt_log(LOG_ERR, "Could not allocate for IP list\n");
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

#ifndef DLT_USE_UNIX_SOCKET_IPC
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
    dlt_log_init(daemon_local.flags.loggingMode);

    /* Print version information */
    dlt_get_version(version, DLT_DAEMON_TEXTBUFSIZE);

    snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "Starting DLT Daemon; %s\n", version);
    dlt_log(LOG_NOTICE, str);

    PRINT_FUNCTION_VERBOSE(daemon_local.flags.vflag);

#ifndef DLT_USE_UNIX_SOCKET_IPC

    /* Make sure the parent user directory is created */
    if (dlt_mkdir_recursive(dltFifoBaseDir) != 0) {
        snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "Base dir %s cannot be created!\n", dltFifoBaseDir);
        dlt_log(LOG_ERR, str);
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

    /* --- Daemon init phase 2 begin --- */
    if (dlt_daemon_local_init_p2(&daemon, &daemon_local, daemon_local.flags.vflag) == -1) {
        dlt_log(LOG_CRIT, "Initialization of phase 2 failed!\n");
        return -1;
    }

    /* --- Daemon init phase 2 end --- */

    if (daemon_local.flags.offlineLogstorageDirPath[0]) {
        if (dlt_daemon_logstorage_setup_internal_storage(
                &daemon,
                &daemon_local,
                daemon_local.flags.offlineLogstorageDirPath,
                daemon_local.flags.vflag) == -1)
            dlt_log(LOG_INFO,
                    "Setting up internal offline log storage failed!\n");
    }

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
            dlt_log(LOG_CRIT, "Fail to create gateway\n");
            return -1;
        }

        /* create gateway timer */
        create_timer_fd(&daemon_local,
                        DLT_GATEWAY_TIMER_INTERVAL,
                        DLT_GATEWAY_TIMER_INTERVAL,
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

    if (dlt_daemon_load_runtime_configuration(&daemon, daemon_local.flags.ivalue, daemon_local.flags.vflag) == -1) {
        dlt_log(LOG_ERR, "Could not load runtime config\n");
        return -1;
    }

    dlt_daemon_log_internal(&daemon,
                            &daemon_local,
                            "Daemon launched. Starting to output traces...",
                            daemon_local.flags.vflag);

    /* Even handling loop. */
    while ((back >= 0) && (g_exit >= 0))
        back = dlt_daemon_handle_event(&daemon_local.pEvent,
                                       &daemon,
                                       &daemon_local);

    snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "Exiting DLT daemon... [%d]", g_signo);
    dlt_daemon_log_internal(&daemon, &daemon_local, str, daemon_local.flags.vflag);
    dlt_log(LOG_NOTICE, str);

    dlt_daemon_local_cleanup(&daemon, &daemon_local, daemon_local.flags.vflag);

    dlt_gateway_deinit(&daemon_local.pGateway, daemon_local.flags.vflag);

    dlt_daemon_free(&daemon, daemon_local.flags.vflag);

    dlt_log(LOG_NOTICE, "Leaving DLT daemon\n");

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

#ifndef DLT_USE_UNIX_SOCKET_IPC

    if (dlt_daemon_create_pipes_dir(daemon_local->flags.userPipesDir) == DLT_RETURN_ERROR)
        return DLT_RETURN_ERROR;

#endif

    /* Check for daemon mode */
    if (daemon_local->flags.dflag)
        dlt_daemon_daemonize(daemon_local->flags.vflag);

    /* Re-Initialize internal logging facility after fork */
    dlt_log_set_filename(daemon_local->flags.loggingFilename);
    dlt_log_set_level(daemon_local->flags.loggingLevel);
    dlt_log_init(daemon_local->flags.loggingMode);

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
        if (dlt_offline_trace_init(&(daemon_local->offlineTrace),
                                   daemon_local->flags.offlineTraceDirectory,
                                   daemon_local->flags.offlineTraceFileSize,
                                   daemon_local->flags.offlineTraceMaxSize,
                                   daemon_local->flags.offlineTraceFilenameTimestampBased) == -1) {
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
    if (dlt_shm_init_server(&(daemon_local->dlt_shm), DLT_SHM_KEY, daemon_local->flags.sharedMemorySize) == -1) {
        dlt_log(LOG_ERR, "Could not initialize shared memory\n");
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

    /* Binary semaphore for thread */
    if (sem_init(&dlt_daemon_mutex, 0, 1) == -1) {
        dlt_log(LOG_ERR, "Could not initialize binary semaphore\n");
        return -1;
    }

    /* Get ECU version info from a file. If it fails, use dlt_version as fallback. */
    if (dlt_daemon_local_ecu_version_init(daemon, daemon_local, daemon_local->flags.vflag) < 0) {
        daemon->ECUVersionString = malloc(DLT_DAEMON_TEXTBUFSIZE);

        if (daemon->ECUVersionString == 0) {
            dlt_log(LOG_WARNING, "Could not allocate memory for version string\n");
            return -1;
        }

        dlt_get_version(daemon->ECUVersionString, DLT_DAEMON_TEXTBUFSIZE);
    }

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
        snprintf(str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "Failed to open serial device %s\n",
                 daemon_local->flags.yvalue);
        dlt_log(LOG_ERR, str);

        daemon_local->flags.yvalue[0] = 0;
        return -1;
    }

    if (isatty(fd)) {
        int speed = DLT_DAEMON_SERIAL_DEFAULT_BAUDRATE;

        if (daemon_local->flags.bvalue[0])
            speed = atoi(daemon_local->flags.bvalue);

        daemon_local->baudrate = dlt_convert_serial_speed(speed);

        if (dlt_setup_serial(fd, daemon_local->baudrate) < 0) {
            close(fd);
            daemon_local->flags.yvalue[0] = 0;

            snprintf(str,
                     DLT_DAEMON_TEXTBUFSIZE,
                     "Failed to configure serial device %s (%s) \n",
                     daemon_local->flags.yvalue,
                     strerror(errno));
            dlt_log(LOG_ERR, str);

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

#ifndef DLT_USE_UNIX_SOCKET_IPC
static int dlt_daemon_init_fifo(DltDaemonLocal *daemon_local)
{
    int ret;
    int fd = -1;
    int fifo_size;
    char local_str[DLT_DAEMON_TEXTBUFSIZE];

    /* open named pipe(FIFO) to receive DLT messages from users */
    umask(0);

    /* Try to delete existing pipe, ignore result of unlink */
    const char *tmpFifo = daemon_local->flags.daemonFifoName;
    unlink(tmpFifo);

    ret = mkfifo(tmpFifo, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

    if (ret == -1) {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "FIFO user %s cannot be created (%s)!\n",
                 tmpFifo,
                 strerror(errno));
        dlt_log(LOG_WARNING, local_str);
        return -1;
    } /* if */

    fd = open(tmpFifo, O_RDWR);

    if (fd == -1) {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "FIFO user %s cannot be opened (%s)!\n",
                 tmpFifo,
                 strerror(errno));
        dlt_log(LOG_WARNING, local_str);
        return -1;
    } /* if */

    if (daemon_local->daemonFifoSize != 0) {
        /* Set Daemon FIFO size */
        if (fcntl(fd, F_SETPIPE_SZ, daemon_local->daemonFifoSize) == -1) {
            snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "set FIFO size error: %s\n", strerror(errno));
            dlt_log(LOG_ERR, str);
        }
    }

    /* Get Daemon FIFO size */
    if ((fifo_size = fcntl(fd, F_GETPIPE_SZ, 0)) == -1) {
        snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "get FIFO size error: %s\n", strerror(errno));
        dlt_log(LOG_ERR, str);
    }
    else {
        snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "FIFO size: %d\n", fifo_size);
        dlt_log(LOG_INFO, str);
    }

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

int dlt_daemon_local_connection_init(DltDaemon *daemon,
                                     DltDaemonLocal *daemon_local,
                                     int verbose)
{
    char local_str[DLT_DAEMON_TEXTBUFSIZE];
    int fd = -1;
    int mask = 0;
    DltBindAddress_t* head = daemon_local->flags.ipNodes;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL)) {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "%s: Invalid function parameters\n",
                 __func__);

        dlt_log(LOG_ERR, local_str);
        return -1;
    }

#ifdef DLT_USE_UNIX_SOCKET_IPC
    /* create and open socket to receive incoming connections from user application
     * socket access permission set to srw-rw-rw- (666) */
    mask = S_IXUSR | S_IXGRP | S_IXOTH;

    if (dlt_daemon_unix_socket_open(&fd,
                                    daemon_local->flags.appSockPath,
                                    SOCK_STREAM,
                                    mask) == DLT_RETURN_OK) {
        if (dlt_connection_create(daemon_local,
                                  &daemon_local->pEvent,
                                  fd,
                                  POLLIN,
                                  DLT_CONNECTION_APP_CONNECT)) {
            dlt_log(LOG_CRIT, "Could not initialize app socket.\n");
            return DLT_RETURN_ERROR;
        }
    }
    else {
        dlt_log(LOG_CRIT, "Could not initialize app socket.\n");
        return DLT_RETURN_ERROR;
    }
#else

    if (dlt_daemon_init_fifo(daemon_local)) {
        dlt_log(LOG_ERR, "Unable to initialize fifo.\n");
        return DLT_RETURN_ERROR;
    }

#endif

    /* create and open socket to receive incoming connections from client */
    daemon_local->client_connections = 0;

    while (head != NULL) {
        fd = -1;
        if (dlt_daemon_socket_open(&fd, daemon_local->flags.port, head->ip) == DLT_RETURN_OK) {
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
        head = head->next;
    }

    /* create and open unix socket to receive incoming connections from
     * control application
     * socket access permission set to srw-rw---- (660)  */
    mask = S_IXUSR | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH;

    fd = -1;
    if (dlt_daemon_unix_socket_open(&fd,
                                    daemon_local->flags.ctrlSockPath,
                                    SOCK_STREAM,
                                    mask) == DLT_RETURN_OK) {
        if (dlt_connection_create(daemon_local,
                                  &daemon_local->pEvent,
                                  fd,
                                  POLLIN,
                                  DLT_CONNECTION_CONTROL_CONNECT)) {
            dlt_log(LOG_ERR, "Could not initialize control socket.\n");
            return DLT_RETURN_ERROR;
        }
    }
    else {
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
    version = malloc(size + 1);

    if (version == 0) {
        dlt_log(LOG_WARNING, "Cannot allocate memory for ECU version.\n");
        fclose(f);
        return -1;
    }

    off_t offset = 0;

    while (!feof(f)) {
        offset += fread(version + offset, 1, size, f);

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
        dlt_offline_trace_free(&(daemon_local->offlineTrace));

    /* Ignore result */
    dlt_file_free(&(daemon_local->file), daemon_local->flags.vflag);

#ifndef DLT_USE_UNIX_SOCKET_IPC
    /* Try to delete existing pipe, ignore result of unlink() */
    unlink(daemon_local->flags.daemonFifoName);
#else
    /* Try to delete existing pipe, ignore result of unlink() */
    unlink(daemon_local->flags.appSockPath);
#endif

#ifdef DLT_SHM_ENABLE
    /* free shared memory */
    dlt_shm_free_server(&(daemon_local->dlt_shm));
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
    char tmp[PATH_MAX + 1] = { 0 };

    snprintf(tmp, PATH_MAX, "%s/dlt", dltFifoBaseDir);
    (void)unlink(tmp);

    /* stop event loop */
    g_exit = -1;
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
    int ret;

    PRINT_FUNCTION_VERBOSE(verbose);

    /* Set storageheader */
    msg.storageheader = (DltStorageHeader *)(msg.headerbuffer);
    dlt_set_storageheader(msg.storageheader, daemon->ecuid);

    /* Set standardheader */
    msg.standardheader = (DltStandardHeader *)(msg.headerbuffer + sizeof(DltStorageHeader));
    msg.standardheader->htyp = DLT_HTYP_UEH | DLT_HTYP_WEID | DLT_HTYP_WSID | DLT_HTYP_WTMS |
        DLT_HTYP_PROTOCOL_VERSION1;
    msg.standardheader->mcnt = uiMsgCount++;

    uiExtraSize = DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp) +
        (DLT_IS_HTYP_UEH(msg.standardheader->htyp) ? sizeof(DltExtendedHeader) : 0);
    msg.headersize = sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + uiExtraSize;

    /* Set extraheader */
    pStandardExtra =
        (DltStandardHeaderExtra *)(msg.headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader));
    dlt_set_id(pStandardExtra->ecu, daemon->ecuid);
    pStandardExtra->tmsp = DLT_HTOBE_32(dlt_uptime());
    pStandardExtra->seid = DLT_HTOBE_32(getpid());

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
    uiSize = strlen(str) + 1;
    msg.datasize = sizeof(uint32_t) + sizeof(uint16_t) + uiSize;

    msg.databuffer = (uint8_t *)malloc(msg.datasize);
    msg.databuffersize = msg.datasize;

    if (msg.databuffer == 0) {
        dlt_log(LOG_WARNING, "Can't allocate buffer for get log info message\n");
        return -1;
    }

    msg.datasize = 0;
    memcpy((uint8_t *)(msg.databuffer + msg.datasize), (uint8_t *)(&uiType), sizeof(uint32_t));
    msg.datasize += sizeof(uint32_t);
    memcpy((uint8_t *)(msg.databuffer + msg.datasize), (uint8_t *)(&uiSize), sizeof(uint16_t));
    msg.datasize += sizeof(uint16_t);
    memcpy((uint8_t *)(msg.databuffer + msg.datasize), str, uiSize);
    msg.datasize += uiSize;

    /* Calc lengths */
    msg.standardheader->len = DLT_HTOBE_16(msg.headersize - sizeof(DltStorageHeader) + msg.datasize);

    /* Sending data... */
    {
        /* check if overflow occurred */
        if (daemon->overflow_counter) {
            if (dlt_daemon_send_message_overflow(daemon, daemon_local, verbose) == 0) {
                sprintf(str, "%u messages discarded!\n", daemon->overflow_counter);
                dlt_log(LOG_WARNING, str);
                daemon->overflow_counter = 0;
            }
        }

        /* look if TCP connection to client is available */
        if ((daemon->mode == DLT_USER_MODE_EXTERNAL) || (daemon->mode == DLT_USER_MODE_BOTH)) {

            if ((ret =
                     dlt_daemon_client_send(DLT_DAEMON_SEND_TO_ALL, daemon, daemon_local, msg.headerbuffer,
                                            sizeof(DltStorageHeader), msg.headerbuffer + sizeof(DltStorageHeader),
                                            msg.headersize - sizeof(DltStorageHeader),
                                            msg.databuffer, msg.datasize, verbose))) {
                if (ret == DLT_DAEMON_ERROR_BUFFER_FULL)
                    daemon->overflow_counter++;
            }
        }
    }

    free(msg.databuffer);

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

    snprintf(local_str,
             DLT_DAEMON_TEXTBUFSIZE,
             "New client connection #%d established, Total Clients : %d\n",
             in_sock,
             daemon_local->client_connections);
    dlt_log(LOG_DEBUG, local_str);
    dlt_daemon_log_internal(daemon, daemon_local, local_str, daemon_local->flags.vflag);

    if (daemon_local->client_connections == 1) {
        if (daemon_local->flags.vflag)
            dlt_log(LOG_DEBUG, "Send ring-buffer to client\n");

        dlt_daemon_change_state(daemon, DLT_DAEMON_STATE_SEND_BUFFER);

        if (dlt_daemon_send_ringbuffer_to_client(daemon, daemon_local, verbose) == -1) {
            dlt_log(LOG_WARNING, "Can't send contents of ringbuffer to clients\n");
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

    must_close_socket = dlt_receiver_receive(receiver, DLT_RECEIVE_SOCKET);

    if (must_close_socket < 0) {
        dlt_daemon_close_socket(receiver->fd,
                                daemon,
                                daemon_local,
                                verbose);
        receiver->fd = -1;
        return -1;
    }

    /* Process all received messages */
    while (dlt_message_read(&(daemon_local->msg),
                            (uint8_t *)receiver->buf,
                            receiver->bytesRcvd,
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

        bytes_to_be_removed = daemon_local->msg.headersize +
            daemon_local->msg.datasize -
            sizeof(DltStorageHeader);

        if (daemon_local->msg.found_serialheader)
            bytes_to_be_removed += sizeof(dltSerialHeader);

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

    if (dlt_receiver_receive(receiver, DLT_RECEIVE_FD) <= 0) {
        dlt_log(LOG_WARNING,
                "dlt_receiver_receive_fd() for messages from serial interface "
                "failed!\n");
        return -1;
    }

    /* Process all received messages */
    while (dlt_message_read(&(daemon_local->msg),
                            (uint8_t *)receiver->buf,
                            receiver->bytesRcvd,
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

        bytes_to_be_removed = daemon_local->msg.headersize +
            daemon_local->msg.datasize -
            sizeof(DltStorageHeader);

        if (daemon_local->msg.found_serialheader)
            bytes_to_be_removed += sizeof(dltSerialHeader);

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

    if (verbose) {
        snprintf(str, DLT_DAEMON_TEXTBUFSIZE,
                 "New connection to control client established\n");
        dlt_log(LOG_INFO, str);
    }

    return 0;
}

#ifdef DLT_USE_UNIX_SOCKET_IPC
int dlt_daemon_process_app_connect(
    DltDaemon *daemon,
    DltDaemonLocal *daemon_local,
    DltReceiver *receiver,
    int verbose)
{
    socklen_t app_size;
    struct sockaddr_un app;
    int in_sock = -1;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (receiver == NULL)) {
        dlt_vlog(LOG_ERR,
                 "%s: Invalid parameters\n",
                 __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* event from UNIX server socket, new connection */
    app_size = sizeof(app);

    if ((in_sock = accept(receiver->fd, (struct sockaddr *)&app, &app_size)) < 0) {
        dlt_vlog(LOG_ERR, "accept() on UNIX socket %d failed: %s\n", receiver->fd, strerror(errno));
        return -1;
    }

    /* check if file file descriptor was already used, and make it invalid if it
     *  is reused. This prevents sending messages to wrong file descriptor */
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

    if (verbose) {
        snprintf(str, DLT_DAEMON_TEXTBUFSIZE,
                 "New connection to application established\n");
        dlt_log(LOG_INFO, str);
    }

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

    if (dlt_receiver_receive(receiver, DLT_RECEIVE_SOCKET) <= 0) {
        dlt_daemon_close_socket(receiver->fd,
                                daemon,
                                daemon_local,
                                verbose);
        receiver->fd = -1;
        /* FIXME: Why the hell do we need to close the socket
         * on control message reception ??
         */
        return 0;
    }

    /* Process all received messages */
    while (dlt_message_read(
               &(daemon_local->msg),
               (uint8_t *)receiver->buf,
               receiver->bytesRcvd,
               daemon_local->flags.nflag,
               daemon_local->flags.vflag) == DLT_MESSAGE_ERROR_OK) {
        /* Check for control message */
        if ((receiver->fd > 0) &&
            DLT_MSG_IS_CONTROL_REQUEST(&(daemon_local->msg)))
            dlt_daemon_client_process_control(receiver->fd,
                                              daemon, daemon_local,
                                              &(daemon_local->msg),
                                              daemon_local->flags.vflag);

        bytes_to_be_removed = daemon_local->msg.headersize +
            daemon_local->msg.datasize -
            sizeof(DltStorageHeader);

        if (daemon_local->msg.found_serialheader)
            bytes_to_be_removed += sizeof(dltSerialHeader);

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
    char local_str[DLT_DAEMON_TEXTBUFSIZE] = { '\0' };
    DltUserHeader *userheader = (DltUserHeader *)(receiver->buf);
    (void)daemon;
    (void)daemon_local;

    PRINT_FUNCTION_VERBOSE(verbose);

    snprintf(local_str,
             DLT_DAEMON_TEXTBUFSIZE,
             "Invalid user message type received: %d!\n",
             userheader->message);
    dlt_log(LOG_ERR, local_str);

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
#ifdef DLT_SHM_ENABLE
    dlt_daemon_process_user_message_log_shm,
#else
    dlt_daemon_process_user_message_not_sup,
#endif
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
    int32_t min_size = (int32_t)sizeof(DltUserHeader);
    DltUserHeader *userheader;
    int recv;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (receiver == NULL)) {
        dlt_log(LOG_ERR,
                "Invalid function parameters used for function "
                "dlt_daemon_process_user_messages()\n");
        return -1;
    }

#ifdef DLT_USE_UNIX_SOCKET_IPC
    recv = dlt_receiver_receive(receiver, DLT_RECEIVE_SOCKET);

    if (recv <= 0) {
        dlt_daemon_close_socket(receiver->fd,
                                daemon,
                                daemon_local,
                                verbose);
        receiver->fd = -1;
        return 0;
    }

#else
    recv = dlt_receiver_receive(receiver, DLT_RECEIVE_FD);

    if (recv < 0) {
        dlt_log(LOG_WARNING,
                "dlt_receiver_receive_fd() for user messages failed!\n");
        return -1;
    }

#endif

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
    char local_str[DLT_DAEMON_TEXTBUFSIZE] = { '\0' };
    DltUserControlMsgBufferOverflow userpayload;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (rec == NULL)) {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "Invalid function parameters used for %s\n",
                 __func__);

        dlt_log(LOG_ERR, local_str);
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
    int to_remove = 0;
    DltDaemonApplication *application = NULL;
    DltDaemonApplication *old_application = NULL;
    pid_t old_pid = 0;
    char local_str[DLT_DAEMON_TEXTBUFSIZE] = { '\0' };
    char description[DLT_DAEMON_DESCSIZE + 1] = { '\0' };
    DltUserControlMsgRegisterApplication userapp;
    char *origin;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (rec == NULL)) {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "Invalid function parameters used for %s\n",
                 __func__);

        dlt_log(LOG_ERR, local_str);
        return -1;
    }

    memset(&userapp, 0, sizeof(DltUserControlMsgRegisterApplication));
    origin = rec->buf;

    /* We shall not remove data before checking that everything is there. */
    to_remove = dlt_receiver_check_and_get(rec,
                                           &userapp,
                                           len,
                                           DLT_RCV_SKIP_HEADER);

    if (to_remove < 0)
        /* Not enough bytes received */
        return -1;

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
        strncpy(description, "Unknown", sizeof("Unknown"));
        /* unknown len of original description, set to 0 to not remove in next
         * step. Because message buffer is re-adjusted the corrupted description
         * is ignored. */
        len = 0;
    }

    /* adjust to_remove */
    to_remove += sizeof(DltUserHeader) + len;
    /* point to begin of message */
    rec->buf = origin;

    /* We can now remove data. */
    if (dlt_receiver_remove(rec, to_remove) != DLT_RETURN_OK) {
        dlt_log(LOG_WARNING, "Can't remove bytes from receiver\n");
        return -1;
    }

    old_application = dlt_daemon_application_find(daemon, userapp.apid, daemon->ecuid, verbose);

    if (old_application != NULL)
        old_pid = old_application->pid;

    application = dlt_daemon_application_add(daemon,
                                             userapp.apid,
                                             userapp.pid,
                                             description,
                                             rec->fd,
                                             daemon->ecuid,
                                             verbose);

    /* send log state to new application */
    dlt_daemon_user_send_log_state(daemon, application, verbose);

    if (application == NULL) {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "Can't add ApplicationID '%.4s' for PID %d\n",
                 userapp.apid,
                 userapp.pid);
        dlt_log(LOG_WARNING, local_str);
        return -1;
    }
    else if (old_pid != application->pid) {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "ApplicationID '%.4s' registered for PID %d, Description=%s\n",
                 application->apid,
                 application->pid,
                 application->application_description);
        dlt_daemon_log_internal(daemon,
                                daemon_local,
                                local_str,
                                daemon_local->flags.vflag);
        dlt_log(LOG_DEBUG, local_str);
    }

    return 0;
}

int dlt_daemon_process_user_message_register_context(DltDaemon *daemon,
                                                     DltDaemonLocal *daemon_local,
                                                     DltReceiver *rec,
                                                     int verbose)
{
    char local_str[DLT_DAEMON_TEXTBUFSIZE] = { '\0' };
    int to_remove = 0;
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
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "Invalid function parameters used for %s\n",
                 __func__);

        dlt_log(LOG_ERR, local_str);
        return -1;
    }

    memset(&userctxt, 0, sizeof(DltUserControlMsgRegisterContext));
    origin = rec->buf;

    to_remove = dlt_receiver_check_and_get(rec,
                                           &userctxt,
                                           len,
                                           DLT_RCV_SKIP_HEADER);

    if (to_remove < 0)
        /* Not enough bytes received */
        return -1;

    len = userctxt.description_length;

    if (len > DLT_DAEMON_DESCSIZE) {
        dlt_vlog(LOG_WARNING, "Context description exceeds limit: %d\n", len);
        len = DLT_DAEMON_DESCSIZE;
    }

    /* adjust buffer pointer */
    rec->buf += to_remove + sizeof(DltUserHeader);

    if (dlt_receiver_check_and_get(rec, description, len, DLT_RCV_NONE) < 0) {
        dlt_log(LOG_ERR, "Unable to get context description\n");
        /* in case description was not readable, set dummy description */
        strncpy(description, "Unknown", sizeof("Unknown"));
        /* unknown len of original description, set to 0 to not remove in next
         * step. Because message buffer is re-adjusted the corrupted description
         * is ignored. */
        len = 0;
    }

    /* adjust to_remove */
    to_remove += sizeof(DltUserHeader) + len;
    /* point to begin of message */
    rec->buf = origin;

    /* We can now remove data. */
    if (dlt_receiver_remove(rec, to_remove) != DLT_RETURN_OK) {
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
    }
    else
    /* Plausibility check */
    if ((userctxt.log_level < DLT_LOG_DEFAULT) ||
        (userctxt.log_level > DLT_LOG_VERBOSE))
        return -1;

    /* Set trace status */
    if (userctxt.trace_status == DLT_USER_TRACE_STATUS_NOT_SET) {
        userctxt.trace_status = DLT_TRACE_STATUS_DEFAULT;
    }
    else
    /* Plausibility check */
    if ((userctxt.trace_status < DLT_TRACE_STATUS_DEFAULT) ||
        (userctxt.trace_status > DLT_TRACE_STATUS_ON))
        return -1;

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
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "Can't add ContextID '%.4s' for ApID '%.4s'\n in %s",
                 userctxt.ctid,
                 userctxt.apid,
                 __func__);
        dlt_log(LOG_WARNING, local_str);
        return -1;
    }
    else {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "ContextID '%.4s' registered for ApID '%.4s', Description=%s\n",
                 context->ctid,
                 context->apid,
                 context->context_description);

        if (verbose)
            dlt_daemon_log_internal(daemon, daemon_local, local_str, verbose);

        dlt_log(LOG_DEBUG, local_str);
    }

    if (daemon_local->flags.offlineLogstorageMaxDevices)
        /* Store log level set for offline logstorage into context structure*/
        context->storage_log_level =
            dlt_daemon_logstorage_get_loglevel(daemon,
                                               daemon_local->flags.offlineLogstorageMaxDevices,
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
        req->options = daemon_local->flags.autoResponseGetLogInfoOption;
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
    char local_str[DLT_DAEMON_TEXTBUFSIZE] = { '\0' };
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
                snprintf(local_str,
                         DLT_DAEMON_TEXTBUFSIZE,
                         "Unregistered ApID '%.4s'\n",
                         userapp.apid);
                dlt_daemon_log_internal(daemon,
                                        daemon_local,
                                        local_str,
                                        verbose);
                dlt_log(LOG_DEBUG, local_str);
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
    char local_str[DLT_DAEMON_TEXTBUFSIZE] = { '\0' };
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

    if (context) {
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
            snprintf(local_str,
                     DLT_DAEMON_TEXTBUFSIZE,
                     "Unregistered CtID '%.4s' for ApID '%.4s'\n",
                     userctxt.ctid,
                     userctxt.apid);

            if (verbose)
                dlt_daemon_log_internal(daemon,
                                        daemon_local,
                                        local_str,
                                        verbose);

            dlt_log(LOG_DEBUG, local_str);
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
    int ret;
    int bytes_to_be_removed;

    static char text[DLT_DAEMON_TEXTSIZE];

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (rec == NULL)) {
        dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_user_message_log()\n");
        return DLT_DAEMON_ERROR_UNKNOWN;
    }

    ret = dlt_message_read(&(daemon_local->msg),
                           (unsigned char *)rec->buf + sizeof(DltUserHeader),
                           rec->bytesRcvd - sizeof(DltUserHeader),
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

        return DLT_DAEMON_ERROR_UNKNOWN;
    }

    /* set overwrite ecu id */
    if ((daemon_local->flags.evalue[0]) && (strncmp(daemon_local->msg.headerextra.ecu, DLT_DAEMON_ECU_ID, 4) == 0)) {
        /* Set header extra parameters */
        dlt_set_id(daemon_local->msg.headerextra.ecu, daemon->ecuid);

        /*msg.headerextra.seid = 0; */
        if (dlt_message_set_extraparameters(&(daemon_local->msg), 0) == DLT_RETURN_ERROR) {
            dlt_log(LOG_WARNING, "Can't set message extra parameters in process user message log\n");
            return DLT_DAEMON_ERROR_UNKNOWN;
        }

        /* Correct value of timestamp, this was changed by dlt_message_set_extraparameters() */
        daemon_local->msg.headerextra.tmsp = DLT_BETOH_32(daemon_local->msg.headerextra.tmsp);
    }

    /* prepare storage header */
    if (DLT_IS_HTYP_WEID(daemon_local->msg.standardheader->htyp)) {
        if (dlt_set_storageheader(daemon_local->msg.storageheader,
                                  daemon_local->msg.headerextra.ecu) == DLT_RETURN_ERROR) {
            dlt_log(LOG_WARNING, "Can't set storage header in process user message log\n");
            return DLT_DAEMON_ERROR_UNKNOWN;
        }
    }
    else if (dlt_set_storageheader(daemon_local->msg.storageheader, daemon->ecuid) == DLT_RETURN_ERROR) {
        dlt_log(LOG_WARNING, "Can't set storage header in process user message log\n");
        return DLT_DAEMON_ERROR_UNKNOWN;
    }

    {
        /* if no filter set or filter is matching display message */
        if (daemon_local->flags.xflag) {
            if (dlt_message_print_hex(&(daemon_local->msg), text, DLT_DAEMON_TEXTSIZE, verbose) == DLT_RETURN_ERROR)
                dlt_log(LOG_WARNING, "dlt_message_print_hex() failed!\n");
        } /*  if */
        else if (daemon_local->flags.aflag)
        {
            if (dlt_message_print_ascii(&(daemon_local->msg), text, DLT_DAEMON_TEXTSIZE, verbose) == DLT_RETURN_ERROR)
                dlt_log(LOG_WARNING, "dlt_message_print_ascii() failed!\n");
        } /* if */
        else if (daemon_local->flags.sflag)
        {
            if (dlt_message_print_header(&(daemon_local->msg), text, DLT_DAEMON_TEXTSIZE, verbose) == DLT_RETURN_ERROR)
                dlt_log(LOG_WARNING, "dlt_message_print_header() failed!\n");

            /* print message header only */
        } /* if */

        /* check if overflow occurred */
        if (daemon->overflow_counter) {
            if (dlt_daemon_send_message_overflow(daemon, daemon_local, verbose) == 0) {
                snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "%u messages discarded!\n", daemon->overflow_counter);
                dlt_log(LOG_WARNING, str);
                daemon->overflow_counter = 0;
            }
        }

        /* send message to client or write to log file */
        if ((ret =
                 dlt_daemon_client_send(DLT_DAEMON_SEND_TO_ALL, daemon, daemon_local, daemon_local->msg.headerbuffer,
                                        sizeof(DltStorageHeader),
                                        daemon_local->msg.headerbuffer + sizeof(DltStorageHeader),
                                        daemon_local->msg.headersize - sizeof(DltStorageHeader),
                                        daemon_local->msg.databuffer, daemon_local->msg.datasize, verbose))) {
            if (ret == DLT_DAEMON_ERROR_BUFFER_FULL)
                daemon->overflow_counter++;
        }
    }

    /* keep not read data in buffer */
    bytes_to_be_removed = daemon_local->msg.headersize + daemon_local->msg.datasize - sizeof(DltStorageHeader) +
        sizeof(DltUserHeader);

    if (daemon_local->msg.found_serialheader)
        bytes_to_be_removed += sizeof(dltSerialHeader);

    if (dlt_receiver_remove(rec, bytes_to_be_removed) == -1) {
        dlt_log(LOG_WARNING, "Can't remove bytes from receiver\n");
        return DLT_DAEMON_ERROR_UNKNOWN;
    }

    return DLT_DAEMON_ERROR_OK;
}

#ifdef DLT_SHM_ENABLE
#   define DLT_SHM_RCV_BUFFER_SIZE 10000
int dlt_daemon_process_user_message_log_shm(DltDaemon *daemon,
                                            DltDaemonLocal *daemon_local,
                                            DltReceiver *rec,
                                            int verbose)
{
    char local_str[DLT_DAEMON_TEXTBUFSIZE] = { '\0' };
    int sent;
    uint8_t *rcv_buffer = NULL;
    int size;
    uint32_t len = sizeof(DltUserHeader);
    DltUserHeader userheader;

    static char text[DLT_DAEMON_TEXTSIZE];

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (rec == NULL)) {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "Invalid function parameters used for %s\n",
                 __func__);

        dlt_log(LOG_ERR, local_str);
        return -1;
    }

    rcv_buffer = calloc(1, DLT_SHM_RCV_BUFFER_SIZE);

    if (!rcv_buffer) {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "No memory to allocate receiver buffer in %s.\n",
                 __func__);

        dlt_log(LOG_ERR, local_str);

        return -1;
    }

    memset(&userheader, 0, len);

    if (dlt_receiver_check_and_get(rec, &userheader, len, DLT_RCV_REMOVE) < 0)
        /* Not enough bytes received */
        return -1;

    /*dlt_shm_status(&(daemon_local->dlt_shm)); */
    while (1) {
        /* log message in SHM */
        size = dlt_shm_copy(&(daemon_local->dlt_shm),
                            rcv_buffer,
                            DLT_SHM_RCV_BUFFER_SIZE);

        if (size <= 0)
            break;

        if (dlt_message_read(&(daemon_local->msg), rcv_buffer, size, 0, verbose) != 0) {
            break;
            dlt_log(LOG_WARNING, "Can't read messages from shm\n");
            return -1;
        }

        /* set overwrite ecu id */
        if ((daemon_local->flags.evalue[0]) &&
            (strncmp(daemon_local->msg.headerextra.ecu, DLT_DAEMON_ECU_ID, 4) == 0)) {
            /* Set header extra parameters */
            dlt_set_id(daemon_local->msg.headerextra.ecu, daemon->ecuid);

            /*msg.headerextra.seid = 0; */
            if (dlt_message_set_extraparameters(&(daemon_local->msg), 0) == -1) {
                dlt_log(LOG_WARNING, "Can't set message extra parameters in process user message log\n");
                dlt_shm_remove(&(daemon_local->dlt_shm));
                return -1;
            }

            /* Correct value of timestamp, this was changed by dlt_message_set_extraparameters() */
            daemon_local->msg.headerextra.tmsp = DLT_BETOH_32(daemon_local->msg.headerextra.tmsp);
        }

        /* prepare storage header */
        if (DLT_IS_HTYP_WEID(daemon_local->msg.standardheader->htyp)) {
            if (dlt_set_storageheader(daemon_local->msg.storageheader, daemon_local->msg.headerextra.ecu) == -1) {
                dlt_log(LOG_WARNING, "Can't set storage header in process user message log\n");
                dlt_shm_remove(&(daemon_local->dlt_shm));
                return -1;
            }
        }
        else if (dlt_set_storageheader(daemon_local->msg.storageheader, daemon->ecuid) == -1) {
            dlt_log(LOG_WARNING, "Can't set storage header in process user message log\n");
            dlt_shm_remove(&(daemon_local->dlt_shm));
            return -1;
        }

        /* display message */
        if (daemon_local->flags.xflag) {
            if (dlt_message_print_hex(&(daemon_local->msg), text, DLT_DAEMON_TEXTSIZE, verbose) == -1)
                dlt_log(LOG_WARNING, "dlt_message_print_hex() failed!\n");
        } /*  if */
        else if (daemon_local->flags.aflag)
        {
            if (dlt_message_print_ascii(&(daemon_local->msg), text, DLT_DAEMON_TEXTSIZE, verbose) == -1)
                dlt_log(LOG_WARNING, "dlt_message_print_ascii() failed!\n");
        } /* if */
        else if (daemon_local->flags.sflag)
        {
            if (dlt_message_print_header(&(daemon_local->msg), text, DLT_DAEMON_TEXTSIZE, verbose) == -1)
                dlt_log(LOG_WARNING, "dlt_message_print_header() failed!\n");

            /* print message header only */
        } /* if */

        sent = 0;

        /* write message to offline trace */
        if (daemon_local->flags.offlineTraceDirectory[0]) {
            dlt_offline_trace_write(&(daemon_local->offlineTrace),
                                    daemon_local->msg.headerbuffer,
                                    daemon_local->msg.headersize,
                                    daemon_local->msg.databuffer,
                                    daemon_local->msg.datasize,
                                    0,
                                    0);
            sent = 1;
        }

        sent = dlt_daemon_client_send_all(daemon, daemon_local, verbose);

        /* Message was not sent to client, so store it in client ringbuffer */
        if (sent == 1) {
            if (userheader.message == DLT_USER_MESSAGE_LOG_SHM)
                /* dlt message was sent, remove from buffer if log message from shm */
                dlt_shm_remove(&(daemon_local->dlt_shm));
        }
        else {
            /* dlt message was not sent, keep in buffer */
            break;
        }
    }

    return 0;
}
#   undef DLT_SHM_RCV_BUFFER_SIZE
#endif

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
                    context->log_level = userctxt.log_level; /* No endianess conversion necessary*/

                    old_trace_status = context->trace_status;
                    context->trace_status = userctxt.trace_status;   /* No endianess conversion necessary */

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
    char local_str[DLT_DAEMON_TEXTBUFSIZE] = { '\0' };
    uint32_t len = sizeof(DltUserControlMsgLogMode);
    DltUserControlMsgLogMode userctxt;
    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (rec == NULL)) {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "Invalid function parameters used for %s\n",
                 __func__);

        dlt_log(LOG_ERR, local_str);
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
        dlt_log(LOG_WARNING, "Could not reset systemd watchdog\n");

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

int create_timer_fd(DltDaemonLocal *daemon_local,
                    int period_sec,
                    int starts_in,
                    DltTimers timer_id)
{
    int local_fd = -1;
    char *timer_name = NULL;

    if (timer_id >= DLT_TIMER_UNKNOWN) {
        dlt_log(DLT_LOG_ERROR, "Unknown timer.");
        return -1;
    }

    timer_name = dlt_timer_names[timer_id];

    if (daemon_local == NULL) {
        dlt_log(DLT_LOG_ERROR, "Daemaon local structure is NULL");
        return -1;
    }

    if (period_sec <= 0 || starts_in <= 0 ) {
        /* timer not activated via the service file */
        dlt_vlog(LOG_INFO, "<%s> not set: period=0\n", timer_name);
        local_fd = -1;
    }
#ifdef linux
    else
    {
        struct itimerspec l_timer_spec;
        local_fd = timerfd_create(CLOCK_MONOTONIC, 0);

        if (local_fd < 0) {
            snprintf(str,
                     sizeof(str),
                     "<%s> timerfd_create failed: %s\n",
                     timer_name,
                     strerror(errno));
            dlt_log(LOG_WARNING, str);
        }

        l_timer_spec.it_interval.tv_sec = period_sec;
        l_timer_spec.it_interval.tv_nsec = 0;
        l_timer_spec.it_value.tv_sec = starts_in;
        l_timer_spec.it_value.tv_nsec = 0;

        if (timerfd_settime(local_fd, 0, &l_timer_spec, NULL) < 0) {
            snprintf(str,
                     sizeof(str),
                     "<%s> timerfd_settime failed: %s\n",
                     timer_name,
                     strerror(errno));
            dlt_log(LOG_WARNING, str);
            local_fd = -1;
        }
    }
#endif

    /* If fully initialized we are done.
     * Event handling registration is done later on with other connections.
     */
    if (local_fd > 0) {
        snprintf(str,
                 sizeof(str),
                 "<%s> initialized with %d timer\n",
                 timer_name,
                 period_sec);
        dlt_log(LOG_INFO, str);
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
             "Client connection #%d closed. Total Clients : %d\n",
             sock,
             daemon_local->client_connections);
    dlt_log(LOG_DEBUG, local_str);
    dlt_daemon_log_internal(daemon, daemon_local, local_str, daemon_local->flags.vflag);

    return 0;
}

/**
 \}
 */
