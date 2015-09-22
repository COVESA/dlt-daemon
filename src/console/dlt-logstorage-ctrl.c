/*
 * @licence app begin@
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2013 - 2015  Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO
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
 * \author Syed Hameed <shameed@jp.adit-jv.com> ADIT 2013 - 2015
 * \author Christoph Lipka <clipka@jp.adit-jv.com> ADIT 2015
 *
 * \copyright Copyright © 2013-2015 Advanced Driver Information Technology. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-logstorage-ctrl.c
*/

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-logstorage-ctrl.c                                         **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Syed Hameed shameed@jp.adit-jv.com                            **
**              Christoph Lipka clipka@jp.adit-jv.com                         **
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
**  sh          Syed Hameed                ADIT                               **
**  cl          Christoph Lipka            ADIT                               **
*******************************************************************************/

#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

#include "dlt_common.h"
#include "dlt_client.h"
#include "dlt_protocol.h"

#define DLT_RECEIVE_TEXTBUFSIZE 1024
#define DLT_DAEMON_ECUID_DEFAULT "ECU1"
#define DLT_DAEMON_ECUID_LEN 10
#define DLT_LOGSTORAGE_CTRL_TIMEOUT 10

DltClient dltclient;

/* Result of callback function */
int g_logstorage_callback_return = -1;

const char *serverIP = "localhost";

typedef struct
{
    int connection_type;              /* Conntection Type: 1 (connect), 0 (disconnect) */
    int device_number;                /* Number of Logstorage device [1 .. MAX DEVICES] */
    char ecuid[DLT_DAEMON_ECUID_LEN]; /* Name of ECU */
} DltLogstorageCtrlData;

void usage()
{
    printf("Usage: dlt-logstorage-ctrl [options]\n");
    printf("Send a trigger to DLT daemon to connect/disconnect a certain logstorage device\n");
    printf("\n");
    printf("Options:\n");
    printf("  -c         Connection type: connect = 1, disconnect = 0\n");
    printf("  -d         Device: [1 .. DLT_OFFLINE_LOGSTORAGE_MAX_DEVICES]\n");
    printf("  -e         Set ECU ID (Default: %s\n", DLT_DAEMON_ECUID_DEFAULT);
    printf("  -h         Usage\n");
    printf("  -t         Specify connection timeout (Default: %ds)\n", DLT_LOGSTORAGE_CTRL_TIMEOUT);
}

int dlt_logstorage_message_callback(DltMessage *message,void *data);
int dlt_logstorage_analyze_response(char *data);

/*
 * Prepare request message and send to Dlt Daemon. Since Logstorage control application acts
 * as Dlt client, we use the same functionality found in
 * dlt_daemon_client.c: dlt_daemon_client_send_control_message()
 */
