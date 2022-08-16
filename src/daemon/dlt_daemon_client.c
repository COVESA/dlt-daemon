/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
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
 * Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 * Markus Klein <Markus.Klein@esk.fraunhofer.de>
 * Mikko Rapeli <mikko.rapeli@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_daemon_client.c
 */

#include <netdb.h>
#include <ctype.h>
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), (), and recv() */
#include <sys/stat.h>   /* for stat() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <pthread.h>

#ifdef linux
#   include <sys/timerfd.h>
#endif
#include <sys/time.h>
#if defined(linux) && defined(__NR_statx)
#   include <linux/stat.h>
#endif

#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
#   include <systemd/sd-daemon.h>
#endif

#include "dlt_types.h"
#include "dlt-daemon.h"
#include "dlt-daemon_cfg.h"
#include "dlt_daemon_common_cfg.h"

#include "dlt_daemon_socket.h"
#include "dlt_daemon_serial.h"

#include "dlt_daemon_client.h"
#include "dlt_daemon_connection.h"
#include "dlt_daemon_event_handler.h"

#include "dlt_daemon_offline_logstorage.h"
#include "dlt_gateway.h"

/** Inline function to calculate/set the requested log level or traces status
 *  with default log level or trace status when "ForceContextLogLevelAndTraceStatus"
 *  is enabled and set to 1 in dlt.conf file.
 *
 * @param request_log The requested log level (or) trace status
 * @param context_log The default log level (or) trace status
 *
 * @return The log level if requested log level is lower or equal to ContextLogLevel
 */
static inline int8_t getStatus(uint8_t request_log, int context_log)
{
    return (request_log <= context_log) ? request_log : context_log;
}

#ifdef UDP_CONNECTION_SUPPORT
#   include "dlt_daemon_udp_socket.h"
#endif

/** @brief Sends up to 2 messages to all the clients.
 *
 * Runs through the client list and sends the messages to them. If the message
 * transfer fails and the connection is a socket connection, the socket is closed.
 * Takes and release dlt_daemon_mutex.
 *
 * @param daemon Daemon structure needed for socket closure.
 * @param daemon_local Daemon local structure
 * @param data1 The first message to be sent.
 * @param size1 The size of the first message.
 * @param data2 The second message to be send.
 * @param size2 The second message size.
 * @param verbose Needed for socket closure.
 *
 * @return The amount of data transfered.
 */
static int dlt_daemon_client_send_all_multiple(DltDaemon *daemon,
                                               DltDaemonLocal *daemon_local,
                                               void *data1,
                                               int size1,
                                               void *data2,
                                               int size2,
                                               int verbose)
{
    int sent = 0;
    unsigned int i = 0;
    int ret = 0;
    DltConnection *temp = NULL;
    int type_mask =
        (DLT_CON_MASK_CLIENT_MSG_TCP | DLT_CON_MASK_CLIENT_MSG_SERIAL);

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL)) {
        dlt_vlog(LOG_ERR, "%s: Invalid parameters\n", __func__);
        return 0;
    }

    for (i = 0; i < daemon_local->pEvent.nfds; i++)
    {
        temp = dlt_event_handler_find_connection(&(daemon_local->pEvent),
                                        daemon_local->pEvent.pfd[i].fd);

        if ((temp == NULL) || (temp->receiver == NULL) ||
            !((1 << temp->type) & type_mask)) {
            dlt_log(LOG_DEBUG, "The connection not found or the connection type not TCP/Serial.\n");
            continue;
        }

        ret = dlt_connection_send_multiple(temp,
                                           data1,
                                           size1,
                                           data2,
                                           size2,
                                           daemon->sendserialheader);

        if ((ret != DLT_DAEMON_ERROR_OK) &&
            (DLT_CONNECTION_CLIENT_MSG_TCP == temp->type)) {
            dlt_daemon_close_socket(temp->receiver->fd,
                                    daemon,
                                    daemon_local,
                                    verbose);
        }

        if (ret != DLT_DAEMON_ERROR_OK)
            dlt_vlog(LOG_WARNING, "%s: send dlt message failed\n", __func__);
        else
            /* If sent to at  least one client,
             * then do not store in ring buffer
             */
            sent = 1;
    } /* for */

    return sent;
}

int dlt_daemon_client_send(int sock,
                           DltDaemon *daemon,
                           DltDaemonLocal *daemon_local,
                           void *storage_header,
                           int storage_header_size,
                           void *data1,
                           int size1,
                           void *data2,
                           int size2,
                           int verbose)
{
    int sent, ret;
    static int sent_message_overflow_cnt = 0;

    if ((daemon == NULL) || (daemon_local == NULL)) {
        dlt_vlog(LOG_ERR, "%s: Invalid arguments\n", __func__);
        return DLT_DAEMON_ERROR_UNKNOWN;
    }

    if ((sock != DLT_DAEMON_SEND_TO_ALL) && (sock != DLT_DAEMON_SEND_FORCE)) {
        /* Send message to specific socket */
        if (isatty(sock)) {
            if ((ret =
                     dlt_daemon_serial_send(sock, data1, size1, data2, size2,
                                            daemon->sendserialheader))) {
                dlt_vlog(LOG_WARNING, "%s: serial send dlt message failed\n", __func__);
                return ret;
            }
        } else {
            if ((ret =
                     dlt_daemon_socket_send(sock, data1, size1, data2, size2,
                                            daemon->sendserialheader))) {
                dlt_vlog(LOG_WARNING, "%s: socket send dlt message failed\n", __func__);
                return ret;
            }
        }

        return DLT_DAEMON_ERROR_OK;
    }

    /* write message to offline trace */
    /* In the SEND_BUFFER state we must skip offline tracing because the offline traces */
    /* are going without buffering directly to the offline trace. Thus we have to filter out */
    /* the traces that are coming from the buffer. */
    if ((sock != DLT_DAEMON_SEND_FORCE) && (daemon->state != DLT_DAEMON_STATE_SEND_BUFFER)) {
        if (((daemon->mode == DLT_USER_MODE_INTERNAL) || (daemon->mode == DLT_USER_MODE_BOTH))
            && daemon_local->flags.offlineTraceDirectory[0]) {
            if (dlt_offline_trace_write(&(daemon_local->offlineTrace), storage_header, storage_header_size, data1,
                                        size1, data2, size2)) {
                static int error_dlt_offline_trace_write_failed = 0;

                if (!error_dlt_offline_trace_write_failed) {
                    dlt_vlog(LOG_ERR, "%s: dlt_offline_trace_write failed!\n", __func__);
                    error_dlt_offline_trace_write_failed = 1;
                }

                /*return DLT_DAEMON_ERROR_WRITE_FAILED; */
            }
        }

        /* write messages to offline logstorage only if there is an extended header set
         * this need to be checked because the function is dlt_daemon_client_send is called by
         * newly introduced dlt_daemon_log_internal */
        if (daemon_local->flags.offlineLogstorageMaxDevices > 0)
            dlt_daemon_logstorage_write(daemon,
                                        &daemon_local->flags,
                                        storage_header,
                                        storage_header_size,
                                        data1,
                                        size1,
                                        data2,
                                        size2);
    }

    /* send messages to daemon socket */
    if ((daemon->mode == DLT_USER_MODE_EXTERNAL) || (daemon->mode == DLT_USER_MODE_BOTH)) {
#ifdef UDP_CONNECTION_SUPPORT

        if (daemon_local->UDPConnectionSetup == MULTICAST_CONNECTION_ENABLED)
            dlt_daemon_udp_dltmsg_multicast(data1,
                                            size1,
                                            data2,
                                            size2,
                                            verbose);

#endif

        if ((sock == DLT_DAEMON_SEND_FORCE) || (daemon->state == DLT_DAEMON_STATE_SEND_DIRECT)) {
            sent = dlt_daemon_client_send_all_multiple(daemon,
                                                       daemon_local,
                                                       data1,
                                                       size1,
                                                       data2,
                                                       size2,
                                                       verbose);

            if ((sock == DLT_DAEMON_SEND_FORCE) && !sent)
                return DLT_DAEMON_ERROR_SEND_FAILED;
        }
    }

    /* Message was not sent to client, so store it in client ringbuffer */
    if ((sock != DLT_DAEMON_SEND_FORCE) &&
        ((daemon->state == DLT_DAEMON_STATE_BUFFER) || (daemon->state == DLT_DAEMON_STATE_SEND_BUFFER) ||
         (daemon->state == DLT_DAEMON_STATE_BUFFER_FULL))) {
        if (daemon->state != DLT_DAEMON_STATE_BUFFER_FULL) {
            /* Store message in history buffer */
            ret = dlt_buffer_push3(&(daemon->client_ringbuffer), data1, size1, data2, size2, 0, 0);
            if (ret < DLT_RETURN_OK) {
                dlt_daemon_change_state(daemon, DLT_DAEMON_STATE_BUFFER_FULL);
            }
        }
        if (daemon->state == DLT_DAEMON_STATE_BUFFER_FULL) {
            daemon->overflow_counter += 1;
            if (daemon->overflow_counter == 1)
                dlt_vlog(LOG_INFO, "%s: Buffer is full! Messages will be discarded.\n", __func__);

            return DLT_DAEMON_ERROR_BUFFER_FULL;
        }
    } else {
        if ((daemon->overflow_counter > 0) &&
            (daemon_local->client_connections > 0)) {
            sent_message_overflow_cnt++;
            if (sent_message_overflow_cnt >= 2) {
                sent_message_overflow_cnt--;
            }
            else {
                if (dlt_daemon_send_message_overflow(daemon, daemon_local,
                                          verbose) == DLT_DAEMON_ERROR_OK) {
                    dlt_vlog(LOG_WARNING,
                             "%s: %u messages discarded! Now able to send messages to the client.\n",
                             __func__,
                             daemon->overflow_counter);
                    daemon->overflow_counter = 0;
                    sent_message_overflow_cnt--;
                }
            }
        }
    }

    return DLT_DAEMON_ERROR_OK;

}

