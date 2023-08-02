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
 * \author Lassi Marttala <lassi.lm.marttala@partner.bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-system-syslog.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-system-syslog.c                                                  **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Lassi Marttala <lassi.lm.marttala@partner.bmw.de>             **
**              Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
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


#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

#include <systemd/sd-journal.h>
#include <poll.h>

#include "dlt-system.h"

DLT_IMPORT_CONTEXT(dltsystem)
DLT_DECLARE_CONTEXT(syslogContext)
#define RECV_BUF_SZ 1024

int init_socket(SyslogOptions opts)
{
    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-syslog, init socket, port: "),
            DLT_INT(opts.Port));

    int sock = -1;

#ifdef DLT_USE_IPv6
    /* declare struct for IPv6 socket address*/
    struct sockaddr_in6 syslog_addr;
    sock = socket(AF_INET6, SOCK_DGRAM, 0);
#else
    /* declare struct for IPv4 socket address*/
    struct sockaddr_in syslog_addr;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
#endif

    if (sock < 0) {
        DLT_LOG(syslogContext, DLT_LOG_FATAL,
                DLT_STRING("Unable to create socket for SYSLOG."));
        return -1;
    }

    /* initialize struct syslog_addr */
    memset(&syslog_addr, 0, sizeof(syslog_addr));
#ifdef DLT_USE_IPv6
    syslog_addr.sin6_family = AF_INET6;
    syslog_addr.sin6_addr = in6addr_any;
    syslog_addr.sin6_port = htons(opts.Port);
#else
    syslog_addr.sin_family = AF_INET;
    syslog_addr.sin_addr.s_addr = INADDR_ANY;
    syslog_addr.sin_port = htons(opts.Port);
    memset(&(syslog_addr.sin_zero), 0, 8);
#endif

    /* bind the socket address to local interface */
    if (bind(sock, (struct sockaddr *)&syslog_addr,
             sizeof(syslog_addr)) == -1) {
        DLT_LOG(syslogContext, DLT_LOG_FATAL,
                DLT_STRING("Unable to bind socket for SYSLOG, error description: "),
                DLT_STRING(strerror(errno)));
        close(sock);
        return -1;
    }

    return sock;
}

int read_socket(int sock)
{
    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-syslog, read socket"));
    char recv_data[RECV_BUF_SZ];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    int bytes_read = recvfrom(sock, recv_data, RECV_BUF_SZ, 0,
                              (struct sockaddr *)&client_addr, &addr_len);

    if (bytes_read == -1) {
        if (errno == EINTR) {
            return 0;
        }
        else {
            DLT_LOG(syslogContext, DLT_LOG_FATAL,
                    DLT_STRING("Read from socket failed in SYSLOG."));
            return -1;
        }
    }

    recv_data[bytes_read] = '\0';

    if (bytes_read != 0)
    {
        DLT_LOG(syslogContext, DLT_LOG_INFO, DLT_STRING(recv_data));
    }

    return bytes_read;
}

int register_syslog_fd(struct pollfd *pollfd, int i, DltSystemConfiguration *config)
{
    DLT_REGISTER_CONTEXT(syslogContext, config->Syslog.ContextId, "SYSLOG Adapter");
    int syslogSock = init_socket(config->Syslog);
    if (syslogSock < 0) {
        DLT_LOG(dltsystem, DLT_LOG_ERROR, DLT_STRING("Could not init syslog socket\n"));
        return -1;
    }
    pollfd[i].fd = syslogSock;
    pollfd[i].events = POLLIN;
    return syslogSock;
}

void syslog_fd_handler(int syslogSock)
{
    read_socket(syslogSock);
}