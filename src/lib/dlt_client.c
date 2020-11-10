/*
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
 */

/*!
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_client.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_client.c                                                  **
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
**                      Revision Control History                              **
*******************************************************************************/

/*
 * $LastChangedRevision$
 * $LastChangedDate$
 * $LastChangedBy$
 * Initials    Date         Comment
 * aw          12.07.2010   initial
 */

#include <stdio.h>

#if defined (__WIN32__) || defined (_MSC_VER)
#   pragma warning(disable : 4996) /* Switch off C4996 warnings */
#   include <winsock2.h> /* for socket(), connect(), send(), and recv() */
#else
#   include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#   include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#   include <netdb.h>
#   include <sys/stat.h>
#   include <sys/un.h>
#endif

#if defined(_MSC_VER)
#   include <io.h>
#else
#   include <unistd.h>
#   include <syslog.h>
#endif

#include <fcntl.h>

#include <stdlib.h> /* for malloc(), free() */
#include <string.h> /* for strlen(), memcmp(), memmove() */
#include <errno.h>
#include <limits.h>

#include "dlt_types.h"
#include "dlt_client.h"
#include "dlt_client_cfg.h"

static int (*message_callback_function)(DltMessage *message, void *data) = NULL;

void dlt_client_register_message_callback(int (*registerd_callback)(DltMessage *message, void *data))
{
    message_callback_function = registerd_callback;
}