int dlt_daemon_client_send_message_to_all_client(DltDaemon *daemon,
                                       DltDaemonLocal *daemon_local,
                                       int verbose)
{
    static char text[DLT_DAEMON_TEXTSIZE];
    char * ecu_ptr = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL)) {
        dlt_vlog(LOG_ERR, "%s: invalid arguments\n", __func__);
        return DLT_DAEMON_ERROR_UNKNOWN;
    }

    /* set overwrite ecu id */
    if ((daemon_local->flags.evalue[0]) &&
        (strncmp(daemon_local->msg.headerextra.ecu,
                 DLT_DAEMON_ECU_ID, DLT_ID_SIZE) == 0)) {
        /* Set header extra parameters */
        dlt_set_id(daemon_local->msg.headerextra.ecu, daemon->ecuid);

        /*msg.headerextra.seid = 0; */
        if (dlt_message_set_extraparameters(&(daemon_local->msg), 0)) {
            dlt_vlog(LOG_WARNING,
                     "%s: failed to set message extra parameters.\n", __func__);
            return DLT_DAEMON_ERROR_UNKNOWN;
        }

        /* Correct value of timestamp, this was changed by dlt_message_set_extraparameters() */
        daemon_local->msg.headerextra.tmsp =
                        DLT_BETOH_32(daemon_local->msg.headerextra.tmsp);
    }

    /* prepare storage header */
    if (DLT_IS_HTYP_WEID(daemon_local->msg.standardheader->htyp)) {
        ecu_ptr = daemon_local->msg.headerextra.ecu;
    } else {
        ecu_ptr = daemon->ecuid;
    }

    if (dlt_set_storageheader(daemon_local->msg.storageheader, ecu_ptr)) {
        dlt_vlog(LOG_WARNING,
                 "%s: failed to set storage header with header type: 0x%x\n",
                 __func__, daemon_local->msg.standardheader->htyp);
        return DLT_DAEMON_ERROR_UNKNOWN;
    }

    /* if no filter set or filter is matching display message */
    if (daemon_local->flags.xflag) {
        if (DLT_RETURN_OK !=
            dlt_message_print_hex(&(daemon_local->msg), text,
                                  DLT_DAEMON_TEXTSIZE, verbose))
            dlt_log(LOG_WARNING, "dlt_message_print_hex() failed!\n");
    } else if (daemon_local->flags.aflag) {
        if (DLT_RETURN_OK !=
            dlt_message_print_ascii(&(daemon_local->msg), text,
                                    DLT_DAEMON_TEXTSIZE, verbose))
            dlt_log(LOG_WARNING, "dlt_message_print_ascii() failed!\n");
    } else if (daemon_local->flags.sflag) {
        if (DLT_RETURN_OK !=
            dlt_message_print_header(&(daemon_local->msg), text,
                                     DLT_DAEMON_TEXTSIZE, verbose))
            dlt_log(LOG_WARNING, "dlt_message_print_header() failed!\n");
    }

    /* send message to client or write to log file */
    return dlt_daemon_client_send(DLT_DAEMON_SEND_TO_ALL, daemon, daemon_local,
                daemon_local->msg.headerbuffer, sizeof(DltStorageHeader),
                daemon_local->msg.headerbuffer + sizeof(DltStorageHeader),
                (int) (daemon_local->msg.headersize - sizeof(DltStorageHeader)),
                daemon_local->msg.databuffer, (int) daemon_local->msg.datasize, verbose);

}

int dlt_daemon_client_send_control_message(int sock,
                                           DltDaemon *daemon,
                                           DltDaemonLocal *daemon_local,
                                           DltMessage *msg,
                                           char *apid,
                                           char *ctid,
                                           int verbose)
{
    int ret;
    int32_t len;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == 0) || (msg == 0) || (apid == 0) || (ctid == 0))
        return DLT_DAEMON_ERROR_UNKNOWN;

    /* prepare storage header */
    msg->storageheader = (DltStorageHeader *)msg->headerbuffer;

    if (dlt_set_storageheader(msg->storageheader, daemon->ecuid) == DLT_RETURN_ERROR)
        return DLT_DAEMON_ERROR_UNKNOWN;

    /* prepare standard header */
    msg->standardheader = (DltStandardHeader *)(msg->headerbuffer + sizeof(DltStorageHeader));
    msg->standardheader->htyp = DLT_HTYP_WEID | DLT_HTYP_WTMS | DLT_HTYP_UEH | DLT_HTYP_PROTOCOL_VERSION1;

#if (BYTE_ORDER == BIG_ENDIAN)
    msg->standardheader->htyp = (msg->standardheader->htyp | DLT_HTYP_MSBF);
#endif

    msg->standardheader->mcnt = 0;

    /* Set header extra parameters */
    dlt_set_id(msg->headerextra.ecu, daemon->ecuid);

    /*msg->headerextra.seid = 0; */

    msg->headerextra.tmsp = dlt_uptime();

    dlt_message_set_extraparameters(msg, verbose);

    /* prepare extended header */
    msg->extendedheader =
        (DltExtendedHeader *)(msg->headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader) +
                              DLT_STANDARD_HEADER_EXTRA_SIZE(msg->standardheader->htyp));
    msg->extendedheader->msin = DLT_MSIN_CONTROL_RESPONSE;

    msg->extendedheader->noar = 1; /* number of arguments */

    if (strcmp(apid, "") == 0)
        dlt_set_id(msg->extendedheader->apid, DLT_DAEMON_CTRL_APID);       /* application id */
    else
        dlt_set_id(msg->extendedheader->apid, apid);

    if (strcmp(ctid, "") == 0)
        dlt_set_id(msg->extendedheader->ctid, DLT_DAEMON_CTRL_CTID);       /* context id */
    else
        dlt_set_id(msg->extendedheader->ctid, ctid);

    /* prepare length information */
    msg->headersize = (uint32_t) (sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + sizeof(DltExtendedHeader) +
        DLT_STANDARD_HEADER_EXTRA_SIZE(msg->standardheader->htyp));

    len = (int32_t) (msg->headersize - sizeof(DltStorageHeader) + msg->datasize);

    if (len > UINT16_MAX) {
        dlt_log(LOG_WARNING, "Huge control message discarded!\n");
        return DLT_DAEMON_ERROR_UNKNOWN;
    }

    msg->standardheader->len = DLT_HTOBE_16(((uint16_t)len));

    if ((ret =
             dlt_daemon_client_send(sock, daemon, daemon_local, msg->headerbuffer, sizeof(DltStorageHeader),
                                    msg->headerbuffer + sizeof(DltStorageHeader),
                                    (int) (msg->headersize - sizeof(DltStorageHeader)),
                                    msg->databuffer, (int) msg->datasize, verbose))) {
        dlt_log(LOG_DEBUG, "dlt_daemon_control_send_control_message: DLT message send to all failed!.\n");
        return ret;
    }

    return DLT_DAEMON_ERROR_OK;
}

