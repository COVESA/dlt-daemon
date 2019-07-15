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


#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

#include "dlt-system.h"

extern DltSystemThreads threads;

DLT_IMPORT_CONTEXT(dltsystem)
DLT_DECLARE_CONTEXT(syslogContext)
#define RECV_BUF_SZ 1024

int init_socket(SyslogOptions opts)
{
    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-syslog, init socket, port: "),
            DLT_INT(opts.Port));

    int sock = -1;
    struct sockaddr_in syslog_addr;

#ifdef DLT_USE_IPv6
    sock = socket(AF_INET6, SOCK_DGRAM, 0);
#else
    sock = socket(AF_INET, SOCK_DGRAM, 0);
#endif

    if (sock < 0) {
        DLT_LOG(syslogContext, DLT_LOG_FATAL,
                DLT_STRING("Unable to create socket for SYSLOG."));
        return -1;
    }

#ifdef DLT_USE_IPv6
    syslog_addr.sin_family = AF_INET6;
#else
    syslog_addr.sin_family = AF_INET;
#endif
    syslog_addr.sin_port = htons(opts.Port);
    syslog_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(syslog_addr.sin_zero), 0, 8);

    if (bind(sock, (struct sockaddr *)&syslog_addr,
             sizeof(struct sockaddr)) == -1) {
        DLT_LOG(syslogContext, DLT_LOG_FATAL,
                DLT_STRING("Unable to bind socket for SYSLOG."));
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
        DLT_LOG(syslogContext, DLT_LOG_INFO, DLT_STRING(recv_data));

    return bytes_read;
}

void syslog_thread(void *v_conf)
{
    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-syslog, in thread."));

    DltSystemConfiguration *conf = (DltSystemConfiguration *)v_conf;
    DLT_REGISTER_CONTEXT(syslogContext, conf->Syslog.ContextId, "SYSLOG Adapter");

    int sock = init_socket(conf->Syslog);

    if (sock < 0)
        return;

    while (!threads.shutdown)
        if (read_socket(sock) < 0) {
            close(sock);
            return;
        }

    close (sock);
}

void start_syslog(DltSystemConfiguration *conf)
{
    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-syslog, start syslog"));
    static pthread_attr_t t_attr;
    static pthread_t pt;
    pthread_create(&pt, &t_attr, (void *)syslog_thread, conf);
    threads.threads[threads.count++] = pt;
}
