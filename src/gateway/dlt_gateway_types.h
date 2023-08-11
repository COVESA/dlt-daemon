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

#include "dlt_protocol.h"
#include "dlt_client.h"

#define DLT_GATEWAY_CONFIG_PATH CONFIGURATION_FILES_DIR "/dlt_gateway.conf"
#define DLT_GATEWAY_TIMER_DEFAULT_INTERVAL 1
#define DLT_GATEWAY_GENERAL_SECTION_NAME "General"

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

typedef enum
{
    CONTROL_MESSAGE_UNDEFINED = -1,
    CONTROL_MESSAGE_ON_STARTUP,     /* send on startup */
    CONTROL_MESSAGE_PERIODIC,       /* send periodically */
    CONTROL_MESSAGE_BOTH,           /* send on startup and periodically */
    CONTROL_MESSAGE_ON_DEMAND       /* send on demand only */
} control_msg_trigger;

typedef enum
{
    CONTROL_MESSAGE_REQUEST_UNDEFINED = -1,
    CONTROL_MESSAGE_NOT_REQUESTED,  /* control msg not requested (default) */
    CONTROL_MESSAGE_REQUESTED       /* control msg requested */
} control_msg_request;

/* Passive control message */
typedef struct DltPassiveControlMessage {
    uint32_t id;                /* msg ID */
    uint32_t user_id;
    control_msg_trigger type;   /* on startup or periodic or both */
    control_msg_request req;    /* whether it is requested from gateway or not */
    int interval;               /* interval for periodic sending. if on startup, -1 */
    struct DltPassiveControlMessage *next; /* for multiple passive control message */
} DltPassiveControlMessage;

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
    int sendtime;               /* periodic sending max time */
    int sendtime_cnt;           /* periodic sending counter */
    DltPassiveControlMessage *p_control_msgs; /* passive control msgs */
    DltPassiveControlMessage *head; /* to go back to the head pointer of p_control_msgs */
    int send_serial;            /* Send serial header with control messages */
    DltClient client;           /* DltClient structure */
    int default_log_level;      /* Default Log Level on passive node */
} DltGatewayConnection;

/* DltGateway structure */
typedef struct
{
    int send_serial;     /* Default: Send serial header with control messages */
    DltGatewayConnection *connections; /* pointer to connections */
    int num_connections; /* number of connections */
    int interval;        /* interval of retry connection */
} DltGateway;

typedef struct {
    char *key;  /* The configuration key*/
    int (*func)(DltGatewayConnection *con, char *value); /* Conf handler */
    int is_opt; /* If the configuration is optional or not */
} DltGatewayConf;

typedef struct {
    char *key;  /* The configuration key*/
    int (*func)(DltGateway *gateway, char *value); /* Conf handler */
    int is_opt; /* If the configuration is optional or not */
} DltGatewayGeneralConf;

typedef enum {
    GW_CONF_IP_ADDRESS = 0,
    GW_CONF_PORT,
    GW_CONF_ECUID,
    GW_CONF_CONNECT,
    GW_CONF_TIMEOUT,
    GW_CONF_SEND_CONTROL,
    GW_CONF_SEND_PERIODIC_CONTROL,
    GW_CONF_SEND_SERIAL_HEADER,
    GW_CONF_COUNT
} DltGatewayConfType;

typedef enum {
    GW_CONF_GENERAL_INTERVAL = 0,
    GW_CONF_GENEREL_COUNT
} DltGatewayGeneralConfType;

#endif /* DLT_GATEWAY_TYPES_H_ */