DltReturnValue dlt_client_init_port(DltClient *client, int port, int verbose)
{
    if (verbose && (port != DLT_DAEMON_TCP_PORT))
        dlt_vlog(LOG_INFO, "Init dlt client struct with port %d\n", port);

    if (client == NULL)
        return DLT_RETURN_ERROR;

    client->sock = -1;
    client->servIP = NULL;
    client->serialDevice = NULL;
    client->baudrate = DLT_CLIENT_INITIAL_BAUDRATE;
    client->port = port;
    client->socketPath = NULL;
    client->mode = DLT_CLIENT_MODE_TCP;
    client->receiver.buffer = NULL;
    client->receiver.buf = NULL;
    client->receiver.backup_buf = NULL;
    client->hostip = NULL;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_client_init(DltClient *client, int verbose)
{
    char *env_daemon_port;
    int tmp_port;
    /* the port may be specified by an environment variable, defaults to DLT_DAEMON_TCP_PORT */
    unsigned short servPort = DLT_DAEMON_TCP_PORT;

    /* the port may be specified by an environment variable */
    env_daemon_port = getenv(DLT_CLIENT_ENV_DAEMON_TCP_PORT);

    if (env_daemon_port != NULL) {
        tmp_port = atoi(env_daemon_port);

        if ((tmp_port < IPPORT_RESERVED) || ((unsigned)tmp_port > USHRT_MAX)) {
            dlt_vlog(LOG_ERR,
                     "Specified port is out of possible range: %d.\n",
                     tmp_port);
            return DLT_RETURN_ERROR;
        }
        else {
            servPort = (unsigned short)tmp_port;
        }
    }

    if (verbose)
        dlt_vlog(LOG_INFO,
                 "Init dlt client struct with default port: %hu.\n",
                 servPort);
    return dlt_client_init_port(client, servPort, verbose);
}

DltReturnValue dlt_client_connect(DltClient *client, int verbose)
{
    const int yes = 1;
    int connect_errno = 0;
    char portnumbuffer[33];
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_un addr;
    int rv;
    struct ip_mreq mreq;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;

    if (client == 0)
        return DLT_RETURN_ERROR;

    switch (client->mode) {
    case DLT_CLIENT_MODE_TCP:
        snprintf(portnumbuffer, 32, "%d", client->port);

        if ((rv = getaddrinfo(client->servIP, portnumbuffer, &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return DLT_RETURN_ERROR;
        }

        for (p = servinfo; p != NULL; p = p->ai_next) {
            if ((client->sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
                dlt_vlog(LOG_WARNING, "socket() failed! %s\n", strerror(errno));
                continue;
            }

            if (connect(client->sock, p->ai_addr, p->ai_addrlen) < 0) {
                connect_errno = errno;
                close(client->sock);
                continue;
            }

            break;
        }

        freeaddrinfo(servinfo);

        if (p == NULL) {
            dlt_vlog(LOG_ERR, "ERROR: failed to connect! %s\n", strerror(connect_errno));
            return DLT_RETURN_ERROR;
        }

        if (verbose)
            printf("Connected to DLT daemon (%s)\n", client->servIP);

        break;
    case DLT_CLIENT_MODE_SERIAL:
        /* open serial connection */
        client->sock = open(client->serialDevice, O_RDWR);

        if (client->sock < 0) {
            fprintf(stderr, "ERROR: Failed to open device %s\n", client->serialDevice);
            return DLT_RETURN_ERROR;
        }

        if (isatty(client->sock)) {
            #if !defined (__WIN32__)

            if (dlt_setup_serial(client->sock, client->baudrate) < DLT_RETURN_OK) {
                fprintf(stderr, "ERROR: Failed to configure serial device %s (%s) \n", client->serialDevice,
                        strerror(errno));
                return DLT_RETURN_ERROR;
            }

            #else
            return DLT_RETURN_ERROR;
            #endif
        }
        else {
            if (verbose)
                fprintf(stderr,
                        "ERROR: Device is not a serial device, device = %s (%s) \n",
                        client->serialDevice,
                        strerror(errno));

            return DLT_RETURN_ERROR;
        }

        if (verbose)
            printf("Connected to %s\n", client->serialDevice);

        break;
    case DLT_CLIENT_MODE_UNIX:

        if ((client->sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
            fprintf(stderr, "ERROR: (unix) socket error: %s\n", strerror(errno));
            return DLT_RETURN_ERROR;
        }

        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        memcpy(addr.sun_path, client->socketPath, sizeof(addr.sun_path) - 1);

        if (connect(client->sock,
                    (struct sockaddr *) &addr,
                    sizeof(addr)) == -1) {
            fprintf(stderr, "ERROR: (unix) connect error: %s\n", strerror(errno));
            return DLT_RETURN_ERROR;
        }

        if (client->sock < 0) {
            fprintf(stderr, "ERROR: Failed to open device %s\n",
                    client->socketPath);
            return DLT_RETURN_ERROR;
        }

        break;
    case DLT_CLIENT_MODE_UDP_MULTICAST:

        if ((client->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            fprintf(stderr, "ERROR: socket error: %s\n", strerror(errno));
            return DLT_RETURN_ERROR;
        }

        /* allow multiple sockets to use the same PORT number */
        if (setsockopt(client->sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
        {
            fprintf(stderr, "ERROR: Reusing address failed: %s\n", strerror(errno));
            return DLT_RETURN_ERROR;
        }

        memset(&client->receiver.addr, 0, sizeof(client->receiver.addr));
        client->receiver.addr.sin_family = AF_INET;
        client->receiver.addr.sin_addr.s_addr = htonl(INADDR_ANY);
        client->receiver.addr.sin_port = htons(client->port);

        /* bind to receive address */
        if (bind(client->sock, (struct sockaddr*) &client->receiver.addr, sizeof(client->receiver.addr)) < 0)
        {
            fprintf(stderr, "ERROR: bind failed: %s\n", strerror(errno));
            return DLT_RETURN_ERROR;
        }

        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        if (client->hostip)
        {
            mreq.imr_interface.s_addr = inet_addr(client->hostip);
        }
        if (client->servIP == NULL)
        {
            fprintf(stderr, "ERROR: server address not set\n");
            return DLT_RETURN_ERROR;
        }

        mreq.imr_multiaddr.s_addr = inet_addr(client->servIP);
        if (mreq.imr_multiaddr.s_addr == (in_addr_t)-1)
        {
            fprintf(stderr, "ERROR: server address not not valid %s\n", client->servIP);
            return DLT_RETURN_ERROR;
        }

        if (setsockopt(client->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) < 0)
        {
            fprintf(stderr, "ERROR: setsockopt add membership failed: %s\n", strerror(errno));
            return DLT_RETURN_ERROR;
        }

        break;
    default:

        if (verbose)
            fprintf(stderr, "ERROR: Mode not supported: %d\n", client->mode);

        return DLT_RETURN_ERROR;
    }

    if (dlt_receiver_init(&(client->receiver), client->sock, DLT_RECEIVE_BUFSIZE) != DLT_RETURN_OK) {
        fprintf(stderr, "ERROR initializing receiver\n");
        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_client_cleanup(DltClient *client, int verbose)
{
    int ret = DLT_RETURN_OK;

    if (verbose)
        printf("Cleanup dlt client\n");

    if (client == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (client->sock != -1)
        close(client->sock);

    if (dlt_receiver_free(&(client->receiver)) != DLT_RETURN_OK) {
        dlt_vlog(LOG_WARNING, "Failed to free receiver\n");
        ret = DLT_RETURN_ERROR;
    }

    if (client->serialDevice) {
        free(client->serialDevice);
        client->serialDevice = NULL;
    }

    if (client->servIP) {
        free(client->servIP);
        client->servIP = NULL;
    }

    if (client->socketPath) {
        free(client->socketPath);
        client->socketPath = NULL;
    }

    if (client->hostip) {
        free(client->hostip);
        client->hostip = NULL;
    }
    return ret;
}

DltReturnValue dlt_client_main_loop(DltClient *client, void *data, int verbose)
{
    DltMessage msg;
    int ret;

    if (client == 0)
        return DLT_RETURN_ERROR;

    if (dlt_message_init(&msg, verbose) == DLT_RETURN_ERROR)
        return DLT_RETURN_ERROR;

    while (1) {
        /* wait for data from socket or serial connection */
        ret = dlt_receiver_receive(&(client->receiver), client->mode);

        if (ret <= 0) {
            /* No more data to be received */
            if (dlt_message_free(&msg, verbose) == DLT_RETURN_ERROR)
                return DLT_RETURN_ERROR;

            return DLT_RETURN_TRUE;
        }

        while (dlt_message_read(&msg, (unsigned char *)(client->receiver.buf), client->receiver.bytesRcvd, 0,
                                verbose) == DLT_MESSAGE_ERROR_OK) {
            /* Call callback function */
            if (message_callback_function)
                (*message_callback_function)(&msg, data);

            if (msg.found_serialheader) {
                if (dlt_receiver_remove(&(client->receiver),
                                        msg.headersize + msg.datasize - sizeof(DltStorageHeader) +
                                        sizeof(dltSerialHeader)) ==
                    DLT_RETURN_ERROR) {
                    /* Return value ignored */
                    dlt_message_free(&msg, verbose);
                    return DLT_RETURN_ERROR;
                }
            }
            else if (dlt_receiver_remove(&(client->receiver),
                                         msg.headersize + msg.datasize - sizeof(DltStorageHeader)) ==
                     DLT_RETURN_ERROR) {
                /* Return value ignored */
                dlt_message_free(&msg, verbose);
                return DLT_RETURN_ERROR;
            }
        }

        if (dlt_receiver_move_to_begin(&(client->receiver)) == DLT_RETURN_ERROR) {
            /* Return value ignored */
            dlt_message_free(&msg, verbose);
            return DLT_RETURN_ERROR;
        }
    }

    if (dlt_message_free(&msg, verbose) == DLT_RETURN_ERROR)
        return DLT_RETURN_ERROR;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_client_send_ctrl_msg(DltClient *client, char *apid, char *ctid, uint8_t *payload, uint32_t size)
{
    DltMessage msg;
    int ret;

    int32_t len;
    uint32_t id_tmp;
    uint32_t id;

    if ((client == 0) || (client->sock < 0) || (apid == 0) || (ctid == 0))
        return DLT_RETURN_ERROR;

    /* initialise new message */
    if (dlt_message_init(&msg, 0) == DLT_RETURN_ERROR)
        return DLT_RETURN_ERROR;

    /* prepare payload of data */
    msg.datasize = size;

    if (msg.databuffer && (msg.databuffersize < msg.datasize)) {
        free(msg.databuffer);
        msg.databuffer = 0;
    }

    if (msg.databuffer == 0) {
        msg.databuffer = (uint8_t *)malloc(msg.datasize);
        msg.databuffersize = msg.datasize;
    }

    if (msg.databuffer == 0) {
        dlt_message_free(&msg, 0);
        return DLT_RETURN_ERROR;
    }

    /* copy data */
    memcpy(msg.databuffer, payload, size);

    /* prepare storage header */
    msg.storageheader = (DltStorageHeader *)msg.headerbuffer;

    if (dlt_set_storageheader(msg.storageheader, "") == DLT_RETURN_ERROR) {
        dlt_message_free(&msg, 0);
        return DLT_RETURN_ERROR;
    }

    /* prepare standard header */
    msg.standardheader = (DltStandardHeader *)(msg.headerbuffer + sizeof(DltStorageHeader));
    msg.standardheader->htyp = DLT_HTYP_WEID | DLT_HTYP_WTMS | DLT_HTYP_UEH | DLT_HTYP_PROTOCOL_VERSION1;

    #if (BYTE_ORDER == BIG_ENDIAN)
    msg.standardheader->htyp = (msg.standardheader->htyp | DLT_HTYP_MSBF);
    #endif

    msg.standardheader->mcnt = 0;

    /* Set header extra parameters */
    dlt_set_id(msg.headerextra.ecu, client->ecuid);
    /*msg.headerextra.seid = 0; */
    msg.headerextra.tmsp = dlt_uptime();

    /* Copy header extra parameters to headerbuffer */
    if (dlt_message_set_extraparameters(&msg, 0) == DLT_RETURN_ERROR) {
        dlt_message_free(&msg, 0);
        return DLT_RETURN_ERROR;
    }

    /* prepare extended header */
    msg.extendedheader = (DltExtendedHeader *)(msg.headerbuffer +
                                               sizeof(DltStorageHeader) +
                                               sizeof(DltStandardHeader) +
                                               DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp));

    msg.extendedheader->msin = DLT_MSIN_CONTROL_REQUEST;

    msg.extendedheader->noar = 1; /* number of arguments */

    dlt_set_id(msg.extendedheader->apid, (apid[0] == '\0') ? DLT_CLIENT_DUMMY_APP_ID : apid);
    dlt_set_id(msg.extendedheader->ctid, (ctid[0] == '\0') ? DLT_CLIENT_DUMMY_CON_ID : ctid);

    /* prepare length information */
    msg.headersize = sizeof(DltStorageHeader) +
        sizeof(DltStandardHeader) +
        sizeof(DltExtendedHeader) +
        DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp);

    len = msg.headersize - sizeof(DltStorageHeader) + msg.datasize;

    if (len > UINT16_MAX) {
        fprintf(stderr, "Critical: Huge injection message discarded!\n");
        dlt_message_free(&msg, 0);

        return DLT_RETURN_ERROR;
    }

    msg.standardheader->len = DLT_HTOBE_16(len);

    /* Send data (without storage header) */
    if ((client->mode == DLT_CLIENT_MODE_TCP) || (client->mode == DLT_CLIENT_MODE_SERIAL)) {
        /* via FileDescriptor */
        ret =
            write(client->sock, msg.headerbuffer + sizeof(DltStorageHeader), msg.headersize - sizeof(DltStorageHeader));

        if (0 > ret) {
            dlt_log(LOG_ERR, "Sending message failed\n");
            dlt_message_free(&msg, 0);
            return DLT_RETURN_ERROR;
        }

        ret = write(client->sock, msg.databuffer, msg.datasize);

        if (0 > ret) {
            dlt_log(LOG_ERR, "Sending message failed\n");
            dlt_message_free(&msg, 0);
            return DLT_RETURN_ERROR;
        }

        id_tmp = *((uint32_t *)(msg.databuffer));
        id = DLT_ENDIAN_GET_32(msg.standardheader->htyp, id_tmp);

        dlt_vlog(LOG_INFO,
                 "Control message forwarded : %s\n",
                 dlt_get_service_name(id));
    }
    else {
        /* via Socket */
        send(client->sock, (const char *)(msg.headerbuffer + sizeof(DltStorageHeader)),
             msg.headersize - sizeof(DltStorageHeader), 0);
        send(client->sock, (const char *)msg.databuffer, msg.datasize, 0);
    }

    /* free message */
    if (dlt_message_free(&msg, 0) == DLT_RETURN_ERROR)
        return DLT_RETURN_ERROR;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_client_send_inject_msg(DltClient *client,
                                          char *apid,
                                          char *ctid,
                                          uint32_t serviceID,
                                          uint8_t *buffer,
                                          uint32_t size)
{
    uint8_t *payload;
    int offset;

    payload = (uint8_t *)malloc(sizeof(uint32_t) + sizeof(uint32_t) + size);

    if (payload == 0)
        return DLT_RETURN_ERROR;

    offset = 0;
    memcpy(payload, &serviceID, sizeof(serviceID));
    offset += sizeof(uint32_t);
    memcpy(payload + offset, &size, sizeof(size));
    offset += sizeof(uint32_t);
    memcpy(payload + offset, buffer, size);

    /* free message */
    if (dlt_client_send_ctrl_msg(client, apid, ctid, payload,
                                 sizeof(uint32_t) + sizeof(uint32_t) + size) == DLT_RETURN_ERROR) {
        free(payload);
        return DLT_RETURN_ERROR;
    }

    free(payload);

    return DLT_RETURN_OK;

}

DltReturnValue dlt_client_send_log_level(DltClient *client, char *apid, char *ctid, uint8_t logLevel)
{
    DltServiceSetLogLevel *req;
    int ret = DLT_RETURN_ERROR;

    if (client == NULL)
        return ret;

    req = (DltServiceSetLogLevel *)malloc(sizeof(DltServiceSetLogLevel));

    if (req == NULL)
        return ret;

    memset(req, 0, sizeof(DltServiceSetLogLevel));
    req->service_id = DLT_SERVICE_ID_SET_LOG_LEVEL;
    dlt_set_id(req->apid, apid);
    dlt_set_id(req->ctid, ctid);
    req->log_level = logLevel;
    dlt_set_id(req->com, "remo");

    /* free message */
    ret = dlt_client_send_ctrl_msg(client,
                                   "APP",
                                   "CON",
                                   (uint8_t *)req,
                                   sizeof(DltServiceSetLogLevel));


    free(req);

    return ret;
}

DltReturnValue dlt_client_get_log_info(DltClient *client)
{
    DltServiceGetLogInfoRequest *req;
    int ret = DLT_RETURN_ERROR;

    if (client == NULL)
        return ret;

    req = (DltServiceGetLogInfoRequest *)malloc(sizeof(DltServiceGetLogInfoRequest));

    if (req == NULL)
        return ret;

    req->service_id = DLT_SERVICE_ID_GET_LOG_INFO;
    req->options = 7;
    dlt_set_id(req->apid, "");
    dlt_set_id(req->ctid, "");
    dlt_set_id(req->com, "remo");

    /* send control message to daemon*/
    ret = dlt_client_send_ctrl_msg(client,
                                   "",
                                   "",
                                   (uint8_t *)req,
                                   sizeof(DltServiceGetLogInfoRequest));

    free(req);

    return ret;
}

DltReturnValue dlt_client_get_default_log_level(DltClient *client)
{
    DltServiceGetDefaultLogLevelRequest *req;
    int ret = DLT_RETURN_ERROR;

    if (client == NULL)
        return ret;

    req = (DltServiceGetDefaultLogLevelRequest *)
        malloc(sizeof(DltServiceGetDefaultLogLevelRequest));

    if (req == NULL)
        return ret;

    req->service_id = DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL;

    /* send control message to daemon*/
    ret = dlt_client_send_ctrl_msg(client,
                                   "",
                                   "",
                                   (uint8_t *)req,
                                   sizeof(DltServiceGetDefaultLogLevelRequest));

    free(req);

    return ret;
}

DltReturnValue dlt_client_get_software_version(DltClient *client)
{
    DltServiceGetSoftwareVersion *req;
    int ret = DLT_RETURN_ERROR;

    if (client == NULL)
        return ret;

    req = (DltServiceGetSoftwareVersion *)malloc(sizeof(DltServiceGetSoftwareVersion));

    req->service_id = DLT_SERVICE_ID_GET_SOFTWARE_VERSION;

    /* send control message to daemon*/
    ret = dlt_client_send_ctrl_msg(client,
                                   "",
                                   "",
                                   (uint8_t *)req,
                                   sizeof(DltServiceGetSoftwareVersion));

    free(req);

    return ret;
}

DltReturnValue dlt_client_send_trace_status(DltClient *client, char *apid, char *ctid, uint8_t traceStatus)
{
    DltServiceSetLogLevel *req;
    uint8_t *payload;

    payload = (uint8_t *)malloc(sizeof(DltServiceSetLogLevel));

    if (payload == 0)
        return DLT_RETURN_ERROR;

    req = (DltServiceSetLogLevel *)payload;
    memset(req, 0, sizeof(DltServiceSetLogLevel));
    req->service_id = DLT_SERVICE_ID_SET_TRACE_STATUS;
    dlt_set_id(req->apid, apid);
    dlt_set_id(req->ctid, ctid);
    req->log_level = traceStatus;
    dlt_set_id(req->com, "remo");

    /* free message */
    if (dlt_client_send_ctrl_msg(client, "APP", "CON", payload, sizeof(DltServiceSetLogLevel)) == DLT_RETURN_ERROR) {
        free(payload);
        return DLT_RETURN_ERROR;
    }

    free(payload);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_client_send_default_log_level(DltClient *client, uint8_t defaultLogLevel)
{
    DltServiceSetDefaultLogLevel *req;
    uint8_t *payload;

    payload = (uint8_t *)malloc(sizeof(DltServiceSetDefaultLogLevel));

    if (payload == 0)
        return DLT_RETURN_ERROR;

    req = (DltServiceSetDefaultLogLevel *)payload;

    req->service_id = DLT_SERVICE_ID_SET_DEFAULT_LOG_LEVEL;
    req->log_level = defaultLogLevel;
    dlt_set_id(req->com, "remo");

    /* free message */
    if (dlt_client_send_ctrl_msg(client, "APP", "CON", payload,
                                 sizeof(DltServiceSetDefaultLogLevel)) == DLT_RETURN_ERROR) {
        free(payload);
        return DLT_RETURN_ERROR;
    }

    free(payload);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_client_send_all_log_level(DltClient *client, uint8_t LogLevel)
{
    DltServiceSetDefaultLogLevel *req;
    uint8_t *payload;

    payload = (uint8_t *)malloc(sizeof(DltServiceSetDefaultLogLevel));

    if (payload == 0)
        return DLT_RETURN_ERROR;

    req = (DltServiceSetDefaultLogLevel *)payload;

    req->service_id = DLT_SERVICE_ID_SET_ALL_LOG_LEVEL;
    req->log_level = LogLevel;
    dlt_set_id(req->com, "remo");

    /* free message */
    if (dlt_client_send_ctrl_msg(client, "APP", "CON", payload, sizeof(DltServiceSetDefaultLogLevel)) == -1) {
        free(payload);
        return DLT_RETURN_ERROR;
    }

    free(payload);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_client_send_default_trace_status(DltClient *client, uint8_t defaultTraceStatus)
{
    DltServiceSetDefaultLogLevel *req;
    uint8_t *payload;

    payload = (uint8_t *)malloc(sizeof(DltServiceSetDefaultLogLevel));

    if (payload == 0)
        return DLT_RETURN_ERROR;

    req = (DltServiceSetDefaultLogLevel *)payload;

    req->service_id = DLT_SERVICE_ID_SET_DEFAULT_TRACE_STATUS;
    req->log_level = defaultTraceStatus;
    dlt_set_id(req->com, "remo");

    /* free message */
    if (dlt_client_send_ctrl_msg(client, "APP", "CON", payload,
                                 sizeof(DltServiceSetDefaultLogLevel)) == DLT_RETURN_ERROR) {
        free(payload);
        return DLT_RETURN_ERROR;
    }

    free(payload);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_client_send_all_trace_status(DltClient *client, uint8_t traceStatus)
{
    DltServiceSetDefaultLogLevel *req;
    uint8_t *payload;

    if (client == NULL) {
        dlt_vlog(LOG_ERR, "%s: Invalid parameters\n", __func__);
        return DLT_RETURN_ERROR;
    }

    payload = (uint8_t *)malloc(sizeof(DltServiceSetDefaultLogLevel));

    if (payload == 0) {
        dlt_vlog(LOG_ERR, "%s: Could not allocate memory %zu\n", __func__, sizeof(DltServiceSetDefaultLogLevel));
        return DLT_RETURN_ERROR;
    }

    req = (DltServiceSetDefaultLogLevel *)payload;

    req->service_id = DLT_SERVICE_ID_SET_ALL_TRACE_STATUS;
    req->log_level = traceStatus;
    dlt_set_id(req->com, "remo");

    /* free message */
    if (dlt_client_send_ctrl_msg(client, "APP", "CON", payload, sizeof(DltServiceSetDefaultLogLevel)) == -1) {
        free(payload);
        return DLT_RETURN_ERROR;
    }

    free(payload);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_client_send_timing_pakets(DltClient *client, uint8_t timingPakets)
{
    DltServiceSetVerboseMode *req;
    uint8_t *payload;

    payload = (uint8_t *)malloc(sizeof(DltServiceSetVerboseMode));

    if (payload == 0)
        return DLT_RETURN_ERROR;

    req = (DltServiceSetVerboseMode *)payload;

    req->service_id = DLT_SERVICE_ID_SET_TIMING_PACKETS;
    req->new_status = timingPakets;

    /* free message */
    if (dlt_client_send_ctrl_msg(client, "APP", "CON", payload, sizeof(DltServiceSetVerboseMode)) == DLT_RETURN_ERROR) {
        free(payload);
        return DLT_RETURN_ERROR;
    }

    free(payload);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_client_send_store_config(DltClient *client)
{
    uint32_t service_id;

    service_id = DLT_SERVICE_ID_STORE_CONFIG;

    /* free message */
    if (dlt_client_send_ctrl_msg(client, "APP", "CON", (uint8_t *)&service_id, sizeof(uint32_t)) == DLT_RETURN_ERROR)
        return DLT_RETURN_ERROR;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_client_send_reset_to_factory_default(DltClient *client)
{
    uint32_t service_id;

    service_id = DLT_SERVICE_ID_RESET_TO_FACTORY_DEFAULT;

    /* free message */
    if (dlt_client_send_ctrl_msg(client, "APP", "CON", (uint8_t *)&service_id, sizeof(uint32_t)) == DLT_RETURN_ERROR)
        return DLT_RETURN_ERROR;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_client_setbaudrate(DltClient *client, int baudrate)
{
    if (client == 0)
        return DLT_RETURN_ERROR;

    client->baudrate = dlt_convert_serial_speed(baudrate);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_client_set_mode(DltClient *client, DltClientMode mode)
{
    if (client == 0)
        return DLT_RETURN_ERROR;

    client->mode = mode;
    return DLT_RETURN_OK;

}

int dlt_client_set_server_ip(DltClient *client, char *ipaddr)
{
    client->servIP = strdup(ipaddr);

    if (client->servIP == NULL) {
        dlt_log(LOG_ERR, "ERROR: failed to duplicate server IP\n");
        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

int dlt_client_set_host_if_address(DltClient *client, char *hostip)
{
    client->hostip = strdup(hostip);

    if (client->hostip == NULL) {
        dlt_log(LOG_ERR, "ERROR: failed to duplicate UDP interface address\n");
        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

int dlt_client_set_serial_device(DltClient *client, char *serial_device)
{
    client->serialDevice = strdup(serial_device);

    if (client->serialDevice == NULL) {
        dlt_log(LOG_ERR, "ERROR: failed to duplicate serial device\n");
        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

int dlt_client_set_socket_path(DltClient *client, char *socket_path)
{
    client->socketPath = strdup(socket_path);

    if (client->socketPath == NULL) {
        dlt_log(LOG_ERR, "ERROR: failed to duplicate socket path\n");
        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}
/**
 * free allocation when calloc failed
 *
 * @param resp          DltServiceGetLogInfoResponse
 * @param count_app_ids number of app_ids which needs to be freed
 */
DLT_STATIC void dlt_client_free_calloc_failed_get_log_info(DltServiceGetLogInfoResponse *resp,
                                                           int count_app_ids)
{
    AppIDsType *app = NULL;
    ContextIDsInfoType *con = NULL;
    int i = 0;
    int j = 0;

    for (i = 0; i < count_app_ids; i++) {
        app = &(resp->log_info_type.app_ids[i]);

        for (j = 0; j < app->count_context_ids; j++) {
            con = &(app->context_id_info[j]);

            free(con->context_description);
            con->context_description = NULL;
        }

        free(app->app_description);
        app->app_description = NULL;

        free(app->context_id_info);
        app->context_id_info = NULL;
    }

    free(resp->log_info_type.app_ids);
    resp->log_info_type.app_ids = NULL;

    return;
}

DltReturnValue dlt_client_parse_get_log_info_resp_text(DltServiceGetLogInfoResponse *resp,
                                                       char *resp_text)
{
    AppIDsType *app = NULL;
    ContextIDsInfoType *con = NULL;
    int i = 0;
    int j = 0;
    char *rp = NULL;
    int rp_count = 0;

    if ((resp == NULL) || (resp_text == NULL))
        return DLT_RETURN_WRONG_PARAMETER;

    /* ------------------------------------------------------
    *  get_log_info data structure(all data is ascii)
    *
    *  get_log_info, aa, bb bb cc cc cc cc dd dd ee ee ee ee ff gg hh hh ii ii ii .. ..
    *                ~~  ~~~~~ ~~~~~~~~~~~ ~~~~~ ~~~~~~~~~~~~~~
    *                          cc cc cc cc dd dd ee ee ee ee ff gg hh hh ii ii ii .. ..
    *                    jj jj kk kk kk .. ..
    *                          ~~~~~~~~~~~ ~~~~~ ~~~~~~~~~~~~~~
    *  aa         : get mode (fix value at 0x07)
    *  bb bb      : list num of apid (little endian)
    *  cc cc cc cc: apid
    *  dd dd      : list num of ctid (little endian)
    *  ee ee ee ee: ctid
    *  ff         : log level
    *  gg         : trace status
    *  hh hh      : description length of ctid
    *  ii ii ..   : description text of ctid
    *  jj jj      : description length of apid
    *  kk kk ..   : description text of apid
    *  ------------------------------------------------------ */

    rp = resp_text + DLT_GET_LOG_INFO_HEADER;
    rp_count = 0;

    /* check if status is acceptable */
    if ((resp->status < GET_LOG_INFO_STATUS_MIN) ||
        (resp->status > GET_LOG_INFO_STATUS_MAX)) {
        if (resp->status == GET_LOG_INFO_STATUS_NO_MATCHING_CTX)
            dlt_vlog(LOG_WARNING,
                     "The status(%d) is invalid: NO matching Context IDs\n",
                     resp->status);
        else if (resp->status == GET_LOG_INFO_STATUS_RESP_DATA_OVERFLOW)
            dlt_vlog(LOG_WARNING,
                     "The status(%d) is invalid: Response data over flow\n",
                     resp->status);
        else
            dlt_vlog(LOG_WARNING,
                     "The status(%d) is invalid\n",
                     resp->status);

        return DLT_RETURN_ERROR;
    }

    /* count_app_ids */
    resp->log_info_type.count_app_ids = dlt_getloginfo_conv_ascii_to_uint16_t(rp,
                                                                              &rp_count);

    resp->log_info_type.app_ids = (AppIDsType *)calloc
            (resp->log_info_type.count_app_ids, sizeof(AppIDsType));

    if (resp->log_info_type.app_ids == NULL) {
        dlt_vlog(LOG_ERR, "calloc failed for app_ids\n");
        dlt_client_free_calloc_failed_get_log_info(resp, 0);
        return DLT_RETURN_ERROR;
    }

    for (i = 0; i < resp->log_info_type.count_app_ids; i++) {
        app = &(resp->log_info_type.app_ids[i]);
        /* get app id */
        dlt_getloginfo_conv_ascii_to_id(rp, &rp_count, app->app_id, DLT_ID_SIZE);

        /* count_con_ids */
        app->count_context_ids = dlt_getloginfo_conv_ascii_to_uint16_t(rp,
                                                                       &rp_count);

        app->context_id_info = (ContextIDsInfoType *)calloc
                (app->count_context_ids, sizeof(ContextIDsInfoType));

        if (app->context_id_info == NULL) {
            dlt_vlog(LOG_ERR,
                     "calloc failed for context_id_info\n");
            dlt_client_free_calloc_failed_get_log_info(resp, i);
            return DLT_RETURN_ERROR;
        }

        for (j = 0; j < app->count_context_ids; j++) {
            con = &(app->context_id_info[j]);
            /* get con id */
            dlt_getloginfo_conv_ascii_to_id(rp,
                                            &rp_count,
                                            con->context_id,
                                            DLT_ID_SIZE);

            /* log_level */
            if ((resp->status == 4) || (resp->status == 6) || (resp->status == 7))
                con->log_level = dlt_getloginfo_conv_ascii_to_int16_t(rp,
                                                                      &rp_count);

            /* trace status */
            if ((resp->status == 5) || (resp->status == 6) || (resp->status == 7))
                con->trace_status = dlt_getloginfo_conv_ascii_to_int16_t(rp,
                                                                         &rp_count);

            /* context desc */
            if (resp->status == 7) {
                con->len_context_description = dlt_getloginfo_conv_ascii_to_uint16_t(rp,
                                                                                     &rp_count);
                con->context_description = (char *)calloc
                        (con->len_context_description + 1, sizeof(char));

                if (con->context_description == 0) {
                    dlt_log(LOG_ERR, "calloc failed for context description\n");
                    dlt_client_free_calloc_failed_get_log_info(resp, i);
                    return DLT_RETURN_ERROR;
                }

                dlt_getloginfo_conv_ascii_to_id(rp,
                                                &rp_count,
                                                con->context_description,
                                                con->len_context_description);
            }
        }

        /* application desc */
        if (resp->status == 7) {
            app->len_app_description = dlt_getloginfo_conv_ascii_to_uint16_t(rp,
                                                                             &rp_count);
            app->app_description = (char *)calloc
                    (app->len_app_description + 1, sizeof(char));

            if (app->app_description == 0) {
                dlt_log(LOG_ERR, "calloc failed for application description\n");
                dlt_client_free_calloc_failed_get_log_info(resp, i);
                return DLT_RETURN_ERROR;
            }

            dlt_getloginfo_conv_ascii_to_id(rp,
                                            &rp_count,
                                            app->app_description,
                                            app->len_app_description);
        }
    }

    return DLT_RETURN_OK;
}

int dlt_client_cleanup_get_log_info(DltServiceGetLogInfoResponse *resp)
{
    AppIDsType app;
    int i = 0;
    int j = 0;

    if (resp == NULL)
        return DLT_RETURN_OK;

    for (i = 0; i < resp->log_info_type.count_app_ids; i++) {
        app = resp->log_info_type.app_ids[i];

        for (j = 0; j < app.count_context_ids; j++) {
            free(app.context_id_info[j].context_description);
            app.context_id_info[j].context_description = NULL;
        }

        free(app.context_id_info);
        app.context_id_info = NULL;
        free(app.app_description);
        app.app_description = NULL;
    }

    free(resp->log_info_type.app_ids);
    resp->log_info_type.app_ids = NULL;

    free(resp);
    resp = NULL;

    return DLT_RETURN_OK;
}