int dlt_send_logstorage_request(int sockid, int device_number, int connection_type, char *ecuid)
{
    DltMessage msg;
    DltServiceOfflineLogstorage *req = NULL;
    int32_t len = 0;
    int ret = 0;

    /* Initialise new message */
    if (dlt_message_init(&msg, 0) == -1)
    {
        fprintf( stderr, "Cannot initialize DltMessage\n");
        return -1;
    }

    msg.datasize = sizeof(DltServiceOfflineLogstorage);

    /* prepare payload of data */
    if (msg.databuffer && (msg.databuffersize < msg.datasize))
    {
        free(msg.databuffer);
        msg.databuffer = 0;
    }
    if (msg.databuffer == 0)
    {
        /* malloc databuffer for DltMessage */
        msg.databuffer = (uint8_t *) malloc(msg.datasize);
        msg.databuffersize = msg.datasize;
    }
    if(msg.databuffer == 0)
    {
        fprintf( stderr, "Cannot allocate memory for data buffer\n");
        dlt_message_free(&msg, 0);
        return -1;
    }

    req = (DltServiceOfflineLogstorage*) msg.databuffer;

    req->service_id = DLT_SERVICE_ID_OFFLINE_LOGSTORAGE;
    req->dev_num = device_number;
    req->connection_type = connection_type;
    dlt_set_id(req->comid,"remo");

    /* prepare storage header */
    msg.storageheader = (DltStorageHeader*) msg.headerbuffer;
    if (dlt_set_storageheader(msg.storageheader, "") == -1)
    {
        fprintf( stderr, "Init storage header failed\n");
        dlt_message_free(&msg, 0);
        return -1;
    }

    /* prepare standard header */
    msg.standardheader = (DltStandardHeader*)(msg.headerbuffer + sizeof(DltStorageHeader));
    msg.standardheader->htyp = DLT_HTYP_WEID | DLT_HTYP_WTMS | DLT_HTYP_UEH | DLT_HTYP_PROTOCOL_VERSION1;

#if (BYTE_ORDER==BIG_ENDIAN)
    msg.standardheader->htyp = (msg.standardheader->htyp | DLT_HTYP_MSBF);
#endif

    msg.standardheader->mcnt = 0;

    /* Set header extra parameters */
    if (ecuid == NULL)
    {
        dlt_set_id(msg.headerextra.ecu,DLT_DAEMON_ECUID_DEFAULT);
    }
    else
    {
        dlt_set_id(msg.headerextra.ecu,ecuid);
    }

    msg.headerextra.tmsp = dlt_uptime();

    /* Copy header extra parameters to headerbuffer */
    if (dlt_message_set_extraparameters(&msg, 0) == -1)
    {
        fprintf( stderr, "Cannot copy header extra parameter\n");
        dlt_message_free(&msg,0);
        return -1;
    }

    /* prepare extended header */
    msg.extendedheader = (DltExtendedHeader*)(msg.headerbuffer +
                         sizeof(DltStorageHeader) +
                         sizeof(DltStandardHeader) +
                         DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp) );

    msg.extendedheader->msin = DLT_MSIN_CONTROL_REQUEST;

    msg.extendedheader->noar = 1; /* one payload packet */

    /* Dummy values have to be set: Offline_Log_Storage_Control, Context1 */
    dlt_set_id(msg.extendedheader->apid, "OLSC");
    dlt_set_id(msg.extendedheader->ctid, "CON1");

    /* prepare length information */
    msg.headersize = sizeof(DltStorageHeader) +
                     sizeof(DltStandardHeader) +
                     sizeof(DltExtendedHeader) +
                     DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp);

    len = msg.headersize - sizeof(DltStorageHeader) + msg.datasize;
    if (len > UINT16_MAX)
    {
        fprintf( stderr, "Message header too long\n");
        dlt_message_free(&msg, 0);
        return -1;
    }

    msg.standardheader->len = DLT_HTOBE_16(len);

    /* Send via Socket */
    if (send(sockid, (const char *)(msg.headerbuffer+sizeof(DltStorageHeader)), msg.headersize-sizeof(DltStorageHeader), 0) == -1)
    {
        fprintf( stderr, "Sending message to socket failed: %s\n", strerror(errno));
        ret = -1;
    }

    if (send(sockid, (const char *) msg.databuffer,msg.datasize, 0) == -1)
    {
        fprintf( stderr, "Sending message to socket failed: %s\n", strerror(errno));
        ret = -1;
    }

    /* free message */
    if (dlt_message_free(&msg, 0) == -1)
    {
        fprintf( stderr, "free DltMessage failed\n");
        return -1;
    }

    return ret;
}

int dlt_logstorage_message_callback(DltMessage *message, void *data)
{
    char text[DLT_RECEIVE_TEXTBUFSIZE] = {0};
    int ret = 0;

    (void)data; /* ignore data */

    if (message == 0)
    {
        fprintf( stderr, "Received message is null\n");
        g_logstorage_callback_return = -1;
        return -1;
    }

    /* prepare storage header */
    if (DLT_IS_HTYP_WEID(message->standardheader->htyp))
    {
        dlt_set_storageheader(message->storageheader, message->headerextra.ecu);
    }
    else
    {
        dlt_set_storageheader(message->storageheader, "LCTL");
    }

    dlt_message_header(message, text, DLT_RECEIVE_TEXTBUFSIZE, 1);

    /* payload contains "DLT_SERVICE_ID_OFFLINE_LOGSTORAGE, DLT_SERVICE_RESPONSE" as string
     * that has to be checked */
    dlt_message_payload(message, text, DLT_RECEIVE_TEXTBUFSIZE,DLT_OUTPUT_ASCII, 1);

    ret = dlt_logstorage_analyze_response(text);
    dlt_client_cleanup(&dltclient, 1);

    g_logstorage_callback_return = ret;
    return ret;
}

/*
 * Communicate with daemon in a thread to detect communication errors (timeout).
 */
void *communicate_with_daemon (void *data)
{
    DltLogstorageCtrlData *ctrl = (DltLogstorageCtrlData *) data;
    /* Initialize DLT Client */
    dlt_client_init(&dltclient, 1);

    /* Register callback to be called when message was received */
    dlt_client_register_message_callback(dlt_logstorage_message_callback);

    dltclient.servIP = "localhost";

    /* Connect to TCP socket or open serial device */
    dlt_client_connect(&dltclient, 1);

    if (dlt_send_logstorage_request(dltclient.sock, ctrl->device_number, ctrl->connection_type, ctrl->ecuid) != 0)
    {
        fprintf(stderr, "dlt_send_logstorage_request failed. Exit.\n");
    }
    else
    {
        /* Dlt Client Main Loop */
        dlt_client_main_loop(&dltclient, NULL, 1);
    }

    return (void *)0;
}

