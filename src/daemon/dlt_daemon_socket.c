/* @licence app begin@
 * Copyright (C) 2012-2014  BMW AG
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
 * \author
 * Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 * Markus Klein <Markus.Klein@esk.fraunhofer.de>
 * Mikko Rapeli <mikko.rapeli@bmw.de>
 *
 * \file dlt_daemon_socket.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <netdb.h>
#include <ctype.h>
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), (), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <net/if.h>

#ifdef linux
#include <sys/timerfd.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#ifdef linux
#include <linux/stat.h>
#endif

#include "dlt_types.h"
#include "dlt-daemon.h"
#include "dlt-daemon_cfg.h"
#include "dlt_daemon_common_cfg.h"

#include "dlt_daemon_socket.h"

/** Global text output buffer, mainly used for creation of error/warning strings */
static char str[DLT_DAEMON_TEXTBUFSIZE];

int dlt_daemon_socket_open(int *sock)
{
    int yes = 1;
    char portnumbuffer[33];
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // force IPv6 - will still work with IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP address

    sprintf(portnumbuffer, "%d", DLT_DAEMON_TCP_PORT);
    if ((rv = getaddrinfo(NULL, portnumbuffer, &hints, &servinfo)) != 0) {
        snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "getaddrinfo: %s\n", gai_strerror(rv));
        dlt_log(LOG_ERR, str);
        return -1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((*sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            dlt_log(LOG_ERR, "socket() error\n");
            continue;
        }

        snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "%s: Socket created - socket_family:%i, socket_type:%i, protocol:%i\n",
                 __FUNCTION__, p->ai_family, p->ai_socktype, p->ai_protocol);
        dlt_log(LOG_INFO, str);

        if (setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
            snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "dlt_daemon_socket_open: Setsockopt error in dlt_daemon_local_connection_init: %s\n", strerror(errno));
            dlt_log(LOG_ERR, str);
            continue;
        }

        if (bind(*sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(*sock);
            dlt_log(LOG_ERR, "bind() error\n");
            continue;
        }

        break;
    }

    if (p == NULL) {
        dlt_log(LOG_ERR, "failed to bind socket\n");
        return -1;
    }

    freeaddrinfo(servinfo);

    snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "%s: Listening on port: %u\n", __FUNCTION__, DLT_DAEMON_TCP_PORT);
    dlt_log(LOG_INFO, str);

    // get socket buffer size
    snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "dlt_daemon_socket_open: Socket send queue size: %d\n", dlt_daemon_socket_get_send_qeue_max_size(*sock));
    dlt_log(LOG_INFO, str);

    if (listen(*sock, 3) < 0)
    {
        dlt_log(LOG_ERR, "dlt_daemon_socket_open: listen() failed!\n");
        return -1;
    }

    return 0; // OK
}

int dlt_daemon_socket_close(int sock)
{
	close(sock);

	return 0;
}

int dlt_daemon_socket_send(int sock,void* data1,int size1,void* data2,int size2,char serialheader)
{
	/* Optional: Send serial header, if requested */
	if (serialheader)
	{
		if ( 0 > send(sock, dltSerialHeader,sizeof(dltSerialHeader),0) )
			return DLT_DAEMON_ERROR_SEND_FAILED;

	}

	/* Send data */
	if(data1 && size1>0)
	{
		if (0 > send(sock, data1,size1,0))
			return DLT_DAEMON_ERROR_SEND_FAILED;
	}

	if(data2 && size2>0)
	{
		if (0 > send(sock, data2,size2,0))
		return DLT_DAEMON_ERROR_SEND_FAILED;
	}

	return DLT_DAEMON_ERROR_OK;
}

int dlt_daemon_socket_get_send_qeue_max_size(int sock)
{
    int n = 0;
    socklen_t m = sizeof(n);
    getsockopt(sock,SOL_SOCKET,SO_SNDBUF,(void *)&n, &m);

    return n;
}

