/*
 * Copyright (c) 2019 LG Electronics Inc.
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of COVESA Project DLT - Diagnostic Log and Trace.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.covesa.org/.
 */

/*!
 * \author
 * Guruprasad KN <guruprasad.kn@lge.com>
 * Sachin Sudhakar Shetty <sachin.shetty@lge.com>
 * Sunil Kovila Sampath <sunil.s@lge.com>
 *
 * \copyright Copyright (c) 2019 LG Electronics Inc.
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_daemon_udp_socket.c
 */

#include "dlt_daemon_udp_common_socket.h"

static void dlt_daemon_udp_clientmsg_send(DltDaemonClientSockInfo *clientinfo,
                                          void *data1, int size1, void *data2, int size2, int verbose);
static int g_udp_sock_fd = -1;
static DltDaemonClientSockInfo g_udpmulticast_addr;

/* ************************************************************************** */
/* Function   : dlt_daemon_udp_init_clientstruct */
/* In Param   : UDP client_info struct to be initilzed */
/* Out Param  : NIL */
/* Description: client struct to be initilized to copy control/connect/disconnect */
/*                client addr */
/* ************************************************************************** */
void dlt_daemon_udp_init_clientstruct(DltDaemonClientSockInfo *clientinfo_struct)
{
    if (clientinfo_struct == NULL) {
        dlt_vlog(LOG_ERR, "%s: NULL arg\n", __func__);
        return;
    }

    memset(&clientinfo_struct->clientaddr, 0x00, sizeof(clientinfo_struct->clientaddr));
    clientinfo_struct->clientaddr_size = sizeof(clientinfo_struct->clientaddr);
    clientinfo_struct->isvalidflag = ADDRESS_INVALID; /* info is invalid */
    dlt_vlog(LOG_DEBUG, "%s: client addr struct init success \n", __func__);
}

/* ************************************************************************** */
/* Function   : dlt_daemon_udp_setmulticast_addr */
/* In Param   : NIL */
/* Out Param  : NIL */
/* Description: set the multicast addr to global variables */
/* ************************************************************************** */
void dlt_daemon_udp_setmulticast_addr(DltDaemonLocal *daemon_local)
{
    if (daemon_local == NULL) {
        dlt_vlog(LOG_ERR, "%s: NULL arg\n", __func__);
        return;
    }

    dlt_daemon_udp_init_clientstruct(&g_udpmulticast_addr);

    struct sockaddr_in clientaddr;
    clientaddr.sin_family = AF_INET;
    inet_pton(AF_INET, daemon_local->UDPMulticastIPAddress, &clientaddr.sin_addr);
    clientaddr.sin_port = htons(daemon_local->UDPMulticastIPPort);
    memcpy(&g_udpmulticast_addr.clientaddr, &clientaddr, sizeof(struct sockaddr_in));
    g_udpmulticast_addr.clientaddr_size = sizeof(g_udpmulticast_addr.clientaddr);
    g_udpmulticast_addr.isvalidflag = ADDRESS_VALID;
}

/* ************************************************************************** */
/* Function   : dlt_daemon_udp_connection_setup */
/* In Param   : contains daemon param values used globally */
/* Out Param  : status of udp connection setup and fd registration */
/* Description: DataGram socket fd connection is setup */
/* ************************************************************************** */
DltReturnValue dlt_daemon_udp_connection_setup(DltDaemonLocal *daemon_local)
{
    int fd = DLT_FD_INIT;
    DltReturnValue ret_val = DLT_RETURN_WRONG_PARAMETER;

    if (daemon_local == NULL)
        return ret_val;

    if ((ret_val = dlt_daemon_udp_socket_open(&fd, daemon_local->flags.port)) != DLT_RETURN_OK) {
        dlt_log(LOG_ERR, "Could not initialize udp socket.\n");
    }
    else {
        /* assign to global udp fd */
        g_udp_sock_fd = fd;
        /* set global multicast addr */
        dlt_daemon_udp_setmulticast_addr(daemon_local);
        dlt_log(LOG_DEBUG, "initialize udp socket success\n");
    }

    return ret_val;
}