int dlt_daemon_client_process_control(int sock,
                                      DltDaemon *daemon,
                                      DltDaemonLocal *daemon_local,
                                      DltMessage *msg,
                                      int verbose)
{
    uint32_t id, id_tmp = 0;
    DltStandardHeaderExtra extra;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (msg == NULL))
        return -1;

    if (msg->datasize < (int32_t)sizeof(uint32_t))
        return -1;

    extra = msg->headerextra;

    /* check if the message needs to be forwarded */
    if (daemon_local->flags.gatewayMode == 1) {
        if (strncmp(daemon_local->flags.evalue, extra.ecu, DLT_ID_SIZE) != 0)
            return dlt_gateway_forward_control_message(&daemon_local->pGateway,
                                                       daemon_local,
                                                       msg,
                                                       extra.ecu,
                                                       verbose);
    }

    id_tmp = *((uint32_t *)(msg->databuffer));
    id = DLT_ENDIAN_GET_32(msg->standardheader->htyp, id_tmp);

    if ((id > DLT_SERVICE_ID) && (id < DLT_SERVICE_ID_CALLSW_CINJECTION)) {
        /* Control message handling */
        switch (id) {
        case DLT_SERVICE_ID_SET_LOG_LEVEL:
        {
            dlt_daemon_control_set_log_level(sock, daemon, daemon_local, msg, verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_TRACE_STATUS:
        {
            dlt_daemon_control_set_trace_status(sock, daemon, daemon_local, msg, verbose);
            break;
        }
        case DLT_SERVICE_ID_GET_LOG_INFO:
        {
            dlt_daemon_control_get_log_info(sock, daemon, daemon_local, msg, verbose);
            break;
        }
        case DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL:
        {
            dlt_daemon_control_get_default_log_level(sock, daemon, daemon_local, verbose);
            break;
        }
        case DLT_SERVICE_ID_STORE_CONFIG:
        {
            if (dlt_daemon_applications_save(daemon, daemon->runtime_application_cfg, verbose) == 0) {
                if (dlt_daemon_contexts_save(daemon, daemon->runtime_context_cfg, verbose) == 0) {
                    dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_OK,
                                                        verbose);
                }
                else {
                    /* Delete saved files */
                    dlt_daemon_control_reset_to_factory_default(daemon,
                                                                daemon->runtime_application_cfg,
                                                                daemon->runtime_context_cfg,
                                                                daemon_local->flags.contextLogLevel,
                                                                daemon_local->flags.contextTraceStatus,
                                                                daemon_local->flags.enforceContextLLAndTS,
                                                                verbose);
                    dlt_daemon_control_service_response(sock,
                                                        daemon,
                                                        daemon_local,
                                                        id,
                                                        DLT_SERVICE_RESPONSE_ERROR,
                                                        verbose);
                }
            }
            else {
                dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR,
                                                    verbose);
            }

            break;
        }
        case DLT_SERVICE_ID_RESET_TO_FACTORY_DEFAULT:
        {
            dlt_daemon_control_reset_to_factory_default(daemon,
                                                        daemon->runtime_application_cfg,
                                                        daemon->runtime_context_cfg,
                                                        daemon_local->flags.contextLogLevel,
                                                        daemon_local->flags.contextTraceStatus,
                                                        daemon_local->flags.enforceContextLLAndTS,
                                                        verbose);
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_OK, verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_COM_INTERFACE_STATUS:
        {
            dlt_daemon_control_service_response(sock,
                                                daemon,
                                                daemon_local,
                                                id,
                                                DLT_SERVICE_RESPONSE_NOT_SUPPORTED,
                                                verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_COM_INTERFACE_MAX_BANDWIDTH:
        {
            dlt_daemon_control_service_response(sock,
                                                daemon,
                                                daemon_local,
                                                id,
                                                DLT_SERVICE_RESPONSE_NOT_SUPPORTED,
                                                verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_VERBOSE_MODE:
        {
            dlt_daemon_control_service_response(sock,
                                                daemon,
                                                daemon_local,
                                                id,
                                                DLT_SERVICE_RESPONSE_NOT_SUPPORTED,
                                                verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_MESSAGE_FILTERING:
        {
            dlt_daemon_control_service_response(sock,
                                                daemon,
                                                daemon_local,
                                                id,
                                                DLT_SERVICE_RESPONSE_NOT_SUPPORTED,
                                                verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_TIMING_PACKETS:
        {
            dlt_daemon_control_set_timing_packets(sock, daemon, daemon_local, msg, verbose);
            break;
        }
        case DLT_SERVICE_ID_GET_LOCAL_TIME:
        {
            /* Send response with valid timestamp (TMSP) field */
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_OK, verbose);
            break;
        }
        case DLT_SERVICE_ID_USE_ECU_ID:
        {
            dlt_daemon_control_service_response(sock,
                                                daemon,
                                                daemon_local,
                                                id,
                                                DLT_SERVICE_RESPONSE_NOT_SUPPORTED,
                                                verbose);
            break;
        }
        case DLT_SERVICE_ID_USE_SESSION_ID:
        {
            dlt_daemon_control_service_response(sock,
                                                daemon,
                                                daemon_local,
                                                id,
                                                DLT_SERVICE_RESPONSE_NOT_SUPPORTED,
                                                verbose);
            break;
        }
        case DLT_SERVICE_ID_USE_TIMESTAMP:
        {
            dlt_daemon_control_service_response(sock,
                                                daemon,
                                                daemon_local,
                                                id,
                                                DLT_SERVICE_RESPONSE_NOT_SUPPORTED,
                                                verbose);
            break;
        }
        case DLT_SERVICE_ID_USE_EXTENDED_HEADER:
        {
            dlt_daemon_control_service_response(sock,
                                                daemon,
                                                daemon_local,
                                                id,
                                                DLT_SERVICE_RESPONSE_NOT_SUPPORTED,
                                                verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_DEFAULT_LOG_LEVEL:
        {
            dlt_daemon_control_set_default_log_level(sock, daemon, daemon_local, msg, verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_DEFAULT_TRACE_STATUS:
        {
            dlt_daemon_control_set_default_trace_status(sock, daemon, daemon_local, msg, verbose);
            break;
        }
        case DLT_SERVICE_ID_GET_SOFTWARE_VERSION:
        {
            dlt_daemon_control_get_software_version(sock, daemon, daemon_local, verbose);
            break;
        }
        case DLT_SERVICE_ID_MESSAGE_BUFFER_OVERFLOW:
        {
            dlt_daemon_control_message_buffer_overflow(sock, daemon, daemon_local, daemon->overflow_counter, "",
                                                       verbose);
            break;
        }
        case DLT_SERVICE_ID_OFFLINE_LOGSTORAGE:
        {
            dlt_daemon_control_service_logstorage(sock, daemon, daemon_local, msg, verbose);
            break;
        }
        case DLT_SERVICE_ID_PASSIVE_NODE_CONNECT:
        {
            dlt_daemon_control_passive_node_connect(sock,
                                                    daemon,
                                                    daemon_local,
                                                    msg,
                                                    verbose);
            break;
        }
        case DLT_SERVICE_ID_PASSIVE_NODE_CONNECTION_STATUS:
        {
            dlt_daemon_control_passive_node_connect_status(sock,
                                                           daemon,
                                                           daemon_local,
                                                           verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_ALL_LOG_LEVEL:
        {
            dlt_daemon_control_set_all_log_level(sock, daemon, daemon_local, msg, verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_ALL_TRACE_STATUS:
        {
            dlt_daemon_control_set_all_trace_status(sock, daemon, daemon_local, msg, verbose);
            break;
        }
        default:
        {
            dlt_daemon_control_service_response(sock,
                                                daemon,
                                                daemon_local,
                                                id,
                                                DLT_SERVICE_RESPONSE_NOT_SUPPORTED,
                                                verbose);
            break;
        }
        }
    }
    else {
        /* Injection handling */
        dlt_daemon_control_callsw_cinjection(sock, daemon, daemon_local, msg, verbose);
    }

    return 0;
}

void dlt_daemon_control_get_software_version(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    DltMessage msg;
    uint32_t len;
    DltServiceGetSoftwareVersionResponse *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == 0)
        return;

    /* initialise new message */
    if (dlt_message_init(&msg, 0) == DLT_RETURN_ERROR) {
        dlt_daemon_control_service_response(sock,
                                            daemon,
                                            daemon_local,
                                            DLT_SERVICE_ID_GET_SOFTWARE_VERSION,
                                            DLT_SERVICE_RESPONSE_ERROR,
                                            verbose);
        return;
    }

    /* prepare payload of data */
    len = (uint32_t) strlen(daemon->ECUVersionString);

    /* msg.datasize = sizeof(serviceID) + sizeof(status) + sizeof(length) + len */
    msg.datasize = (uint32_t) (sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint32_t) + len);

    if (msg.databuffer && (msg.databuffersize < msg.datasize)) {
        free(msg.databuffer);
        msg.databuffer = 0;
    }

    if (msg.databuffer == 0) {
        msg.databuffer = (uint8_t *)malloc(msg.datasize);
        msg.databuffersize = msg.datasize;
    }

    if (msg.databuffer == 0) {
        dlt_daemon_control_service_response(sock,
                                            daemon,
                                            daemon_local,
                                            DLT_SERVICE_ID_GET_SOFTWARE_VERSION,
                                            DLT_SERVICE_RESPONSE_ERROR,
                                            verbose);
        return;
    }

    resp = (DltServiceGetSoftwareVersionResponse *)msg.databuffer;
    resp->service_id = DLT_SERVICE_ID_GET_SOFTWARE_VERSION;
    resp->status = DLT_SERVICE_RESPONSE_OK;
    resp->length = len;
    memcpy(msg.databuffer + msg.datasize - len, daemon->ECUVersionString, len);

    /* send message */
    dlt_daemon_client_send_control_message(sock, daemon, daemon_local, &msg, "", "", verbose);

    /* free message */
    dlt_message_free(&msg, 0);
}

void dlt_daemon_control_get_default_log_level(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    DltMessage msg;
    DltServiceGetDefaultLogLevelResponse *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == 0)
        return;

    /* initialise new message */
    if (dlt_message_init(&msg, 0) == DLT_RETURN_ERROR) {
        dlt_daemon_control_service_response(sock,
                                            daemon,
                                            daemon_local,
                                            DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL,
                                            DLT_SERVICE_RESPONSE_ERROR,
                                            verbose);
        return;
    }

    msg.datasize = sizeof(DltServiceGetDefaultLogLevelResponse);

    if (msg.databuffer && (msg.databuffersize < msg.datasize)) {
        free(msg.databuffer);
        msg.databuffer = 0;
    }

    if (msg.databuffer == 0) {
        msg.databuffer = (uint8_t *)malloc(msg.datasize);
        msg.databuffersize = msg.datasize;
    }

    if (msg.databuffer == 0) {
        dlt_daemon_control_service_response(sock,
                                            daemon,
                                            daemon_local,
                                            DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL,
                                            DLT_SERVICE_RESPONSE_ERROR,
                                            verbose);
        return;
    }

    resp = (DltServiceGetDefaultLogLevelResponse *)msg.databuffer;
    resp->service_id = DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL;
    resp->status = DLT_SERVICE_RESPONSE_OK;
    resp->log_level = (uint8_t) daemon->default_log_level;

    /* send message */
    dlt_daemon_client_send_control_message(sock, daemon, daemon_local, &msg, "", "", verbose);

    /* free message */
    dlt_message_free(&msg, 0);
}

void dlt_daemon_control_get_log_info(int sock,
                                     DltDaemon *daemon,
                                     DltDaemonLocal *daemon_local,
                                     DltMessage *msg,
                                     int verbose)
{
    DltServiceGetLogInfoRequest *req;
    DltMessage resp;
    DltDaemonContext *context = 0;
    DltDaemonApplication *application = 0;

    int num_applications = 0, num_contexts = 0;
    uint16_t count_app_ids = 0, count_con_ids = 0;

#if (DLT_DEBUG_GETLOGINFO == 1)
    char buf[255];
#endif

    int32_t i, j;
    size_t offset = 0;
    char *apid = 0;
    int8_t ll, ts;
    uint16_t len;
    int8_t value;
    size_t sizecont = 0;
    int offset_base;

    uint32_t sid;

    DltDaemonRegisteredUsers *user_list = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (msg == NULL) || (msg->databuffer == NULL))
        return;

    if (dlt_check_rcv_data_size(msg->datasize, sizeof(DltServiceGetLogInfoRequest)) < 0)
        return;

    user_list = dlt_daemon_find_users_list(daemon, daemon->ecuid, verbose);

    if (user_list == NULL)
        return;

    /* prepare pointer to message request */
    req = (DltServiceGetLogInfoRequest *)(msg->databuffer);

    /* initialise new message */
    if (dlt_message_init(&resp, 0) == DLT_RETURN_ERROR) {
        dlt_daemon_control_service_response(sock,
                                            daemon,
                                            daemon_local,
                                            DLT_SERVICE_ID_GET_LOG_INFO,
                                            DLT_SERVICE_RESPONSE_ERROR,
                                            verbose);
        return;
    }

    /* check request */
    if ((req->options < 3) || (req->options > 7)) {
        dlt_daemon_control_service_response(sock,
                                            daemon,
                                            daemon_local,
                                            DLT_SERVICE_ID_GET_LOG_INFO,
                                            DLT_SERVICE_RESPONSE_ERROR,
                                            verbose);
        return;
    }

    if (req->apid[0] != '\0') {
        application = dlt_daemon_application_find(daemon,
                                                  req->apid,
                                                  daemon->ecuid,
                                                  verbose);

        if (application) {
            num_applications = 1;

            if (req->ctid[0] != '\0') {
                context = dlt_daemon_context_find(daemon,
                                                  req->apid,
                                                  req->ctid,
                                                  daemon->ecuid,
                                                  verbose);

                num_contexts = ((context) ? 1 : 0);
            }
            else {
                num_contexts = application->num_contexts;
            }
        }
        else {
            num_applications = 0;
            num_contexts = 0;
        }
    }
    else {
        /* Request all applications and contexts */
        num_applications = user_list->num_applications;
        num_contexts = user_list->num_contexts;
    }

    /* prepare payload of data */

    /* Calculate maximum size for a response */
    resp.datasize = sizeof(uint32_t) /* SID */ + sizeof(int8_t) /* status*/ + sizeof(ID4) /* DLT_DAEMON_REMO_STRING */;

    sizecont = sizeof(uint32_t) /* context_id */;

    /* Add additional size for response of Mode 4, 6, 7 */
    if ((req->options == 4) || (req->options == 6) || (req->options == 7))
        sizecont += sizeof(int8_t); /* log level */

    /* Add additional size for response of Mode 5, 6, 7 */
    if ((req->options == 5) || (req->options == 6) || (req->options == 7))
        sizecont += sizeof(int8_t); /* trace status */

    resp.datasize += (uint32_t) (((uint32_t) num_applications * (sizeof(uint32_t) /* app_id */ + sizeof(uint16_t) /* count_con_ids */)) +
        ((size_t) num_contexts * sizecont));

    resp.datasize += (uint32_t) sizeof(uint16_t) /* count_app_ids */;

    /* Add additional size for response of Mode 7 */
    if (req->options == 7) {
        if (req->apid[0] != '\0') {
            if (req->ctid[0] != '\0') {
                /* One application, one context */
                /* context = dlt_daemon_context_find(daemon, req->apid, req->ctid, verbose); */
                if (context) {
                    resp.datasize += (uint32_t) sizeof(uint16_t) /* len_context_description */;

                    if (context->context_description != 0)
                        resp.datasize += (uint32_t) strlen(context->context_description); /* context_description */
                }
            }
            else
            /* One application, all contexts */
            if ((user_list->applications) && (application)) {
                /* Calculate start offset within contexts[] */
                offset_base = 0;

                for (i = 0; i < (application - (user_list->applications)); i++)
                    offset_base += user_list->applications[i].num_contexts;

                /* Iterate over all contexts belonging to this application */
                for (j = 0; j < application->num_contexts; j++) {

                    context = &(user_list->contexts[offset_base + j]);

                    if (context) {
                        resp.datasize += (uint32_t) sizeof(uint16_t) /* len_context_description */;

                        if (context->context_description != 0)
                            resp.datasize += (uint32_t) strlen(context->context_description);   /* context_description */
                    }
                }
            }

            /* Space for application description */
            if (application) {
                resp.datasize += (uint32_t) sizeof(uint16_t) /* len_app_description */;

                if (application->application_description != 0)
                    resp.datasize += (uint32_t) strlen(application->application_description); /* app_description */
            }
        }
        else {
            /* All applications, all contexts */
            for (i = 0; i < user_list->num_contexts; i++) {
                resp.datasize += (uint32_t) sizeof(uint16_t) /* len_context_description */;

                if (user_list->contexts[i].context_description != 0)
                    resp.datasize +=
                        (uint32_t) strlen(user_list->contexts[i].context_description);
            }

            for (i = 0; i < user_list->num_applications; i++) {
                resp.datasize += (uint32_t) sizeof(uint16_t) /* len_app_description */;

                if (user_list->applications[i].application_description != 0)
                    resp.datasize += (uint32_t) strlen(user_list->applications[i].application_description); /* app_description */
            }
        }
    }

    if (verbose)
        dlt_vlog(LOG_DEBUG,
                 "Allocate %u bytes for response msg databuffer\n",
                 resp.datasize);

    /* Allocate buffer for response message */
    resp.databuffer = (uint8_t *)malloc(resp.datasize);
    resp.databuffersize = resp.datasize;

    if (resp.databuffer == 0) {
        dlt_daemon_control_service_response(sock,
                                            daemon,
                                            daemon_local,
                                            DLT_SERVICE_ID_GET_LOG_INFO,
                                            DLT_SERVICE_RESPONSE_ERROR,
                                            verbose);
        return;
    }

    memset(resp.databuffer, 0, resp.datasize);
    /* Preparation finished */

    /* Prepare response */
    sid = DLT_SERVICE_ID_GET_LOG_INFO;
    memcpy(resp.databuffer, &sid, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    value = (int8_t) (((num_applications != 0) && (num_contexts != 0)) ? req->options : 8); /* 8 = no matching context found */

    memcpy(resp.databuffer + offset, &value, sizeof(int8_t));
    offset += sizeof(int8_t);

    count_app_ids = (uint16_t) num_applications;

    if (count_app_ids != 0) {
        memcpy(resp.databuffer + offset, &count_app_ids, sizeof(uint16_t));
        offset += sizeof(uint16_t);

#if (DLT_DEBUG_GETLOGINFO == 1)
        dlt_vlog(LOG_DEBUG, "#apid: %d \n", count_app_ids);
#endif

        for (i = 0; i < count_app_ids; i++) {
            if (req->apid[0] != '\0') {
                apid = req->apid;
            }
            else {
                if (user_list->applications)
                    apid = user_list->applications[i].apid;
                else
                    /* This should never occur! */
                    apid = 0;
            }

            application = dlt_daemon_application_find(daemon,
                                                      apid,
                                                      daemon->ecuid,
                                                      verbose);

            if ((user_list->applications) && (application)) {
                /* Calculate start offset within contexts[] */
                offset_base = 0;

                for (j = 0; j < (application - (user_list->applications)); j++)
                    offset_base += user_list->applications[j].num_contexts;

                dlt_set_id((char *)(resp.databuffer + offset), apid);
                offset += sizeof(ID4);

#if (DLT_DEBUG_GETLOGINFO == 1)
                dlt_print_id(buf, apid);
                dlt_vlog(LOG_DEBUG, "apid: %s\n", buf);
#endif

                if (req->apid[0] != '\0')
                    count_con_ids = (uint16_t) num_contexts;
                else
                    count_con_ids = (uint16_t) application->num_contexts;

                memcpy(resp.databuffer + offset, &count_con_ids, sizeof(uint16_t));
                offset += sizeof(uint16_t);

#if (DLT_DEBUG_GETLOGINFO == 1)
                dlt_vlog(LOG_DEBUG, "#ctid: %d \n", count_con_ids);
#endif

                for (j = 0; j < count_con_ids; j++) {
#if (DLT_DEBUG_GETLOGINFO == 1)
                    dlt_vlog(LOG_DEBUG, "j: %d \n", j);
#endif

                    if (!((count_con_ids == 1) && (req->apid[0] != '\0') &&
                          (req->ctid[0] != '\0')))
                        context = &(user_list->contexts[offset_base + j]);

                    /* else: context was already searched and found
                     *       (one application (found) with one context (found))*/

                    if ((context) &&
                        ((req->ctid[0] == '\0') || ((req->ctid[0] != '\0') &&
                                                    (memcmp(context->ctid, req->ctid, DLT_ID_SIZE) == 0)))
                        ) {
                        dlt_set_id((char *)(resp.databuffer + offset), context->ctid);
                        offset += sizeof(ID4);

#if (DLT_DEBUG_GETLOGINFO == 1)
                        dlt_print_id(buf, context->ctid);
                        dlt_vlog(LOG_DEBUG, "ctid: %s \n", buf);
#endif

                        /* Mode 4, 6, 7 */
                        if ((req->options == 4) || (req->options == 6) || (req->options == 7)) {
                            ll = context->log_level;
                            memcpy(resp.databuffer + offset, &ll, sizeof(int8_t));
                            offset += sizeof(int8_t);
                        }

                        /* Mode 5, 6, 7 */
                        if ((req->options == 5) || (req->options == 6) || (req->options == 7)) {
                            ts = context->trace_status;
                            memcpy(resp.databuffer + offset, &ts, sizeof(int8_t));
                            offset += sizeof(int8_t);
                        }

                        /* Mode 7 */
                        if (req->options == 7) {
                            if (context->context_description) {
                                len = (uint16_t) strlen(context->context_description);
                                memcpy(resp.databuffer + offset, &len, sizeof(uint16_t));
                                offset += sizeof(uint16_t);
                                memcpy(resp.databuffer + offset, context->context_description,
                                       strlen(context->context_description));
                                offset += strlen(context->context_description);
                            }
                            else {
                                len = 0;
                                memcpy(resp.databuffer + offset, &len, sizeof(uint16_t));
                                offset += sizeof(uint16_t);
                            }
                        }

#if (DLT_DEBUG_GETLOGINFO == 1)
                        dlt_vlog(LOG_DEBUG, "ll=%d ts=%d \n", (int32_t)ll,
                                 (int32_t)ts);
#endif
                    }

#if (DLT_DEBUG_GETLOGINFO == 1)
                    dlt_log(LOG_DEBUG, "\n");
#endif
                }

                /* Mode 7 */
                if (req->options == 7) {
                    if (application->application_description) {
                        len = (uint16_t) strlen(application->application_description);
                        memcpy(resp.databuffer + offset, &len, sizeof(uint16_t));
                        offset += sizeof(uint16_t);
                        memcpy(resp.databuffer + offset, application->application_description,
                               strlen(application->application_description));
                        offset += strlen(application->application_description);
                    }
                    else {
                        len = 0;
                        memcpy(resp.databuffer + offset, &len, sizeof(uint16_t));
                        offset += sizeof(uint16_t);
                    }
                }
            } /* if (application) */

        } /* for (i=0;i<count_app_ids;i++) */

    } /* if (count_app_ids!=0) */

    dlt_set_id((char *)(resp.databuffer + offset), DLT_DAEMON_REMO_STRING);

    /* send message */
    dlt_daemon_client_send_control_message(sock, daemon, daemon_local, &resp, "", "", verbose);

    /* free message */
    dlt_message_free(&resp, 0);
}

int dlt_daemon_control_message_buffer_overflow(int sock,
                                               DltDaemon *daemon,
                                               DltDaemonLocal *daemon_local,
                                               unsigned int overflow_counter,
                                               char *apid,
                                               int verbose)
{
    int ret;
    DltMessage msg;
    DltServiceMessageBufferOverflowResponse *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == 0)
        return DLT_DAEMON_ERROR_UNKNOWN;

    /* initialise new message */
    if (dlt_message_init(&msg, 0) == DLT_RETURN_ERROR) {
        dlt_daemon_control_service_response(sock,
                                            daemon,
                                            daemon_local,
                                            DLT_SERVICE_ID_MESSAGE_BUFFER_OVERFLOW,
                                            DLT_SERVICE_RESPONSE_ERROR,
                                            verbose);
        return DLT_DAEMON_ERROR_UNKNOWN;
    }

    /* prepare payload of data */
    msg.datasize = sizeof(DltServiceMessageBufferOverflowResponse);

    if (msg.databuffer && (msg.databuffersize < msg.datasize)) {
        free(msg.databuffer);
        msg.databuffer = 0;
    }

    if (msg.databuffer == 0) {
        msg.databuffer = (uint8_t *)malloc(msg.datasize);
        msg.databuffersize = msg.datasize;
    }

    if (msg.databuffer == 0)
        return DLT_DAEMON_ERROR_UNKNOWN;

    resp = (DltServiceMessageBufferOverflowResponse *)msg.databuffer;
    resp->service_id = DLT_SERVICE_ID_MESSAGE_BUFFER_OVERFLOW;
    resp->status = DLT_SERVICE_RESPONSE_OK;
    resp->overflow = DLT_MESSAGE_BUFFER_OVERFLOW;
    resp->overflow_counter = overflow_counter;

    /* send message */
    if ((ret = dlt_daemon_client_send_control_message(sock, daemon, daemon_local, &msg, apid, "", verbose))) {
        dlt_message_free(&msg, 0);
        return ret;
    }

    /* free message */
    dlt_message_free(&msg, 0);

    return DLT_DAEMON_ERROR_OK;
}

void dlt_daemon_control_service_response(int sock,
                                         DltDaemon *daemon,
                                         DltDaemonLocal *daemon_local,
                                         uint32_t service_id,
                                         int8_t status,
                                         int verbose)
{
    DltMessage msg;
    DltServiceResponse *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == 0)
        return;

    /* initialise new message */
    if (dlt_message_init(&msg, 0) == DLT_RETURN_ERROR)
        return;

    /* prepare payload of data */
    msg.datasize = sizeof(DltServiceResponse);

    if (msg.databuffer && (msg.databuffersize < msg.datasize)) {
        free(msg.databuffer);
        msg.databuffer = 0;
    }

    if (msg.databuffer == 0) {
        msg.databuffer = (uint8_t *)malloc(msg.datasize);
        msg.databuffersize = msg.datasize;
    }

    if (msg.databuffer == 0)
        return;

    resp = (DltServiceResponse *)msg.databuffer;
    resp->service_id = service_id;
    resp->status = (uint8_t) status;

    /* send message */
    dlt_daemon_client_send_control_message(sock, daemon, daemon_local, &msg, "", "", verbose);

    /* free message */
    dlt_message_free(&msg, 0);
}

int dlt_daemon_control_message_unregister_context(int sock,
                                                  DltDaemon *daemon,
                                                  DltDaemonLocal *daemon_local,
                                                  char *apid,
                                                  char *ctid,
                                                  char *comid,
                                                  int verbose)
{
    DltMessage msg;
    DltServiceUnregisterContext *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == 0)
        return -1;

    /* initialise new message */
    if (dlt_message_init(&msg, 0) == DLT_RETURN_ERROR)
        return -1;

    /* prepare payload of data */
    msg.datasize = sizeof(DltServiceUnregisterContext);

    if (msg.databuffer && (msg.databuffersize < msg.datasize)) {
        free(msg.databuffer);
        msg.databuffer = 0;
    }

    if (msg.databuffer == 0) {
        msg.databuffer = (uint8_t *)malloc(msg.datasize);
        msg.databuffersize = msg.datasize;
    }

    if (msg.databuffer == 0)
        return -1;

    resp = (DltServiceUnregisterContext *)msg.databuffer;
    resp->service_id = DLT_SERVICE_ID_UNREGISTER_CONTEXT;
    resp->status = DLT_SERVICE_RESPONSE_OK;
    dlt_set_id(resp->apid, apid);
    dlt_set_id(resp->ctid, ctid);
    dlt_set_id(resp->comid, comid);

    /* send message */
    if (dlt_daemon_client_send_control_message(sock, daemon, daemon_local, &msg, "", "", verbose)) {
        dlt_message_free(&msg, 0);
        return -1;
    }

    /* free message */
    dlt_message_free(&msg, 0);

    return 0;
}

int dlt_daemon_control_message_connection_info(int sock,
                                               DltDaemon *daemon,
                                               DltDaemonLocal *daemon_local,
                                               uint8_t state,
                                               char *comid,
                                               int verbose)
{
    DltMessage msg;
    DltServiceConnectionInfo *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == 0)
        return -1;

    /* initialise new message */
    if (dlt_message_init(&msg, 0) == DLT_RETURN_ERROR)
        return -1;

    /* prepare payload of data */
    msg.datasize = sizeof(DltServiceConnectionInfo);

    if (msg.databuffer && (msg.databuffersize < msg.datasize)) {
        free(msg.databuffer);
        msg.databuffer = 0;
    }

    if (msg.databuffer == 0) {
        msg.databuffer = (uint8_t *)malloc(msg.datasize);
        msg.databuffersize = msg.datasize;
    }

    if (msg.databuffer == 0)
        return -1;

    resp = (DltServiceConnectionInfo *)msg.databuffer;
    resp->service_id = DLT_SERVICE_ID_CONNECTION_INFO;
    resp->status = DLT_SERVICE_RESPONSE_OK;
    resp->state = state;
    dlt_set_id(resp->comid, comid);

    /* send message */
    if (dlt_daemon_client_send_control_message(sock, daemon, daemon_local, &msg, "", "", verbose)) {
        dlt_message_free(&msg, 0);
        return -1;
    }

    /* free message */
    dlt_message_free(&msg, 0);

    return 0;
}

int dlt_daemon_control_message_timezone(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    DltMessage msg;
    DltServiceTimezone *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == 0)
        return -1;

    /* initialise new message */
    if (dlt_message_init(&msg, 0) == DLT_RETURN_ERROR)
        return -1;

    /* prepare payload of data */
    msg.datasize = sizeof(DltServiceTimezone);

    if (msg.databuffer && (msg.databuffersize < msg.datasize)) {
        free(msg.databuffer);
        msg.databuffer = 0;
    }

    if (msg.databuffer == 0) {
        msg.databuffer = (uint8_t *)malloc(msg.datasize);
        msg.databuffersize = msg.datasize;
    }

    if (msg.databuffer == 0)
        return -1;

    resp = (DltServiceTimezone *)msg.databuffer;
    resp->service_id = DLT_SERVICE_ID_TIMEZONE;
    resp->status = DLT_SERVICE_RESPONSE_OK;

    time_t t = time(NULL);
    struct tm lt;
    tzset();
    localtime_r(&t, &lt);
#if !defined(__CYGWIN__)
    resp->timezone = (int32_t)lt.tm_gmtoff;
#endif
    resp->isdst = (uint8_t)lt.tm_isdst;

    /* send message */
    if (dlt_daemon_client_send_control_message(sock, daemon, daemon_local, &msg, "", "", verbose)) {
        dlt_message_free(&msg, 0);
        return -1;
    }

    /* free message */
    dlt_message_free(&msg, 0);

    return 0;
}

int dlt_daemon_control_message_marker(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    DltMessage msg;
    DltServiceMarker *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == 0)
        return -1;

    /* initialise new message */
    if (dlt_message_init(&msg, 0) == DLT_RETURN_ERROR)
        return -1;

    /* prepare payload of data */
    msg.datasize = sizeof(DltServiceMarker);

    if (msg.databuffer && (msg.databuffersize < msg.datasize)) {
        free(msg.databuffer);
        msg.databuffer = 0;
    }

    if (msg.databuffer == 0) {
        msg.databuffer = (uint8_t *)malloc(msg.datasize);
        msg.databuffersize = msg.datasize;
    }

    if (msg.databuffer == 0)
        return -1;

    resp = (DltServiceMarker *)msg.databuffer;
    resp->service_id = DLT_SERVICE_ID_MARKER;
    resp->status = DLT_SERVICE_RESPONSE_OK;

    /* send message */
    if (dlt_daemon_client_send_control_message(sock, daemon, daemon_local, &msg, "", "", verbose)) {
        dlt_message_free(&msg, 0);
        return -1;
    }

    /* free message */
    dlt_message_free(&msg, 0);

    return 0;
}

void dlt_daemon_control_callsw_cinjection(int sock,
                                          DltDaemon *daemon,
                                          DltDaemonLocal *daemon_local,
                                          DltMessage *msg,
                                          int verbose)
{
    char apid[DLT_ID_SIZE], ctid[DLT_ID_SIZE];
    uint32_t id = 0, id_tmp = 0;
    uint8_t *ptr;
    DltDaemonContext *context;
    uint32_t data_length_inject = 0;
    uint32_t data_length_inject_tmp = 0;

    int32_t datalength;

    DltUserHeader userheader;
    DltUserControlMsgInjection usercontext;
    uint8_t *userbuffer;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL) || (msg == NULL) || (msg->databuffer == NULL))
        return;

    datalength = (int32_t) msg->datasize;
    ptr = msg->databuffer;

    DLT_MSG_READ_VALUE(id_tmp, ptr, datalength, uint32_t); /* Get service id */
    id = DLT_ENDIAN_GET_32(msg->standardheader->htyp, id_tmp);

    /* injectionMode is disabled */
    if (daemon_local->flags.injectionMode == 0) {
        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_PERM_DENIED, verbose);
        return;
    }

    /* id is always less than DLT_DAEMON_INJECTION_MAX since its type is uinit32_t */
    if (id >= DLT_DAEMON_INJECTION_MIN) {
        /* This a a real SW-C injection call */
        data_length_inject = 0;
        data_length_inject_tmp = 0;

        DLT_MSG_READ_VALUE(data_length_inject_tmp, ptr, datalength, uint32_t); /* Get data length */
        data_length_inject = DLT_ENDIAN_GET_32(msg->standardheader->htyp, data_length_inject_tmp);

        /* Get context handle for apid, ctid (and seid) */
        /* Warning: seid is ignored in this implementation! */
        if (DLT_IS_HTYP_UEH(msg->standardheader->htyp)) {
            dlt_set_id(apid, msg->extendedheader->apid);
            dlt_set_id(ctid, msg->extendedheader->ctid);
        }
        else {
            /* No extended header, and therefore no apid and ctid available */
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR, verbose);
            return;
        }

        /* At this point, apid and ctid is available */
        context = dlt_daemon_context_find(daemon,
                                          apid,
                                          ctid,
                                          daemon->ecuid,
                                          verbose);

        if (context == 0) {
            /* dlt_log(LOG_INFO,"No context found!\n"); */
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR, verbose);
            return;
        }

        /* Send user message to handle, specified in context */
        if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_INJECTION) < DLT_RETURN_OK) {
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR, verbose);
            return;
        }

        usercontext.log_level_pos = context->log_level_pos;

        if (data_length_inject > (uint32_t) msg->databuffersize) {
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR, verbose);
            return;
        }

        userbuffer = malloc(data_length_inject);

        if (userbuffer == 0) {
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR, verbose);
            return;
        }

        usercontext.data_length_inject = (uint32_t) data_length_inject;
        usercontext.service_id = id;

        memcpy(userbuffer, ptr, (size_t) data_length_inject);  /* Copy received injection to send buffer */

        /* write to FIFO */
        DltReturnValue ret =
            dlt_user_log_out3_with_timeout(context->user_handle, &(userheader), sizeof(DltUserHeader),
                              &(usercontext), sizeof(DltUserControlMsgInjection),
                              userbuffer, (size_t) data_length_inject);

        if (ret < DLT_RETURN_OK) {
            if (ret == DLT_RETURN_PIPE_ERROR) {
                /* Close connection */
                close(context->user_handle);
                context->user_handle = DLT_FD_INIT;
            }

            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR, verbose);
        }
        else {
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_OK, verbose);
        }

        free(userbuffer);
        userbuffer = 0;

    }
    else {
        /* Invalid ID */
        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,
                                            verbose);
    }
}

