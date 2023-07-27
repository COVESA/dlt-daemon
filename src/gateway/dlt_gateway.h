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
 * \file dlt_gateway.h
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_gateway.h                                                 **
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

#ifndef DLT_GATEWAY_H_
#define DLT_GATEWAY_H_

#include "dlt-daemon.h"
#include "dlt_gateway_types.h"
/**
 * Initialize the gateway to passive nodes
 *
 * TODO: Make path to configuration file configurable
 *
 * @param daemon_local  pointer to DltDaemonLocal
 * @param verbose       verbose flag
 * @return 0 on success, -1 on error
 */
int dlt_gateway_init(DltDaemonLocal *daemon_local, int verbose);

/**
 * De-initialize the gateway. All internal data will be freed.
 *
 * @param g DltGateway pointer
 * @param verbose verbose flag
 */
void dlt_gateway_deinit(DltGateway *g, int verbose);

/**
 * Establish all connections to passive nodes that are configured to be started
 * on daemon startup and add this connections to the main event loop.
 *
 * TODO: This function is called during gateway initialization and in main loop
 *       whenever the poll returns. This may need to be improved.
 *
 * @param g             DltGateway
 * @param daemon_local  DltDaemonLocal
 * @param verbose       verbose flag
 * @return 0 on success, -1 on error
 */
int dlt_gateway_establish_connections(DltGateway *g,
                                      DltDaemonLocal *daemon_local,
                                      int verbose);

/**
 * Return the receiver for a given file descriptor
 *
 * @param g DltGateway
 * @param fd file descriptor
 * @return Pointer to DltReceiver on success, NULL otherwise
 */
DltReceiver *dlt_gateway_get_connection_receiver(DltGateway *g, int fd);


/**
 * Process incoming messages from passive nodes
 *
 * @param daemon          DltDaemon
 * @param daemon_local    DltDaemonLocal
 * @param recv            DltReceiver structure
 * @param verbose verbose flag
 * @return 0 on success, -1 otherwise
 */
int dlt_gateway_process_passive_node_messages(DltDaemon *daemon,
                                              DltDaemonLocal *daemon_local,
                                              DltReceiver *recv,
                                              int verbose);

/**
 * Process gateway timer
 *
 * @param daemon          DltDaemon
 * @param daemon_local    DltDaemonLocal
 * @param rec             DltReceiver
 * @param verbose verbose flag
 * @return 0 on success, -1 otherwise
 */
int dlt_gateway_process_gateway_timer(DltDaemon *daemon,
                                      DltDaemonLocal *daemon_local,
                                      DltReceiver *rec,
                                      int verbose);

/**
 * Forward control messages to the specified passive node DLT Daemon.
 *
 * @param g            DltGateway
 * @param daemon_local DltDaemonLocal
 * @param msg          DltMessage
 * @param ecu          Identifier of the passive node
 * @param verbose      verbose flag
 * @return 0 on success, -1 otherwise
 */
int dlt_gateway_forward_control_message(DltGateway *g,
                                        DltDaemonLocal *daemon_local,
                                        DltMessage *msg,
                                        char *ecu,
                                        int verbose);

/**
 * Process on demand connect/disconnect of passive nodes
 *
 * @param g                 DltGateway
 * @param daemon_local      DltDaemonLocal
 * @param node_id           Passive Node identifier
 * @param connection_status Connection status
 * @param verbose           verbose flag
 * @return 0 on success, -1 otherwise
 */
int dlt_gateway_process_on_demand_request(DltGateway *g,
                                          DltDaemonLocal *daemon_local,
                                          char *node_id,
                                          int connection_status,
                                          int verbose);

/**
 * Send control message to passive node
 *
 * @param con           DltGatewayConnection
 * @param control_msg   DltPassiveControlMessage
 * @param data          DltMessage
 * @param verbose       verbose flag
 * @return 0 on success, -1 otherwise
 */
int dlt_gateway_send_control_message(DltGatewayConnection *con,
                                     DltPassiveControlMessage *control_msg,
                                     void *data,
                                     int verbose);

/**
 * Gets the connection handle of passive node with specified ECU
 *
 * @param g             DltGateway
 * @param ecu           Identifier string
 * @param verbose       verbose flag
 * @returns Gateway connection handle on success, NULL otherwise
 */
DltGatewayConnection *dlt_gateway_get_connection(DltGateway *g,
                                                 char *ecu,
                                                 int verbose);

#endif