/* ************************************************************************** */
/* Function   : dlt_daemon_udp_socket_open */
/* In Param   : contains udp port number */
/* Out Param  : status of udp connection setup */
/* Description: This funtion is used to setup DGRAM connection */
/*              does socket()->bind() on udp port */
/* ************************************************************************** */
DltReturnValue dlt_daemon_udp_socket_open(int *sock, unsigned int servPort)
{
    int enable_reuse_addr = 1;
    int sockbuffer = DLT_DAEMON_RCVBUFSIZESOCK;
    char portnumbuffer[SOCKPORT_MAX_LEN] = { 0 };
    struct addrinfo hints;
    struct addrinfo *servinfo = NULL;
    struct addrinfo *addrinfo_iterator = NULL;
    int getaddrinfo_errorcode = -1;

    if (sock == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    memset(&hints, 0, sizeof hints);
#ifdef DLT_USE_IPv6
    hints.ai_family = AF_INET6; /* force IPv6 - will still work with IPv4 */
#else
    hints.ai_family = AF_INET;
#endif
    hints.ai_socktype = SOCK_DGRAM;/* UDP Connection */
    hints.ai_flags = AI_PASSIVE; /* use my IP address */

    snprintf(portnumbuffer, SOCKPORT_MAX_LEN, "%d", servPort);

    if ((getaddrinfo_errorcode = getaddrinfo(NULL, portnumbuffer, &hints, &servinfo)) != 0) {
        dlt_vlog(LOG_WARNING, "[%s:%d] getaddrinfo: %s\n", __func__, __LINE__,
                 gai_strerror(getaddrinfo_errorcode));
        return DLT_RETURN_ERROR;
    }

    for (addrinfo_iterator = servinfo; addrinfo_iterator != NULL; addrinfo_iterator = addrinfo_iterator->ai_next) {
        if ((*sock = socket(addrinfo_iterator->ai_family, addrinfo_iterator->ai_socktype,
                            addrinfo_iterator->ai_protocol)) == SYSTEM_CALL_ERROR) {
            dlt_log(LOG_WARNING, "socket() error\n");
            continue;
        }

        dlt_vlog(LOG_INFO,
                 "[%s:%d] Socket created - socket_family:%i socket_type:%i, protocol:%i\n",
                 __func__, __LINE__, addrinfo_iterator->ai_family,
                 addrinfo_iterator->ai_socktype, addrinfo_iterator->ai_protocol);

        if (setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, &enable_reuse_addr, sizeof(enable_reuse_addr))
            == SYSTEM_CALL_ERROR) {
            dlt_vlog(LOG_WARNING, "[%s:%d] Setsockopt error %s\n", __func__, __LINE__,
                     strerror(errno));
            close(*sock);
            continue;
        }

        if (setsockopt(*sock, SOL_SOCKET, SO_RCVBUF, &sockbuffer, sizeof(sockbuffer))
            == SYSTEM_CALL_ERROR) {
            dlt_vlog(LOG_WARNING, "[%s:%d] Setsockopt error %s\n", __func__, __LINE__,
                     strerror(errno));
            close(*sock);
            continue;
        }

        if (bind(*sock, addrinfo_iterator->ai_addr, addrinfo_iterator->ai_addrlen)
            == SYSTEM_CALL_ERROR) {
            close(*sock);
            dlt_log(LOG_WARNING, "bind() error\n");
            continue;
        }

        break;
    }

    if (addrinfo_iterator == NULL) {
        dlt_log(LOG_WARNING, "failed to bind socket\n");
        return DLT_RETURN_ERROR;
    }

    freeaddrinfo(servinfo);

    return DLT_RETURN_OK; /* OK */
}

/* ************************************************************************** */
/* Function   : dlt_daemon_udp_dltmsg_multicast */
/* In Param   : data bytes in dlt format */
/* Out Param  : NIL */
/* Description: multicast UDP dlt-message packets to dlt-client */
/* ************************************************************************** */
void dlt_daemon_udp_dltmsg_multicast(void *data1, int size1,
                                     void *data2, int size2,
                                     int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    /*
     * When UDP Buffer is implemented then data2 would be expected to be NULL
     * as the data comes from buffer directly. In that case data2 should
     * not be checked for NULL
     */
    if ((data1 == NULL) || (data2 == NULL)) {
        dlt_vlog(LOG_ERR, "%s: NULL arg\n", __func__);
        return;
    }

    dlt_daemon_udp_clientmsg_send(&g_udpmulticast_addr, data1, size1,
                                  data2, size2, verbose);
}

/* ************************************************************************** */
/* Function   : dlt_daemon_udp_clientmsg_send */
/* In Param   : data bytes & respective size in dlt format */
/* Out Param  : NIL */
/* Description: common interface to send data via UDP protocol */
/* ************************************************************************** */
void dlt_daemon_udp_clientmsg_send(DltDaemonClientSockInfo *clientinfo,
                                   void *data1, int size1, void *data2, int size2, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if ((clientinfo->isvalidflag == ADDRESS_VALID) &&
        (size1 > 0) && (size2 > 0)) {
        void *data = (void *)calloc(size1 + size2, sizeof(char));

        if (data == NULL) {
            dlt_vlog(LOG_ERR, "%s: calloc failure\n", __func__);
            return;
        }

        memcpy(data, data1, size1);
        memcpy(data + size1, data2, size2);

        if (sendto(g_udp_sock_fd, data, size1 + size2, 0, (struct sockaddr *)&clientinfo->clientaddr,
                   clientinfo->clientaddr_size) < 0)
            dlt_vlog(LOG_ERR, "%s: Send UDP Packet Data failed\n", __func__);

        free(data);
        data = NULL;

    }
    else {
        if (clientinfo->isvalidflag != ADDRESS_VALID)
            dlt_vlog(LOG_ERR, "%s: clientinfo->isvalidflag != ADDRESS_VALID %d\n", __func__, clientinfo->isvalidflag);

        if (size1 <= 0)
            dlt_vlog(LOG_ERR, "%s: size1 <= 0\n", __func__);

        if (size2 <= 0)
            dlt_vlog(LOG_ERR, "%s: size2 <= 0\n", __func__);
    }
}

/* ************************************************************************** */
/* Function   : dlt_daemon_udp_close_connection */
/* In Param   : NIL */
/* Out Param  : NIL */
/* Description: Closes UDP Connection */
/* ************************************************************************** */
void dlt_daemon_udp_close_connection(void)
{
    if (close(g_udp_sock_fd) == SYSTEM_CALL_ERROR)
        dlt_vlog(LOG_WARNING, "[%s:%d] close error %s\n", __func__, __LINE__,
                 strerror(errno));
}