void dlt_daemon_send_log_level(int sock,
                               DltDaemon *daemon,
                               DltDaemonLocal *daemon_local,
                               DltDaemonContext *context,
                               int8_t loglevel,
                               int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    int32_t id = DLT_SERVICE_ID_SET_LOG_LEVEL;
    int8_t old_log_level = 0;

    old_log_level = context->log_level;
    context->log_level = loglevel; /* No endianess conversion necessary*/

    if ((context->user_handle >= DLT_FD_MINIMUM) &&
        (dlt_daemon_user_send_log_level(daemon, context, verbose) == 0)) {
        dlt_daemon_control_service_response(sock, daemon, daemon_local, (uint32_t) id, DLT_SERVICE_RESPONSE_OK, verbose);
    }
    else {
        dlt_log(LOG_ERR, "Log level could not be sent!\n");
        context->log_level = old_log_level;
        dlt_daemon_control_service_response(sock, daemon, daemon_local, (uint32_t) id, DLT_SERVICE_RESPONSE_ERROR, verbose);
    }
}

void dlt_daemon_find_multiple_context_and_send_log_level(int sock,
                                                         DltDaemon *daemon,
                                                         DltDaemonLocal *daemon_local,
                                                         int8_t app_flag,
                                                         char *str,
                                                         int8_t len,
                                                         int8_t loglevel,
                                                         int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    int count = 0;
    DltDaemonContext *context = NULL;
    char src_str[DLT_ID_SIZE + 1] = { 0 };
    int ret = 0;
    DltDaemonRegisteredUsers *user_list = NULL;

    if (daemon == 0) {
        dlt_vlog(LOG_ERR, "%s: Invalid parameters\n", __func__);
        return;
    }

    user_list = dlt_daemon_find_users_list(daemon, daemon->ecuid, verbose);

    if (user_list == NULL)
        return;

    for (count = 0; count < user_list->num_contexts; count++) {
        context = &(user_list->contexts[count]);

        if (context) {
            if (app_flag == 1)
                strncpy(src_str, context->apid, DLT_ID_SIZE);
            else
                strncpy(src_str, context->ctid, DLT_ID_SIZE);

            ret = strncmp(src_str, str, len);

            if (ret == 0)
                dlt_daemon_send_log_level(sock, daemon, daemon_local, context, loglevel, verbose);
            else if ((ret > 0) && (app_flag == 1))
                break;
            else
                continue;
        }
    }
}

