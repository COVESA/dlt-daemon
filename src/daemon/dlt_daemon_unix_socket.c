/*
 * @licence app begin@
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2015, Advanced Driver Information Technology
 * Copyright of Advanced Driver Information Technology, Bosch and Denso
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
 * Christoph Lipka <clipka@jp.adit-jv.com>
 *
 * \copyright Copyright © 2015 ADIT. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_daemon_unix_socket.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <syslog.h>
#include <errno.h>
#include "dlt-daemon.h"
#include "dlt_common.h"
#include "dlt-daemon_cfg.h"
#include "dlt_daemon_socket.h"
#include "dlt_daemon_unix_socket.h"

char err_string[DLT_DAEMON_TEXTBUFSIZE];

int dlt_daemon_unix_socket_open(int *sock, char *sock_path)
{
    struct sockaddr_un addr;

    if (sock == NULL || sock_path == NULL)
    {
        dlt_log(LOG_ERR, "dlt_daemon_unix_socket_open: arguments invalid");
        return -1;
    }

    if ((*sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        dlt_log(LOG_WARNING, "unix socket: socket() error");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, sock_path, sizeof(addr.sun_path));

    unlink(sock_path);

    if (bind(*sock, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
        dlt_log(LOG_WARNING, "unix socket: bind() error");
        return -1;
    }

    if (listen(*sock, 1) == -1)
    {
        dlt_log(LOG_WARNING, "unix socket: listen error");
        return -1;
    }

    return 0;
}

int dlt_daemon_unix_socket_close(int sock)
{
    int ret = close(sock);

    if (ret != 0)
    {
        sprintf(err_string, "unix socket close failed: %s", strerror(errno));
        dlt_log(LOG_WARNING, err_string);
    }

    return ret;
}

int dlt_daemon_unix_socket_send(
        int sock,
        void *data1,
        int size1,
        void *data2,
        int size2,
        char serialheader)
{
    /* re-use socket send function */
    return dlt_daemon_socket_send(
        sock,
        data1,
        size1,
        data2,
        size2,
        serialheader);
}
