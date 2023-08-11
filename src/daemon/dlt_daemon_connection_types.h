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
 * \file dlt_daemon_connection_types.h
 */

#ifndef DLT_DAEMON_CONNECTION_TYPES_H
#define DLT_DAEMON_CONNECTION_TYPES_H
#include "dlt_common.h"

typedef enum {
    UNDEFINED, /* Undefined status */
    INACTIVE,  /* Connection is inactive, excluded from poll handling */
    ACTIVE,    /* Connection is actively handled by poll */
    DEACTIVATE,/* Request for deactivation of the connection */
    ACTIVATE   /* Request for activation of the connection */
} DltConnectionStatus;

typedef enum {
    DLT_CONNECTION_NONE = 0,
    DLT_CONNECTION_CLIENT_CONNECT,
    DLT_CONNECTION_CLIENT_MSG_TCP,
    DLT_CONNECTION_CLIENT_MSG_SERIAL,
    DLT_CONNECTION_APP_CONNECT,
    DLT_CONNECTION_APP_MSG,
    DLT_CONNECTION_ONE_S_TIMER,
    DLT_CONNECTION_SIXTY_S_TIMER,
    DLT_CONNECTION_SYSTEMD_TIMER,
    DLT_CONNECTION_CONTROL_CONNECT,
    DLT_CONNECTION_CONTROL_MSG,
    DLT_CONNECTION_GATEWAY,
    DLT_CONNECTION_GATEWAY_TIMER,
    DLT_CONNECTION_TYPE_MAX
} DltConnectionType;

#define DLT_CON_MASK_CLIENT_CONNECT     (1 << DLT_CONNECTION_CLIENT_CONNECT)
#define DLT_CON_MASK_CLIENT_MSG_TCP     (1 << DLT_CONNECTION_CLIENT_MSG_TCP)
#define DLT_CON_MASK_CLIENT_MSG_SERIAL  (1 << DLT_CONNECTION_CLIENT_MSG_SERIAL)
#define DLT_CON_MASK_APP_MSG            (1 << DLT_CONNECTION_APP_MSG)
#define DLT_CON_MASK_APP_CONNECT        (1 << DLT_CONNECTION_APP_CONNECT)
#define DLT_CON_MASK_ONE_S_TIMER        (1 << DLT_CONNECTION_ONE_S_TIMER)
#define DLT_CON_MASK_SIXTY_S_TIMER      (1 << DLT_CONNECTION_SIXTY_S_TIMER)
#define DLT_CON_MASK_SYSTEMD_TIMER      (1 << DLT_CONNECTION_SYSTEMD_TIMER)
#define DLT_CON_MASK_CONTROL_CONNECT    (1 << DLT_CONNECTION_CONTROL_CONNECT)
#define DLT_CON_MASK_CONTROL_MSG        (1 << DLT_CONNECTION_CONTROL_MSG)
#define DLT_CON_MASK_GATEWAY            (1 << DLT_CONNECTION_GATEWAY)
#define DLT_CON_MASK_GATEWAY_TIMER      (1 << DLT_CONNECTION_GATEWAY_TIMER)
#define DLT_CON_MASK_ALL                (0xffff)

typedef uintptr_t DltConnectionId;

/* TODO: squash the DltReceiver structure in there
 * and remove any other duplicates of FDs
 */
typedef struct DltConnection {
    DltConnectionId id;
    DltReceiver *receiver; /**< Receiver structure for this connection */
    DltConnectionType type; /**< Represents what type of handle is this (like FIFO, serial, client, server) */
    DltConnectionStatus status; /**< Status of connection */
    struct DltConnection *next;   /**< For multiple client connection using linked list */
    int ev_mask; /**< Mask to set when registering the connection for events */
} DltConnection;

#endif /* DLT_DAEMON_CONNECTION_TYPES_H */