void dlt_daemon_control_set_log_level(int sock,
                                      DltDaemon *daemon,
                                      DltDaemonLocal *daemon_local,
                                      DltMessage *msg,
                                      int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    char apid[DLT_ID_SIZE + 1] = { 0 };
    char ctid[DLT_ID_SIZE + 1] = { 0 };
    DltServiceSetLogLevel *req = NULL;
    DltDaemonContext *context = NULL;
    int8_t apid_length = 0;
    int8_t ctid_length = 0;

    if ((daemon == NULL) || (msg == NULL) || (msg->databuffer == NULL))
        return;

    if (dlt_check_rcv_data_size(msg->datasize, sizeof(DltServiceSetLogLevel)) < 0)
        return;

    req = (DltServiceSetLogLevel *)(msg->databuffer);

    if (daemon_local->flags.enforceContextLLAndTS)
        req->log_level = (uint8_t) getStatus(req->log_level, daemon_local->flags.contextLogLevel);

    dlt_set_id(apid, req->apid);
    dlt_set_id(ctid, req->ctid);
    apid_length = (int8_t) strlen(apid);
    ctid_length = (int8_t) strlen(ctid);

    if ((apid_length != 0) && (apid[apid_length - 1] == '*') && (ctid[0] == 0)) { /*apid provided having '*' in it and ctid is null*/
        dlt_daemon_find_multiple_context_and_send_log_level(sock,
                                                            daemon,
                                                            daemon_local,
                                                            1,
                                                            apid,
                                                            (int8_t) (apid_length - 1),
                                                            (int8_t) req->log_level,
                                                            verbose);
    }
    else if ((ctid_length != 0) && (ctid[ctid_length - 1] == '*') && (apid[0] == 0)) /*ctid provided is having '*' in it and apid is null*/
    {
        dlt_daemon_find_multiple_context_and_send_log_level(sock,
                                                            daemon,
                                                            daemon_local,
                                                            0,
                                                            ctid,
                                                            (int8_t) (ctid_length - 1),
                                                            (int8_t) req->log_level,
                                                            verbose);
    }
    else if ((apid_length != 0) && (apid[apid_length - 1] != '*') && (ctid[0] == 0)) /*only app id case*/
    {
        dlt_daemon_find_multiple_context_and_send_log_level(sock,
                                                            daemon,
                                                            daemon_local,
                                                            1,
                                                            apid,
                                                            DLT_ID_SIZE,
                                                            (int8_t) req->log_level,
                                                            verbose);
    }
    else if ((ctid_length != 0) && (ctid[ctid_length - 1] != '*') && (apid[0] == 0)) /*only context id case*/
    {
        dlt_daemon_find_multiple_context_and_send_log_level(sock,
                                                            daemon,
                                                            daemon_local,
                                                            0,
                                                            ctid,
                                                            DLT_ID_SIZE,
                                                            (int8_t) req->log_level,
                                                            verbose);
    }
    else {
        context = dlt_daemon_context_find(daemon,
                                          apid,
                                          ctid,
                                          daemon->ecuid,
                                          verbose);

        /* Set log level */
        if (context != 0) {
            dlt_daemon_send_log_level(sock, daemon, daemon_local, context, (int8_t) req->log_level, verbose);
        }
        else {
            dlt_vlog(LOG_ERR, "Could not set log level: %d. Context [%.4s:%.4s] not found:", req->log_level, apid,
                     ctid);
            dlt_daemon_control_service_response(sock,
                                                daemon,
                                                daemon_local,
                                                DLT_SERVICE_ID_SET_LOG_LEVEL,
                                                DLT_SERVICE_RESPONSE_ERROR,
                                                verbose);
        }
    }
}


