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
 * \file dlt_daemon_udp_socket.h
 */

#ifndef DLT_DAEMON_UDP_SOCKET_H
#define DLT_DAEMON_UDP_SOCKET_H

#include "dlt-daemon.h"

DltReturnValue dlt_daemon_udp_connection_setup(DltDaemonLocal *daemon_local);
void dlt_daemon_udp_dltmsg_multicast(void *data1, int size1, void *data2, int size2,
                                     int verbose);
void dlt_daemon_udp_close_connection(void);

#endif /* DLT_DAEMON_UDP_SOCKET_H */
