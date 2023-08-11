/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2015 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
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
 * \author
 * Frederic Berat <fberat@de.adit-jv.com>
 *
 * \copyright Copyright © 2015 Advanced Driver Information Technology. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_daemon_connection.h
 */

#ifndef DLT_DAEMON_CONNECTION_H
#define DLT_DAEMON_CONNECTION_H

#include "dlt_daemon_connection_types.h"
#include "dlt_daemon_event_handler_types.h"
#include "dlt-daemon.h"

int dlt_connection_send_multiple(DltConnection *, void *, int, void *, int, int);

DltConnection *dlt_connection_get_next(DltConnection *, int);
int dlt_connection_create_remaining(DltDaemonLocal *);

int dlt_connection_create(DltDaemonLocal *,
                          DltEventHandler *,
                          int,
                          int,
                          DltConnectionType);
void dlt_connection_destroy(DltConnection *);

void *dlt_connection_get_callback(DltConnection *);

#ifdef DLT_UNIT_TESTS
int dlt_connection_send(DltConnection *conn,
                        void *msg,
                        size_t msg_size);

void dlt_connection_destroy_receiver(DltConnection *con);

DltReceiver *dlt_connection_get_receiver(DltDaemonLocal *daemon_local,
                                         DltConnectionType type,
                                         int fd);
#endif

#endif /* DLT_DAEMON_CONNECTION_H */
