/*
 * @licence app begin@
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2015 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
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
 * \copyright Copyright © 2015 Advanced Driver Information Technology. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_gateway_types.h
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_gateway_types.h                                           **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Christoph Lipka clipka@jp.adit-jv.com                         **
**  PURPOSE   :                                                               **
**                                                                            **
**  REMARKS   :                                                               **
**                                                                            **
**  PLATFORM DEPENDANT [yes/no]: yes                                          **
**                                                                            **
**  TO BE CHANGED BY USER [yes/no]: no                                        **
**                                                                            **
*******************************************************************************/

/*******************************************************************************
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  cl          Christoph Lipka            ADIT                               **
*******************************************************************************/

#ifndef DLT_GATEWAY_TYPES_H_
#define DLT_GATEWAY_TYPES_H_

#include "dlt_client.h"

#define DLT_GATEWAY_CONFIG_PATH CONFIGURATION_FILES_DIR "/dlt_gateway.conf"
#define DLT_GATEWAY_TIMER_INTERVAL 1

#define DLT_GATEWAY_RECONNECT_MAX 1 /* reconnect once after connection loss */

/* maximum number of control messages that can be send after connection is
 * established */
#define DLT_GATEWAY_MAX_STARTUP_CTRL_MSG 10

typedef enum
{
    DLT_GATEWAY_UNINITIALIZED,
    DLT_GATEWAY_INITIALIZED,
    DLT_GATEWAY_CONNECTED,
    DLT_GATEWAY_DISCONNECTED
} connection_status;

typedef enum
{
    DLT_GATEWAY_UNDEFINED = -1,
    DLT_GATEWAY_ON_STARTUP,    /* connect directly on startup */
    DLT_GATEWAY_ON_DEMAND,     /* connect on demand only */
    DLT_GATEWAY_DISABLED       /* disable this connection due to problems */
} connection_trigger;

/* DLT Gateway connection structure */
typedef struct {
    int handle;                 /* connection handle */
    connection_status status;   /* connected/disconnected */
    char *ecuid;                /* name of passive node */
    char *ip_address;           /* IP address */
    int sock_domain;            /* socket domain */
    int sock_type;              /* socket type */
    int sock_protocol;          /* socket protocol */
    int port;                   /* port */
    connection_trigger trigger; /* connection trigger */
    int timeout;                /* connection timeout */
    int timeout_cnt;            /* connection timeout counter */
    int reconnect_cnt;          /* reconnection counter */
    int control_msgs[DLT_GATEWAY_MAX_STARTUP_CTRL_MSG]; /* msg IDs send on startup */
    int send_serial;            /* Send serial header with control messages */
    DltClient client;           /* DltClient structure */
} DltGatewayConnection;

/* DltGateway structure */
typedef struct
{
    int send_serial;     /* Default: Send serial header with control messages */
    DltGatewayConnection *connections; /* pointer to connections */
    int num_connections; /* number of connections */
} DltGateway;

#endif /* DLT_GATEWAY_TYPES_H_ */
