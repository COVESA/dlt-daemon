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
 * \file dlt_daemon_event_handler_types.h
 */

#include <poll.h>

#include "dlt_daemon_connection_types.h"

#ifndef DLT_DAEMON_EVENT_HANDLER_TYPES_H
#define DLT_DAEMON_EVENT_HANDLER_TYPES_H

/* FIXME: Remove the need for DltDaemonLocal everywhere in the code
 * These typedefs are needed by DltDaemonLocal which is
 * itself needed for functions used by the event handler
 * (as this structure is used everywhere in the code ...)
 */

typedef enum {
    DLT_TIMER_PACKET = 0,
    DLT_TIMER_ECU,
#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
    DLT_TIMER_SYSTEMD,
#endif
    DLT_TIMER_GATEWAY,
    DLT_TIMER_UNKNOWN
} DltTimers;

typedef struct {
    struct pollfd *pfd;
    nfds_t nfds;
    nfds_t max_nfds;
    DltConnection *connections;
} DltEventHandler;

#endif /* DLT_DAEMON_EVENT_HANDLER_TYPES_H */
