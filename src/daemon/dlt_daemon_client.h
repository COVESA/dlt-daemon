/**
 * @licence app begin@
 * Copyright (C) 2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt - Diagnostic Log and Trace console apps.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> BMW 2011-2012
 *
 * \file dlt_daemon_client.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_daemon_client.h                                           **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
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

/*******************************************************************************
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  aw          Alexander Wenzel           BMW                                **
*******************************************************************************/

#ifndef DLT_DAEMON_CLIENT_H
#define DLT_DAEMON_CLIENT_H

#include <limits.h> /* for NAME_MAX */

#include "dlt_daemon_common.h"
#include "dlt_user_shared.h"
#include "dlt_user_shared_cfg.h"

#include <dlt_offline_trace.h>
#include <sys/time.h>

/**
 * Send out message to client or store message in offline trace.
 * @param sock connection handle used for sending response
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param data1 pointer to data
 * @param size1 size of data
 * @param data2 pointer to data
 * @param size2 size of data
 * @param verbose if set to true verbose information is printed out.
 * @param control if set to true message is not stored in offline trace
 * @return unequal 0 if there is an error or buffer is full
 */
int dlt_daemon_client_send(int sock,DltDaemon *daemon,DltDaemonLocal *daemon_local,void* data1,int size1,void* data2,int size2,int verbose, int control);

/**
 * Send out response message to dlt client
 * @param sock connection handle used for sending response
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param msg pointer to response message
 * @param appid pointer to application id to be used in response message
 * @param contid pointer to context id to be used in response message
 * @param verbose if set to true verbose information is printed out.
 * @return -1 if there is an error or buffer is full
 */
int dlt_daemon_client_send_control_message(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, DltMessage *msg, char* appid, char* contid, int verbose);
/**
 * Process and generate response to received get log info control message
 * @param sock connection handle used for sending response
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param msg pointer to received control message
 * @param verbose if set to true verbose information is printed out.
 */
void dlt_daemon_control_get_log_info(int sock, DltDaemon *daemon,DltDaemonLocal *daemon_local, DltMessage *msg, int verbose);
/**
 * Process and generate response to received get software version control message
 * @param sock connection handle used for sending response
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param verbose if set to true verbose information is printed out.
 */
void dlt_daemon_control_get_software_version(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
/**
 * Process and generate response to received get default log level control message
 * @param sock connection handle used for sending response
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param verbose if set to true verbose information is printed out.
 */
void dlt_daemon_control_get_default_log_level(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
/**
 * Process and generate response to message buffer overflow control message
 * @param sock connection handle used for sending response
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param verbose if set to true verbose information is printed out.
 * @return -1 if there is an error or buffer overflow, else 0
 */
int dlt_daemon_control_message_buffer_overflow(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, unsigned int overflow_counter,char* apid, int verbose);
/**
 * Generate response to control message from dlt client
 * @param sock connection handle used for sending response
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param service_id service id of control message
 * @param status status of response (e.g. ok, not supported, error)
 * @param verbose if set to true verbose information is printed out.
 */
void dlt_daemon_control_service_response(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, uint32_t service_id, int8_t status, int verbose);
/**
 * Send control message unregister context (add on to AUTOSAR standard)
 * @param sock connection handle used for sending response
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param apid application id to be unregisteres
 * @param ctid context id to be unregistered
 * @param comid Communication id where apid is unregistered
 * @param verbose if set to true verbose information is printed out.
 */
int dlt_daemon_control_message_unregister_context(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, char* apid, char* ctid, char* comid, int verbose);
/**
 * Send control message connection info (add on to AUTOSAR standard)
 * @param sock connection handle used for sending response
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param state state of connection
 * @param comid Communication id where connection state changed
 * @param verbose if set to true verbose information is printed out.
 */
int dlt_daemon_control_message_connection_info(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, uint8_t state, char* comid, int verbose);
/**
 * Send control message timezone (add on to AUTOSAR standard)
 * @param sock connection handle used for sending response
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param verbose if set to true verbose information is printed out.
 */
int dlt_daemon_control_message_timezone(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
/**
 * Send control message marker (add on to AUTOSAR standard)
 * @param sock connection handle used for sending response
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param verbose if set to true verbose information is printed out.
 */
int dlt_daemon_control_message_marker(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);
/**
 * Process received control message from dlt client
 * @param sock connection handle used for sending response
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param msg pointer to received control message
 * @param verbose if set to true verbose information is printed out.
 */
int dlt_daemon_client_process_control(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, DltMessage *msg, int verbose);
/**
 * Process and generate response to received sw injection control message
 * @param sock connection handle used for sending response
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param msg pointer to received sw injection control message
 * @param verbose if set to true verbose information is printed out.
 */
void dlt_daemon_control_callsw_cinjection(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, DltMessage *msg, int verbose);
/**
 * Process and generate response to received set log level control message
 * @param sock connection handle used for sending response
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param msg pointer to received control message
 * @param verbose if set to true verbose information is printed out.
 */
void dlt_daemon_control_set_log_level(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, DltMessage *msg, int verbose);
/**
 * Process and generate response to received set trace status control message
 * @param sock connection handle used for sending response
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param msg pointer to received control message
 * @param verbose if set to true verbose information is printed out.
 */
void dlt_daemon_control_set_trace_status(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, DltMessage *msg, int verbose);
/**
 * Process and generate response to received set default log level control message
 * @param sock connection handle used for sending response
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param msg pointer to received control message
 * @param verbose if set to true verbose information is printed out.
 */
void dlt_daemon_control_set_default_log_level(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, DltMessage *msg, int verbose);
/**
 * Process and generate response to received set default trace status control message
 * @param sock connection handle used for sending response
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param msg pointer to received control message
 * @param verbose if set to true verbose information is printed out.
 */
void dlt_daemon_control_set_default_trace_status(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, DltMessage *msg, int verbose);
/**
 * Process and generate response to set timing packets control message
 * @param sock connection handle used for sending response
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param msg pointer to received control message
 * @param verbose if set to true verbose information is printed out.
 */
void dlt_daemon_control_set_timing_packets(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, DltMessage *msg, int verbose);
/**
 * Send time control message
 * @param sock connection handle used for sending response
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param verbose if set to true verbose information is printed out.
 */
void dlt_daemon_control_message_time(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose);

#endif /* DLT_DAEMON_CLIENT_H */