/*
 * Resonse received in form of "service(<id>), <ok, error>
 * check if id is DLT_SERVICE_ID_OFFLINE_LOGSTORAGE and return value either ok or error.
 */
int dlt_logstorage_analyze_response(char *data)
{
    int ret = 0;
    char resp_ok[20] = {0};
    char resp_nok[20] = {0};

    if (data == NULL)
        return -1;

    snprintf(resp_ok, 20, "service(%u), ok", DLT_SERVICE_ID_OFFLINE_LOGSTORAGE);
    snprintf(resp_nok, 20, "service(%u), error", DLT_SERVICE_ID_OFFLINE_LOGSTORAGE);

    if (strncmp(data, resp_ok, strlen(resp_ok)) == 0)
    {
        dlt_client_cleanup(&dltclient, 1);
        ret = 0;
    }
    else if (strncmp(data, resp_nok, strlen(resp_nok)) == 0)
    {
        dlt_client_cleanup(&dltclient, 1);
        ret = -1;
    }
    else
    {
        /* fall through */
    }

    g_logstorage_callback_return = ret;
    return ret;
}

int main(int argc, char *argv[])
{
    DltLogstorageCtrlData ctrl_data;
    pthread_t daemon_connect_thread;
    int c = 0;
    int ret = 0;
    int timeout = DLT_LOGSTORAGE_CTRL_TIMEOUT;
    struct timespec t;

    /* Initialize control data */
    memset(&ctrl_data, 0, sizeof(DltLogstorageCtrlData));

    /* Get command line arguments */
    opterr = 0;

    while ((c = getopt(argc, argv, "t:he:d:c:")) != -1)
    {
        switch(c)
        {
        case 't':
            timeout = (int) strtol(optarg,NULL, 10);
            if (timeout <= 1)
            {
                fprintf(stderr, "Timeout to small. Set to default: %d", DLT_LOGSTORAGE_CTRL_TIMEOUT);
                timeout = DLT_LOGSTORAGE_CTRL_TIMEOUT;
            }
            break;
        case 'h':
            usage();
            return -1;
        case 'e':
            strncpy(ctrl_data.ecuid, optarg, DLT_DAEMON_ECUID_LEN);
            break;
        case 'd':
            ctrl_data.device_number = (int) strtol(optarg, NULL, 10);
            break;
        case 'c':
            ctrl_data.connection_type = (int) strtol(optarg, NULL, 10);
            break;
        case '?':
            if (optopt == 'c' || optopt == 'd' || optopt == 'e')
            {
                 fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            }
            else if (isprint(optopt))
            {
                fprintf(stderr, "Unknown option -%c.\n", optopt);
            }
            else
            {
                fprintf(stderr, "Unknown option character \\x%x.\n", optopt);
            }

            usage();
            return -1;
        default:
            fprintf(stdout, "Try %s -h for more information.\n", argv[0]);
            return -1;
        }
    }

    /* Check if device number and connection type configured correclty */
    if (ctrl_data.connection_type < 0 || ctrl_data.connection_type > 1)
    {
        fprintf(stderr, "Please check connection type parameter. Try %s -h for more information.\n", argv[0]);
        return -1;
    }

    if (ctrl_data.device_number <= 0)
    {
        fprintf(stderr, "Please check device parameter. Try %s -h for more information.\n", argv[0]);
        return -1;
    }

    /* Inside DLT Logstorage, devices are stored in an array. This starts with device 0
     * Therefore, we decrease given device number by 1 */
    ctrl_data.device_number -= 1;

    if (clock_gettime(CLOCK_REALTIME, &t) == -1)
    {
        fprintf(stderr, "Cannot read system time.\n");
        return -1;
    }

    /* set timeout */
    t.tv_sec += timeout;

    /* Contact DLT daemon */
    if (pthread_create(&daemon_connect_thread, NULL, communicate_with_daemon, &ctrl_data) != 0) {
        fprintf(stderr, "Cannot create thread to communicate with DLT daemon.\n");
        return -1;
    }

    /* Wait for thread */
    ret = pthread_timedjoin_np(daemon_connect_thread, NULL, &t);

    if (ret != 0)
    {
        fprintf( stderr, "Connection to Dlt Daemon timed out.\n");
        fprintf( stderr, "Sending command to DLT daemon was not successful.\n");
    }
    else
    {
        /* Check callback return value */
        if (g_logstorage_callback_return != 0) {
            fprintf( stderr, "Sending command to DLT daemon was not successful.\n");
            ret = -1;
        }
    }

    return ret;
}
