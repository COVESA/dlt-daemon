/*
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
#include <string.h>
#include <sys/un.h>
#if defined(ANDROID)
#   include <cutils/sockets.h> /* for android_get_control_socket() */
#   include <libgen.h> /* for basename() */
#else
#   include <sys/socket.h> /* for socket(), connect(), (), and recv() */
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <errno.h>

#include "dlt-daemon.h"
#include "dlt_common.h"
#include "dlt-daemon_cfg.h"
#include "dlt_daemon_socket.h"
#include "dlt_daemon_unix_socket.h"

#ifdef ANDROID
DltReturnValue dlt_daemon_unix_android_get_socket(int *sock, const char *sock_path)
{
    DltReturnValue ret = DLT_RETURN_OK;

    if ((sock == NULL) || (sock_path == NULL)) {
        dlt_log(LOG_ERR, "dlt_daemon_unix_android_get_socket: arguments invalid");
        ret = DLT_RETURN_WRONG_PARAMETER;
    }
    else {
        const char* sock_name = basename(sock_path);
        if (sock_name == NULL) {
            dlt_log(LOG_WARNING,
                    "dlt_daemon_unix_android_get_socket: can't get socket name from its path");
            ret = DLT_RETURN_ERROR;
        }
        else {
            *sock = android_get_control_socket(sock_name);
            if (*sock < 0) {
                dlt_log(LOG_WARNING,
                        "dlt_daemon_unix_android_get_socket: can get socket from init");
                ret = DLT_RETURN_ERROR;
            }
            else {
                if (listen(*sock, 1) == -1) {
                    dlt_vlog(LOG_WARNING, "unix socket: listen error: %s", strerror(errno));
                    ret = DLT_RETURN_ERROR;
                }
            }
        }
    }

    return ret;
}
#endif

int dlt_daemon_unix_socket_open(int *sock, char *sock_path, int type, int mask)
{
    struct sockaddr_un addr;
    int old_mask;

    if ((sock == NULL) || (sock_path == NULL)) {
        dlt_log(LOG_ERR, "dlt_daemon_unix_socket_open: arguments invalid");
        return -1;
    }

    if ((*sock = socket(AF_UNIX, type, 0)) == -1) {
        dlt_log(LOG_WARNING, "unix socket: socket() error");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, sock_path, sizeof(addr.sun_path));

    unlink(sock_path);

    /* set appropriate access permissions */
    old_mask = umask(mask);

    if (bind(*sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        dlt_log(LOG_WARNING, "unix socket: bind() error");
        return -1;
    }

    if (listen(*sock, 1) == -1) {
        dlt_log(LOG_WARNING, "unix socket: listen error");
        return -1;
    }

    /* restore permissions */
    umask(old_mask);

    return 0;
}

int dlt_daemon_unix_socket_close(int sock)
{
    int ret = close(sock);

    if (ret != 0) {
        dlt_vlog(LOG_WARNING, "unix socket close failed: %s", strerror(errno));
    }

    return ret;
}
