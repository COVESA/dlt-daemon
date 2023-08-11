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
 * \file dlt_daemon_udp_common_socket.h
 */

#ifndef DLT_DAEMON_UDP_COMMON_SOCKET_H
#define DLT_DAEMON_UDP_COMMON_SOCKET_H

#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <errno.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <syslog.h>
#include <sys/socket.h> /* for socket(), connect(), (), and recv() */
#include <unistd.h>     /* for close() */

#include "dlt_common.h"
#include "dlt-daemon.h"
#include "dlt_daemon_udp_socket.h"
#include "dlt_types.h"

/* #define variables */
#define ADDRESS_VALID 1
#define ADDRESS_INVALID 0
#define SOCKPORT_MAX_LEN 6 /* port range 0-65535 */
#define SYSTEM_CALL_ERROR -1
#define ZERO_BYTE_RECIEVED 0
#define ONE_BYTE_RECIEVED 0

typedef struct sockaddr_storage CLIENT_ADDR_STRUCT;
typedef socklen_t CLIENT_ADDR_STRUCT_SIZE;

/* udp strutures */
typedef struct
{
    CLIENT_ADDR_STRUCT clientaddr;
    CLIENT_ADDR_STRUCT_SIZE clientaddr_size;
    int isvalidflag;
} DltDaemonClientSockInfo;

/* Function prototype declaration */
void dlt_daemon_udp_init_clientstruct(DltDaemonClientSockInfo *clientinfo_struct);
DltReturnValue dlt_daemon_udp_socket_open(int *sock, unsigned int servPort);
void dlt_daemon_udp_setmulticast_addr(DltDaemonLocal *daemon_local);

#endif /* DLT_DAEMON_UDP_COMMON_SOCKET_H */

