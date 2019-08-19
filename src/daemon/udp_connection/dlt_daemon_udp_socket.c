/*
Copyright (c) 2019 LG Electronics Inc.
SPDX-License-Identifier: MPL-2.0

This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
If a copy of the MPL was not distributed with this file,
You can obtain one at http://mozilla.org/MPL/2.0/.

For further information see http://www.genivi.org/.
*/

/*!
* \author
* Guruprasad KN <guruprasad.kn@lge.com>
* Sachin Sudhakar Shetty <sachin.shetty@lge.com>
* Sunil Kovila Sampath <sunil.s@lge.com>
*
* \Copyright (c) 2019 LG Electronics Inc.
* License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
*
* \file dlt_daemon_udp_socket.c
*/

#include "dlt_daemon_udp_common_socket.h"

static int g_udp_sock_fd = -1;
static DltDaemonClientSockInfo g_udpmulticast_addr;

// **************************************************************************
// Function   : dlt_daemon_udp_init_clientstruct
// In Param   : UDP client_info struct to be initilzed
// Out Param  : NIL
// Description: client struct to be initilized to copy control/connect/disconnect
//                client addr
// **************************************************************************
void dlt_daemon_udp_init_clientstruct(DltDaemonClientSockInfo *clientinfo_struct)
{
    if(clientinfo_struct != NULL){
        memset(&clientinfo_struct->clientaddr, 0x00, sizeof(clientinfo_struct->clientaddr));
        clientinfo_struct->clientaddr_size = sizeof(clientinfo_struct->clientaddr);
        clientinfo_struct->isvalidflag = ADDRESS_INVALID; // info is invalid
        dlt_log(LOG_INFO, "dlt_daemon_udp_init_clientstruct: client addr struct init success \n");
    }
    else{
        dlt_log(LOG_ERR, "dlt_daemon_udp_init_clientstruct: NULL arg\n");
    }
}


// **************************************************************************
// Function   : dlt_daemon_udp_setmulticast_addr
// In Param   : NIL
// Out Param  : NIL
// Description: set the multicast addr to global variables
// **************************************************************************
void dlt_daemon_udp_setmulticast_addr(DltDaemonLocal* daemon_local)
{
    dlt_daemon_udp_init_clientstruct(&g_udpmulticast_addr);
//#ifdef DLT_USE_IPv6 // IPv6 Support
/*    struct sockaddr_in6 clientaddr6;
    clientaddr6.sin6_family = AF_INET6;
    inet_pton(AF_INET, daemon_local->UDPMulticastIPAddress, &clientaddr6.sin6_addr.s6_addr);
    clientaddr6.sin6_port=htons(daemon_local->UDPMulticastIPPort);
    memcpy(&g_udpmulticast_addr.clientaddr, &clientaddr6, sizeof(struct sockaddr_in6));
*/
//#else // IPv4 support
    struct sockaddr_in clientaddr;
    clientaddr.sin_family = AF_INET;
    inet_pton(AF_INET, daemon_local->UDPMulticastIPAddress, &clientaddr.sin_addr);
    clientaddr.sin_port = htons(daemon_local->UDPMulticastIPPort);
    memcpy(&g_udpmulticast_addr.clientaddr, &clientaddr, sizeof(struct sockaddr_in));
//#endif
    g_udpmulticast_addr.clientaddr_size = sizeof(g_udpmulticast_addr.clientaddr);
    g_udpmulticast_addr.isvalidflag = ADDRESS_VALID;
}

// **************************************************************************
// Function   : dlt_daemon_udp_connection_setup
// In Param   : contains daemon param values used globally
// Out Param  : status of udp connection setup and fd registration
// Description: DataGram socket fd connection is setup
//                fd is registered with the epoll
// **************************************************************************
DltReturnValue dlt_daemon_udp_connection_setup(DltDaemonLocal* daemon_local)
{
    int fd = -1;
    DltReturnValue ret_val = DLT_RETURN_ERROR;

    if(daemon_local == NULL)
    {
        return ret_val;
    }

    if(DLT_RETURN_OK != dlt_daemon_udp_socket_open(&fd, daemon_local->flags.port)){
        dlt_log(LOG_ERR, "Could not initialize udp socket.\n");
    }
    else{
        // assign to global udp fd
        g_udp_sock_fd = fd;
        // set global multicast addr
        dlt_daemon_udp_setmulticast_addr(daemon_local);
        ret_val = DLT_RETURN_TRUE;
        dlt_log(LOG_INFO, " initialize udp socket success\n");
    }

    return ret_val;
}