void dlt_daemon_send_trace_status(int sock,
                                  DltDaemon *daemon,
                                  DltDaemonLocal *daemon_local,
                                  DltDaemonContext *context,
                                  int8_t tracestatus,
                                  int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    int32_t id = DLT_SERVICE_ID_SET_TRACE_STATUS;
    int8_t old_trace_status = 0;

    old_trace_status = context->trace_status;
    context->trace_status = tracestatus; /* No endianess conversion necessary*/

    if ((context->user_handle >= DLT_FD_MINIMUM) &&
        (dlt_daemon_user_send_log_level(daemon, context, verbose) == 0)) {
        dlt_daemon_control_service_response(sock, daemon, daemon_local, (uint32_t) id, DLT_SERVICE_RESPONSE_OK, verbose);
    }
    else {
        dlt_log(LOG_ERR, "Trace status could not be sent!\n");
        context->trace_status = old_trace_status;
        dlt_daemon_control_service_response(sock, daemon, daemon_local, (uint32_t) id, DLT_SERVICE_RESPONSE_ERROR, verbose);
    }
}

void dlt_daemon_find_multiple_context_and_send_trace_status(int sock,
                                                            DltDaemon *daemon,
                                                            DltDaemonLocal *daemon_local,
                                                            int8_t app_flag,
                                                            char *str,
                                                            int8_t len,
                                                            int8_t tracestatus,
                                                            int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    int count = 0;
    DltDaemonContext *context = NULL;
    char src_str[DLT_ID_SIZE + 1] = { 0 };
    int ret = 0;
    DltDaemonRegisteredUsers *user_list = NULL;

    if (daemon == 0) {
        dlt_vlog(LOG_ERR, "%s: Invalid parameters\n", __func__);
        return;
    }

    user_list = dlt_daemon_find_users_list(daemon, daemon->ecuid, verbose);

    if (user_list == NULL)
        return;

    for (count = 0; count < user_list->num_contexts; count++) {
        context = &(user_list->contexts[count]);

        if (context) {
            if (app_flag == 1)
                strncpy(src_str, context->apid, DLT_ID_SIZE);
            else
                strncpy(src_str, context->ctid, DLT_ID_SIZE);

            ret = strncmp(src_str, str, len);

            if (ret == 0)
                dlt_daemon_send_trace_status(sock, daemon, daemon_local, context, tracestatus, verbose);
            else if ((ret > 0) && (app_flag == 1))
                break;
            else
                continue;
        }
    }
}

void dlt_daemon_control_set_trace_status(int sock,
                                         DltDaemon *daemon,
                                         DltDaemonLocal *daemon_local,
                                         DltMessage *msg,
                                         int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    char apid[DLT_ID_SIZE + 1] = { 0 };
    char ctid[DLT_ID_SIZE + 1] = { 0 };
    DltServiceSetLogLevel *req = NULL;
    DltDaemonContext *context = NULL;
    int8_t apid_length = 0;
    int8_t ctid_length = 0;

    if ((daemon == NULL) || (msg == NULL) || (msg->databuffer == NULL))
        return;

    if (dlt_check_rcv_data_size(msg->datasize, sizeof(DltServiceSetLogLevel)) < 0)
        return;

    req = (DltServiceSetLogLevel *)(msg->databuffer);

    if (daemon_local->flags.enforceContextLLAndTS)
        req->log_level = (uint8_t) getStatus(req->log_level, daemon_local->flags.contextTraceStatus);

    dlt_set_id(apid, req->apid);
    dlt_set_id(ctid, req->ctid);
    apid_length = (int8_t) strlen(apid);
    ctid_length = (int8_t) strlen(ctid);

    if ((apid_length != 0) && (apid[apid_length - 1] == '*') && (ctid[0] == 0)) { /*apid provided having '*' in it and ctid is null*/
        dlt_daemon_find_multiple_context_and_send_trace_status(sock,
                                                               daemon,
                                                               daemon_local,
                                                               1,
                                                               apid,
                                                               (int8_t) (apid_length - 1),
                                                               (int8_t) req->log_level,
                                                               verbose);
    }
    else if ((ctid_length != 0) && (ctid[ctid_length - 1] == '*') && (apid[0] == 0)) /*ctid provided is having '*' in it and apid is null*/

    {
        dlt_daemon_find_multiple_context_and_send_trace_status(sock,
                                                               daemon,
                                                               daemon_local,
                                                               0,
                                                               ctid,
                                                               (int8_t) (ctid_length - 1),
                                                               (int8_t) req->log_level,
                                                               verbose);
    }
    else if ((apid_length != 0) && (apid[apid_length - 1] != '*') && (ctid[0] == 0)) /*only app id case*/
    {
        dlt_daemon_find_multiple_context_and_send_trace_status(sock,
                                                               daemon,
                                                               daemon_local,
                                                               1,
                                                               apid,
                                                               DLT_ID_SIZE,
                                                               (int8_t) req->log_level,
                                                               verbose);
    }
    else if ((ctid_length != 0) && (ctid[ctid_length - 1] != '*') && (apid[0] == 0)) /*only context id case*/
    {
        dlt_daemon_find_multiple_context_and_send_trace_status(sock,
                                                               daemon,
                                                               daemon_local,
                                                               0,
                                                               ctid,
                                                               DLT_ID_SIZE,
                                                               (int8_t) req->log_level,
                                                               verbose);
    }
    else {
        context = dlt_daemon_context_find(daemon, apid, ctid, daemon->ecuid, verbose);

        /* Set trace status */
        if (context != 0) {
            dlt_daemon_send_trace_status(sock, daemon, daemon_local, context, (int8_t) req->log_level, verbose);
        }
        else {
            dlt_vlog(LOG_ERR,
                     "Could not set trace status: %d. Context [%.4s:%.4s] not found:",
                     req->log_level,
                     apid,
                     ctid);
            dlt_daemon_control_service_response(sock,
                                                daemon,
                                                daemon_local,
                                                DLT_SERVICE_ID_SET_LOG_LEVEL,
                                                DLT_SERVICE_RESPONSE_ERROR,
                                                verbose);
        }
    }
}

void dlt_daemon_control_set_default_log_level(int sock,
                                              DltDaemon *daemon,
                                              DltDaemonLocal *daemon_local,
                                              DltMessage *msg,
                                              int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    DltServiceSetDefaultLogLevel *req;
    uint32_t id = DLT_SERVICE_ID_SET_DEFAULT_LOG_LEVEL;

    if ((daemon == NULL) || (msg == NULL) || (msg->databuffer == NULL))
        return;

    if (dlt_check_rcv_data_size(msg->datasize, sizeof(DltServiceSetDefaultLogLevel)) < 0)
        return;

    req = (DltServiceSetDefaultLogLevel *)(msg->databuffer);

    /* No endianess conversion necessary */
    if (/*(req->log_level>=0) &&*/
        (req->log_level <= DLT_LOG_VERBOSE)) {
        if (daemon_local->flags.enforceContextLLAndTS)
            daemon->default_log_level = getStatus(req->log_level, daemon_local->flags.contextLogLevel);
        else
            daemon->default_log_level = (int8_t) req->log_level; /* No endianess conversion necessary */

        /* Send Update to all contexts using the default log level */
        dlt_daemon_user_send_default_update(daemon, verbose);

        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_OK, verbose);
    }
    else {
        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR, verbose);
    }
}

void dlt_daemon_control_set_all_log_level(int sock,
                                          DltDaemon *daemon,
                                          DltDaemonLocal *daemon_local,
                                          DltMessage *msg,
                                          int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    DltServiceSetDefaultLogLevel *req = NULL;
    uint32_t id = DLT_SERVICE_ID_SET_ALL_LOG_LEVEL;
    int8_t loglevel = 0;

    if ((daemon == NULL) || (msg == NULL) || (msg->databuffer == NULL)) {
        dlt_vlog(LOG_ERR, "%s: Invalid parameters\n", __func__);
        return;
    }

    if (dlt_check_rcv_data_size(msg->datasize, sizeof(DltServiceSetDefaultLogLevel)) < 0)
        return;

    req = (DltServiceSetDefaultLogLevel *)(msg->databuffer);

    /* No endianess conversion necessary */
    if ((req != NULL) && ((req->log_level <= DLT_LOG_VERBOSE) || (req->log_level == (uint8_t)DLT_LOG_DEFAULT))) {
        loglevel = (int8_t) req->log_level;

        /* Send Update to all contexts using the new log level */
        dlt_daemon_user_send_all_log_level_update(
            daemon,
            daemon_local->flags.enforceContextLLAndTS,
            (int8_t)daemon_local->flags.contextLogLevel,
            loglevel,
            verbose);

        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_OK, verbose);
    }
    else {
        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR, verbose);
    }
}

void dlt_daemon_control_set_default_trace_status(int sock,
                                                 DltDaemon *daemon,
                                                 DltDaemonLocal *daemon_local,
                                                 DltMessage *msg,
                                                 int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    /* Payload of request message */
    DltServiceSetDefaultLogLevel *req;
    uint32_t id = DLT_SERVICE_ID_SET_DEFAULT_TRACE_STATUS;

    if ((daemon == NULL) || (msg == NULL) || (msg->databuffer == NULL))
        return;

    if (dlt_check_rcv_data_size(msg->datasize, sizeof(DltServiceSetDefaultLogLevel)) < 0)
        return;

    req = (DltServiceSetDefaultLogLevel *)(msg->databuffer);

    /* No endianess conversion necessary */
    if ((req->log_level == DLT_TRACE_STATUS_OFF) ||
        (req->log_level == DLT_TRACE_STATUS_ON)) {
        if (daemon_local->flags.enforceContextLLAndTS)
            daemon->default_trace_status = getStatus(req->log_level, daemon_local->flags.contextTraceStatus);
        else
            daemon->default_trace_status = (int8_t) req->log_level; /* No endianess conversion necessary*/

        /* Send Update to all contexts using the default trace status */
        dlt_daemon_user_send_default_update(daemon, verbose);

        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_OK, verbose);
    }
    else {
        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR, verbose);
    }
}