// **************************************************************************
// Function   : dlt_daemon_udp_socket_open
// In Param   : contains udp port number
// Out Param  : status of udp connection setup
// Description: This funtion is used to setup DGRAM connection
//              does socket()->bind() on udp port
// **************************************************************************
DltReturnValue dlt_daemon_udp_socket_open(int *sock, unsigned int servPort)
{
    int enable_reuse_addr = 1;
    int sockbuffer = DLT_DAEMON_RCVBUFSIZESOCK;
    char portnumbuffer[SOCKPORT_MAX_LEN] = {0};
    struct addrinfo hints, *servinfo = NULL, *addrinfo_iterator = NULL;
    int getaddrinfo_errorcode = -1;

    if(sock == NULL)
    {
        return DLT_RETURN_ERROR;
    }

    memset(&hints, 0, sizeof hints);
#ifdef DLT_USE_IPv6
    hints.ai_family = AF_INET6; // force IPv6 - will still work with IPv4
#else
    hints.ai_family = AF_INET;
#endif
    hints.ai_socktype = SOCK_DGRAM;// UDP Connection
    hints.ai_flags = AI_PASSIVE; // use my IP address

    snprintf(portnumbuffer, SOCKPORT_MAX_LEN, "%d", servPort);
    if ((getaddrinfo_errorcode = getaddrinfo(NULL, portnumbuffer, &hints, &servinfo)) != 0) {
        dlt_vlog(LOG_WARNING, "[%s/%d] getaddrinfo: %s\n", __func__, __LINE__,
                gai_strerror(getaddrinfo_errorcode));
        return DLT_RETURN_ERROR;
    }

    for(addrinfo_iterator = servinfo; addrinfo_iterator != NULL; addrinfo_iterator = addrinfo_iterator->ai_next) {
        if ((*sock = socket(addrinfo_iterator->ai_family, addrinfo_iterator->ai_socktype,
                     addrinfo_iterator->ai_protocol)) == SYSTEM_CALL_ERROR) {
            dlt_log(LOG_WARNING, "socket() error\n");
            continue;
        }
        dlt_vlog(LOG_INFO,
                "[%s/%d] Socket created - socket_family:%i socket_type:%i, protocol:%i\n",
                __func__, __LINE__,addrinfo_iterator->ai_family,
                addrinfo_iterator->ai_socktype, addrinfo_iterator->ai_protocol);

        if (setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, &enable_reuse_addr, sizeof(int))
                       == SYSTEM_CALL_ERROR)
        {
            dlt_vlog(LOG_WARNING, "[%s/%d] Setsockopt error %s\n", __func__, __LINE__,
                    strerror(errno));
            continue;
        }

        if(setsockopt(*sock, SOL_SOCKET, SO_RCVBUF, &sockbuffer, DLT_DAEMON_RCVBUFSIZESOCK)
                      == SYSTEM_CALL_ERROR)
        {
            dlt_vlog(LOG_WARNING, "[%s/%d] Setsockopt error %s\n", __func__, __LINE__,
                    strerror(errno));
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

    return DLT_RETURN_OK; // OK
}

// **************************************************************************
// Function   : dlt_daemon_udp_dltmsg_multicast
// In Param   : data bytes in dlt format
// Out Param  : NIL
// Description: multicast UDP dlt-message packets to dlt-client
// **************************************************************************
void dlt_daemon_udp_dltmsg_multicast( void* data1, int size1,
                                                  void* data2, int size2,
                                                  int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);
    dlt_daemon_udp_clientmsg_send(&g_udpmulticast_addr, data1, size1,
                                  data2, size2, verbose);
}

// **************************************************************************
// Function   : dlt_daemon_udp_clientmsg_send
// In Param   : data bytes & respective size in dlt format
// Out Param  : NIL
// Description: common interface to send data via UDP protocol
// **************************************************************************
void dlt_daemon_udp_clientmsg_send(DltDaemonClientSockInfo* clientinfo,
                              void* data1, int size1, void* data2, int size2, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);
    if((clientinfo != NULL) && (clientinfo->isvalidflag == ADDRESS_VALID) &&
        (data1 != NULL) && (size1 > 0) &&
        (data2 != NULL) && (size2 > 0)) {
        void *data = (void *) calloc(size1 + size2, sizeof(char));
        if(data != NULL) {
            memcpy(data, data1, size1);
            memcpy(data + size1, data2, size2);

            if(sendto(g_udp_sock_fd, data, size1 + size2, 0, (struct sockaddr*)&clientinfo->clientaddr,
                    clientinfo->clientaddr_size) < 0)
                dlt_log(LOG_ERR,"dlt_daemon_udp_clientmsg_send1: Send UDP Packet Data failed\n");

            free(data);
            data = NULL;
        } else {
            dlt_log(LOG_ERR,"dlt_daemon_udp_clientmsg_send: calloc failure\n");
        }
    } else if ((clientinfo != NULL) && (clientinfo->isvalidflag == ADDRESS_VALID) &&
        (data1 != NULL) && (size1 > 0)) {
        if(sendto(g_udp_sock_fd, data1, size1, 0, (struct sockaddr*)&clientinfo->clientaddr,
                    clientinfo->clientaddr_size) < 0)
            dlt_log(LOG_ERR,"dlt_daemon_udp_clientmsg_send2: Send UDP Packet Data failed\n");

    } else {
        if(NULL == clientinfo)
            dlt_log(LOG_ERR,"dlt_daemon_udp_clientmsg_send: clientinfo is NULL\n");

        if(clientinfo->isvalidflag != ADDRESS_VALID)
            dlt_vlog(LOG_ERR,"dlt_daemon_udp_clientmsg_send: clientinfo->isvalidflag != ADDRESS_VALID %d\n",
                    clientinfo->isvalidflag);

        if(data1 == NULL)
            dlt_log(LOG_ERR,"dlt_daemon_udp_clientmsg_send: data1 is NULL\n");

        if(data2 == NULL)
            dlt_log(LOG_ERR,"dlt_daemon_udp_clientmsg_send: data2 is NULL\n");

        if(size1 <= 0)
            dlt_log(LOG_ERR,"dlt_daemon_udp_clientmsg_send: size1 <= 0\n");

        if(size2 <= 0)
            dlt_log(LOG_ERR,"dlt_daemon_udp_clientmsg_send: size2 <= 0\n");

        dlt_log(LOG_ERR,"dlt_daemon_udp_clientmsg_send: Params error\n");
    }
}