void dlt_daemon_control_set_all_trace_status(int sock,
                                             DltDaemon *daemon,
                                             DltDaemonLocal *daemon_local,
                                             DltMessage *msg,
                                             int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    DltServiceSetDefaultLogLevel *req = NULL;
    uint32_t id = DLT_SERVICE_ID_SET_ALL_TRACE_STATUS;
    int8_t tracestatus = 0;

    if ((daemon == NULL) || (msg == NULL) || (msg->databuffer == NULL)) {
        dlt_vlog(LOG_ERR, "%s: Invalid parameters\n", __func__);
        return;
    }

    if (dlt_check_rcv_data_size(msg->datasize, sizeof(DltServiceSetDefaultLogLevel)) < 0)
        return;

    req = (DltServiceSetDefaultLogLevel *)(msg->databuffer);

    /* No endianess conversion necessary */
    if ((req != NULL) &&
        ((req->log_level <= DLT_TRACE_STATUS_ON) || (req->log_level == (uint8_t)DLT_TRACE_STATUS_DEFAULT))) {
        if (daemon_local->flags.enforceContextLLAndTS)
            tracestatus = getStatus(req->log_level, daemon_local->flags.contextTraceStatus);
        else
            tracestatus = (int8_t) req->log_level; /* No endianess conversion necessary */

        /* Send Update to all contexts using the new log level */
        dlt_daemon_user_send_all_trace_status_update(daemon, tracestatus, verbose);

        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_OK, verbose);
    }
    else {
        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR, verbose);
    }
}

void dlt_daemon_control_set_timing_packets(int sock,
                                           DltDaemon *daemon,
                                           DltDaemonLocal *daemon_local,
                                           DltMessage *msg,
                                           int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    DltServiceSetVerboseMode *req;  /* request uses same struct as set verbose mode */
    uint32_t id = DLT_SERVICE_ID_SET_TIMING_PACKETS;

    if ((daemon == NULL) || (msg == NULL) || (msg->databuffer == NULL))
        return;

    if (dlt_check_rcv_data_size(msg->datasize, sizeof(DltServiceSetVerboseMode)) < 0)
        return;

    req = (DltServiceSetVerboseMode *)(msg->databuffer);

    if ((req->new_status == 0) || (req->new_status == 1)) {
        daemon->timingpackets = req->new_status;

        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_OK, verbose);
    }
    else {
        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR, verbose);
    }
}

void dlt_daemon_control_message_time(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    DltMessage msg;
    int32_t len;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == 0)
        return;

    /* initialise new message */
    if (dlt_message_init(&msg, 0) == DLT_RETURN_ERROR)
        return;

    /* send message */

    /* prepare storage header */
    msg.storageheader = (DltStorageHeader *)msg.headerbuffer;
    dlt_set_storageheader(msg.storageheader, daemon->ecuid);

    /* prepare standard header */
    msg.standardheader = (DltStandardHeader *)(msg.headerbuffer + sizeof(DltStorageHeader));
    msg.standardheader->htyp = DLT_HTYP_WEID | DLT_HTYP_WTMS | DLT_HTYP_UEH | DLT_HTYP_PROTOCOL_VERSION1;

#if (BYTE_ORDER == BIG_ENDIAN)
    msg.standardheader->htyp = (msg.standardheader->htyp | DLT_HTYP_MSBF);
#endif

    msg.standardheader->mcnt = 0;

    /* Set header extra parameters */
    dlt_set_id(msg.headerextra.ecu, daemon->ecuid);
    msg.headerextra.tmsp = dlt_uptime();

    dlt_message_set_extraparameters(&msg, verbose);

    /* prepare extended header */
    msg.extendedheader =
        (DltExtendedHeader *)(msg.headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader) +
                              DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp));
    msg.extendedheader->msin = DLT_MSIN_CONTROL_TIME;

    msg.extendedheader->noar = 0;                  /* number of arguments */
    dlt_set_id(msg.extendedheader->apid, "");       /* application id */
    dlt_set_id(msg.extendedheader->ctid, "");       /* context id */

    /* prepare length information */
    msg.headersize = (uint32_t) (sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + sizeof(DltExtendedHeader) +
        DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp));

    len = (int32_t) (msg.headersize - sizeof(DltStorageHeader) + msg.datasize);

    if (len > UINT16_MAX) {
        dlt_log(LOG_WARNING, "Huge control message discarded!\n");

        /* free message */
        dlt_message_free(&msg, 0);

        return;
    }

    msg.standardheader->len = DLT_HTOBE_16(((uint16_t)len));

    /* Send message, ignore return value */
    dlt_daemon_client_send(sock, daemon, daemon_local, msg.headerbuffer,
                           sizeof(DltStorageHeader),
                           msg.headerbuffer + sizeof(DltStorageHeader),
                           (int) msg.headersize - (int) sizeof(DltStorageHeader),
                           msg.databuffer, (int) msg.datasize, verbose);

    /* free message */
    dlt_message_free(&msg, 0);
}

int dlt_daemon_process_one_s_timer(DltDaemon *daemon,
                                   DltDaemonLocal *daemon_local,
                                   DltReceiver *receiver,
                                   int verbose)
{
    uint64_t expir = 0;
    ssize_t res = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon_local == NULL) || (daemon == NULL) || (receiver == NULL)) {
        dlt_vlog(LOG_ERR, "%s: invalid parameters", __func__);
        return -1;
    }

    res = read(receiver->fd, &expir, sizeof(expir));

    if (res < 0) {
        dlt_vlog(LOG_WARNING, "%s: Fail to read timer (%s)\n", __func__,
                 strerror(errno));
        /* Activity received on timer_wd, but unable to read the fd:
         * let's go on sending notification */
    }

    if ((daemon->state == DLT_DAEMON_STATE_SEND_BUFFER) ||
        (daemon->state == DLT_DAEMON_STATE_BUFFER_FULL)) {
        if (dlt_daemon_send_ringbuffer_to_client(daemon,
                                                 daemon_local,
                                                 daemon_local->flags.vflag))
            dlt_log(LOG_DEBUG,
                    "Can't send contents of ring buffer to clients\n");
    }

    if ((daemon->timingpackets) &&
        (daemon->state == DLT_DAEMON_STATE_SEND_DIRECT))
        dlt_daemon_control_message_time(DLT_DAEMON_SEND_TO_ALL,
                                        daemon,
                                        daemon_local,
                                        daemon_local->flags.vflag);

    dlt_log(LOG_DEBUG, "Timer timingpacket\n");

    return 0;
}

int dlt_daemon_process_sixty_s_timer(DltDaemon *daemon,
                                     DltDaemonLocal *daemon_local,
                                     DltReceiver *receiver,
                                     int verbose)
{
    uint64_t expir = 0;
    ssize_t res = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon_local == NULL) || (daemon == NULL) || (receiver == NULL)) {
        dlt_vlog(LOG_ERR, "%s: invalid parameters", __func__);
        return -1;
    }

    res = read(receiver->fd, &expir, sizeof(expir));

    if (res < 0) {
        dlt_vlog(LOG_WARNING, "%s: Fail to read timer (%s)\n", __func__,
                 strerror(errno));
        /* Activity received on timer_wd, but unable to read the fd:
         * let's go on sending notification */
    }

    if (daemon_local->flags.sendECUSoftwareVersion > 0)
        dlt_daemon_control_get_software_version(DLT_DAEMON_SEND_TO_ALL,
                                                daemon,
                                                daemon_local,
                                                daemon_local->flags.vflag);

    if (daemon_local->flags.sendTimezone > 0) {
        /* send timezone information */
        time_t t = time(NULL);
        struct tm lt;

        /*Added memset to avoid compiler warning for near initialization */
        memset((void *)&lt, 0, sizeof(lt));
        tzset();
        localtime_r(&t, &lt);

        dlt_daemon_control_message_timezone(DLT_DAEMON_SEND_TO_ALL,
                                            daemon,
                                            daemon_local,
                                            daemon_local->flags.vflag);
    }

    dlt_log(LOG_DEBUG, "Timer ecuversion\n");

    return 0;
}

#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
int dlt_daemon_process_systemd_timer(DltDaemon *daemon,
                                     DltDaemonLocal *daemon_local,
                                     DltReceiver *receiver,
                                     int verbose)
{
    uint64_t expir = 0;
    ssize_t res = -1;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon_local == NULL) || (daemon == NULL) || (receiver == NULL)) {
        dlt_vlog(LOG_ERR, "%s: invalid parameters", __func__);
        return res;
    }

    res = read(receiver->fd, &expir, sizeof(expir));

    if (res < 0) {
        dlt_vlog(LOG_WARNING, "Failed to read timer_wd; %s\n", strerror(errno));
        /* Activity received on timer_wd, but unable to read the fd:
         * let's go on sending notification */
    }

#ifdef DLT_SYSTEMD_WATCHDOG_ENFORCE_MSG_RX_ENABLE
    if (!daemon->received_message_since_last_watchdog_interval) {
      dlt_log(LOG_WARNING, "No new messages received since last watchdog timer run\n");
      return 0;
    }
    daemon->received_message_since_last_watchdog_interval = 0;
#endif

    if (sd_notify(0, "WATCHDOG=1") < 0)
        dlt_log(LOG_CRIT, "Could not reset systemd watchdog\n");

    dlt_log(LOG_DEBUG, "Timer watchdog\n");

    return 0;
}
#else
int dlt_daemon_process_systemd_timer(DltDaemon *daemon,
                                     DltDaemonLocal *daemon_local,
                                     DltReceiver *receiver,
                                     int verbose)
{
    (void)daemon;
    (void)daemon_local;
    (void)receiver;
    (void)verbose;

    dlt_log(LOG_DEBUG, "Timer watchdog not enabled\n");

    return -1;
}
#endif

void dlt_daemon_control_service_logstorage(int sock,
                                           DltDaemon *daemon,
                                           DltDaemonLocal *daemon_local,
                                           DltMessage *msg,
                                           int verbose)
{
    DltServiceOfflineLogstorage *req;
    int ret;
    unsigned int connection_type = 0;
    DltLogStorage *device = NULL;
    int device_index = -1;
    uint32_t i = 0;

    int tmp_errno = 0;

    struct stat daemon_mpoint_st = {0};
    int daemon_st_status = 0;

    struct stat req_mpoint_st = {0};
    int req_st_status = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (msg == NULL) || (daemon_local == NULL)) {
        dlt_vlog(LOG_ERR,
                 "%s: Invalid function parameters\n",
                 __func__);
        return;
    }

    if ((daemon_local->flags.offlineLogstorageMaxDevices <= 0) || (msg->databuffer == NULL)) {
        dlt_daemon_control_service_response(sock,
                                            daemon,
                                            daemon_local,
                                            DLT_SERVICE_ID_OFFLINE_LOGSTORAGE,
                                            DLT_SERVICE_RESPONSE_ERROR,
                                            verbose);

        dlt_log(LOG_INFO,
                "Logstorage functionality not enabled or MAX device set is 0\n");
        return;
    }

    if (dlt_check_rcv_data_size(msg->datasize, sizeof(DltServiceOfflineLogstorage)) < 0)
        return;

    req = (DltServiceOfflineLogstorage *)(msg->databuffer);

    if(req->connection_type != DLT_OFFLINE_LOGSTORAGE_SYNC_CACHES) {
        req_st_status = stat(req->mount_point, &req_mpoint_st);
        tmp_errno = errno;
        if (req_st_status < 0) {
            dlt_daemon_control_service_response(sock,
                                                daemon,
                                                daemon_local,
                                                DLT_SERVICE_ID_OFFLINE_LOGSTORAGE,
                                                DLT_SERVICE_RESPONSE_ERROR,
                                                verbose);

            dlt_vlog(LOG_WARNING,
                     "%s: Failed to stat requested mount point [%s] with error [%s]\n",
                     __func__, req->mount_point, strerror(tmp_errno));
            return;
        }
    }

    for (i = 0; i < (uint32_t) daemon_local->flags.offlineLogstorageMaxDevices; i++) {
        connection_type = daemon->storage_handle[i].connection_type;

        memset(&daemon_mpoint_st, 0, sizeof(struct stat));
        if (strlen(daemon->storage_handle[i].device_mount_point) > 1) {
            daemon_st_status = stat(daemon->storage_handle[i].device_mount_point,
                    &daemon_mpoint_st);
            tmp_errno = errno;

            if (daemon_st_status < 0) {
                dlt_daemon_control_service_response(sock,
                                                    daemon,
                                                    daemon_local,
                                                    DLT_SERVICE_ID_OFFLINE_LOGSTORAGE,
                                                    DLT_SERVICE_RESPONSE_ERROR,
                                                    verbose);
                dlt_vlog(LOG_WARNING,
                        "%s: Failed to stat daemon mount point [%s] with error [%s]\n",
                        __func__, daemon->storage_handle[i].device_mount_point,
                        strerror(tmp_errno));
                return;
            }

            /* Check if the requested device path is already used as log storage device */
            if (req_mpoint_st.st_dev == daemon_mpoint_st.st_dev &&
                    req_mpoint_st.st_ino == daemon_mpoint_st.st_ino) {
                device_index = (int) i;
                break;
            }
        }

        /* Get first available device index here */
        if ((connection_type != DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED) &&
            (device_index == -1))
            device_index = (int) i;
    }

    /* It might be possible to sync all caches of all devices */
    if ((req->connection_type == DLT_OFFLINE_LOGSTORAGE_SYNC_CACHES) &&
        (strlen(req->mount_point) == 0)) {
        /* It is expected to receive an empty mount point to sync all Logstorage
         * devices in this case. */
    }
    else if (device_index == -1) {
        dlt_daemon_control_service_response(sock,
                                            daemon,
                                            daemon_local,
                                            DLT_SERVICE_ID_OFFLINE_LOGSTORAGE,
                                            DLT_SERVICE_RESPONSE_ERROR,
                                            verbose);
        dlt_log(LOG_WARNING, "MAX devices already in use  \n");
        return;
    }

    /* Check for device connection request from log storage ctrl app  */
    device = &daemon->storage_handle[device_index];

    if (req->connection_type == DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED) {
        ret = dlt_logstorage_device_connected(device, req->mount_point);

        if (ret == 1) {
            dlt_daemon_control_service_response(sock,
                                                daemon,
                                                daemon_local,
                                                DLT_SERVICE_ID_OFFLINE_LOGSTORAGE,
                                                DLT_SERVICE_RESPONSE_WARNING,
                                                verbose);
            return;
        }
        else if (ret != 0)
        {
            dlt_daemon_control_service_response(sock,
                                                daemon,
                                                daemon_local,
                                                DLT_SERVICE_ID_OFFLINE_LOGSTORAGE,
                                                DLT_SERVICE_RESPONSE_ERROR,
                                                verbose);
            return;
        }

        dlt_daemon_control_service_response(sock,
                                            daemon,
                                            daemon_local,
                                            DLT_SERVICE_ID_OFFLINE_LOGSTORAGE,
                                            DLT_SERVICE_RESPONSE_OK,
                                            verbose);

        /* Update maintain logstorage loglevel if necessary */
        if (daemon->storage_handle[device_index].maintain_logstorage_loglevel != DLT_MAINTAIN_LOGSTORAGE_LOGLEVEL_UNDEF)
        {
            daemon->maintain_logstorage_loglevel = daemon->storage_handle[device_index].maintain_logstorage_loglevel;
        }

        /* Check if log level of running application needs an update */
        dlt_daemon_logstorage_update_application_loglevel(daemon,
                                                          daemon_local,
                                                          device_index,
                                                          verbose);

    }
    /* Check for device disconnection request from log storage ctrl app  */
    else if (req->connection_type == DLT_OFFLINE_LOGSTORAGE_DEVICE_DISCONNECTED)
    {
        /* Check if log level of running application needs to be reset */
        dlt_daemon_logstorage_reset_application_loglevel(
            daemon,
            daemon_local,
            device_index,
            (int) daemon_local->flags.offlineLogstorageMaxDevices,
            verbose);

        dlt_logstorage_device_disconnected(&(daemon->storage_handle[device_index]),
                                           DLT_LOGSTORAGE_SYNC_ON_DEVICE_DISCONNECT);

        dlt_daemon_control_service_response(sock,
                                            daemon,
                                            daemon_local,
                                            DLT_SERVICE_ID_OFFLINE_LOGSTORAGE,
                                            DLT_SERVICE_RESPONSE_OK,
                                            verbose);

    }
    /* Check for cache synchronization request from log storage ctrl app */
    else if (req->connection_type == DLT_OFFLINE_LOGSTORAGE_SYNC_CACHES)
    {
        ret = 0;

        if (device_index == -1) { /* sync all Logstorage devices */

            for (i = 0; i < (uint32_t) daemon_local->flags.offlineLogstorageMaxDevices; i++)
                if (daemon->storage_handle[i].connection_type ==
                    DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED)
                    ret = dlt_daemon_logstorage_sync_cache(
                        daemon,
                        daemon_local,
                        daemon->storage_handle[i].device_mount_point,
                        verbose);
        }
        else {
            /* trigger logstorage to sync caches */
            ret = dlt_daemon_logstorage_sync_cache(daemon,
                                                   daemon_local,
                                                   req->mount_point,
                                                   verbose);
        }

        if (ret == 0)
            dlt_daemon_control_service_response(sock,
                                                daemon,
                                                daemon_local,
                                                DLT_SERVICE_ID_OFFLINE_LOGSTORAGE,
                                                DLT_SERVICE_RESPONSE_OK,
                                                verbose);
        else
            dlt_daemon_control_service_response(sock,
                                                daemon,
                                                daemon_local,
                                                DLT_SERVICE_ID_OFFLINE_LOGSTORAGE,
                                                DLT_SERVICE_RESPONSE_ERROR,
                                                verbose);
    }
    else {
        dlt_daemon_control_service_response(sock,
                                            daemon,
                                            daemon_local,
                                            DLT_SERVICE_ID_OFFLINE_LOGSTORAGE,
                                            DLT_SERVICE_RESPONSE_ERROR,
                                            verbose);
    }
}

void dlt_daemon_control_passive_node_connect(int sock,
                                             DltDaemon *daemon,
                                             DltDaemonLocal *daemon_local,
                                             DltMessage *msg,
                                             int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    DltServicePassiveNodeConnect *req;
    uint32_t id = DLT_SERVICE_ID_PASSIVE_NODE_CONNECT;

    if ((daemon == NULL) || (daemon_local == NULL) || (msg == NULL) ||
        (msg->databuffer == NULL))
        return;

    /* return error, if gateway mode not enabled*/
    if (daemon_local->flags.gatewayMode == 0) {
        dlt_log(LOG_WARNING,
                "Received passive node connection status request, "
                "but GatewayMode is disabled\n");

        dlt_daemon_control_service_response(
            sock,
            daemon,
            daemon_local,
            DLT_SERVICE_ID_PASSIVE_NODE_CONNECTION_STATUS,
            DLT_SERVICE_RESPONSE_ERROR,
            verbose);

        return;
    }

    if (dlt_check_rcv_data_size(msg->datasize, sizeof(DltServicePassiveNodeConnect)) < 0)
        return;

    req = (DltServicePassiveNodeConnect *)msg->databuffer;

    if (dlt_gateway_process_on_demand_request(&daemon_local->pGateway,
                                              daemon_local,
                                              req->node_id,
                                              (int) req->connection_status,
                                              verbose) < 0)
        dlt_daemon_control_service_response(sock,
                                            daemon,
                                            daemon_local,
                                            id,
                                            DLT_SERVICE_RESPONSE_ERROR,
                                            verbose);
    else
        dlt_daemon_control_service_response(sock,
                                            daemon,
                                            daemon_local,
                                            id,
                                            DLT_SERVICE_RESPONSE_OK,
                                            verbose);
}

void dlt_daemon_control_passive_node_connect_status(int sock,
                                                    DltDaemon *daemon,
                                                    DltDaemonLocal *daemon_local,
                                                    int verbose)
{
    DltMessage msg;
    DltServicePassiveNodeConnectionInfo *resp;
    DltGatewayConnection *con = NULL;
    unsigned int i = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL))
        return;

    if (dlt_message_init(&msg, verbose) == -1)
        return;

    /* return error, if gateway mode not enabled*/
    if (daemon_local->flags.gatewayMode == 0) {
        dlt_log(LOG_WARNING,
                "Received passive node connection status request, "
                "but GatewayMode is disabled\n");

        dlt_daemon_control_service_response(
            sock,
            daemon,
            daemon_local,
            DLT_SERVICE_ID_PASSIVE_NODE_CONNECTION_STATUS,
            DLT_SERVICE_RESPONSE_ERROR,
            verbose);

        return;
    }

    /* prepare payload of data */
    msg.datasize = sizeof(DltServicePassiveNodeConnectionInfo);

    if (msg.databuffer && (msg.databuffersize < msg.datasize))
        msg.databuffer = NULL;

    if (msg.databuffer == NULL) {
        msg.databuffer = (uint8_t *)malloc(msg.datasize);

        if (msg.databuffer == NULL) {
            dlt_log(LOG_CRIT, "Cannot allocate memory for message response\n");
            return;
        }

        msg.databuffersize = msg.datasize;
    }

    resp = (DltServicePassiveNodeConnectionInfo *)msg.databuffer;
    memset(resp, 0, msg.datasize);
    resp->service_id = DLT_SERVICE_ID_PASSIVE_NODE_CONNECTION_STATUS;
    resp->status = DLT_SERVICE_RESPONSE_OK;
    resp->num_connections = (uint32_t) daemon_local->pGateway.num_connections;

    for (i = 0; i < resp->num_connections; i++) {
        if ((i * DLT_ID_SIZE) > DLT_ENTRY_MAX) {
            dlt_log(LOG_ERR,
                    "Maximal message size reached. Skip further information\n");
            break;
        }

        con = &daemon_local->pGateway.connections[i];

        if (con == NULL) {
            dlt_log(LOG_CRIT, "Passive node connection structure is NULL\n");
            dlt_daemon_control_service_response(
                sock,
                daemon,
                daemon_local,
                DLT_SERVICE_ID_PASSIVE_NODE_CONNECTION_STATUS,
                DLT_SERVICE_RESPONSE_ERROR,
                verbose);

            /* free message */
            dlt_message_free(&msg, verbose);

            return;
        }

        resp->connection_status[i] = con->status;
        memcpy(&resp->node_id[i * DLT_ID_SIZE], con->ecuid, DLT_ID_SIZE);
    }

    dlt_daemon_client_send_control_message(sock,
                                           daemon,
                                           daemon_local,
                                           &msg,
                                           "",
                                           "",
                                           verbose);
    /* free message */
    dlt_message_free(&msg, verbose);
}
