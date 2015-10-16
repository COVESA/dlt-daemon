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
 * Frederic Berat <fberat@de.adit-jv.com>
 *
 * \copyright Copyright © 2015 Advanced Driver Information Technology. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_daemon_connection.c
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/types.h>

#include "dlt_daemon_connection_types.h"
#include "dlt_daemon_connection.h"
#include "dlt_daemon_event_handler_types.h"
#include "dlt_daemon_event_handler.h"
#include "dlt-daemon.h"
#include "dlt-daemon_cfg.h"
#include "dlt_daemon_common.h"
#include "dlt_common.h"
#include "dlt_gateway.h"

/** @brief Generic sending function.
 *
 * We manage different type of connection which have similar send/write
 * functions. We can then abstract the data transfer using this function,
 * moreover as we often transfer data to different kind of connection
 * within the same loop.
 *
 * @param conn The connection structure.
 * @param msg The message buffer to be sent
 * @param msg_size The length of the message to be sent
 *
 * @return The amount of bytes send on success, -1 otherwise.
 *         errno is appropriately set.
 */
static int dlt_connection_send(DltConnection *conn,
                              void *msg,
                              size_t msg_size)
{
    DltConnectionType type = DLT_CONNECTION_TYPE_MAX;

    if (conn != NULL)
    {
        type = conn->type;
    }

    switch (type)
    {
    case DLT_CONNECTION_CLIENT_MSG_SERIAL:
        return write(conn->fd, msg, msg_size);
    case DLT_CONNECTION_CLIENT_MSG_TCP:
        return send(conn->fd, msg, msg_size, 0);
    default:
        return -1;
    }
}

/** @brief Send up to two messages through a connection.
 *
 * We often need to send 2 messages through a specific connection, plus
 * the serial header. This function groups these different calls.
 *
 * @param con The connection to send the messages through.
 * @param data1 The first message to be sent.
 * @param size1 The size of the first message.
 * @param data2 The second message to be send.
 * @param size2 The second message size.
 * @param sendserialheader Whether we need or not to send the serial header.
 *
 * @return DLT_DAEMON_ERROR_OK on success, -1 otherwise. errno is properly set.
 */
int dlt_connection_send_multiple(DltConnection *con,
                                void *data1,
                                int size1,
                                void *data2,
                                int size2,
                                int sendserialheader)
{
    int ret = 0;

    if (con == NULL)
    {
        return -1;
    }

    if (sendserialheader)
    {
        ret = dlt_connection_send(con,
                                 (void *)dltSerialHeader,
                                 sizeof(dltSerialHeader));
    }

    if ((data1 != NULL) && (ret >= 0))
    {
        ret = dlt_connection_send(con, data1, size1);
    }

    if ((data2 != NULL) && (ret >= 0))
    {
        ret = dlt_connection_send(con, data2, size2);
    }

    if (ret >=0)
    {
        ret = DLT_DAEMON_ERROR_OK;
    }

    return ret;
}

/** @brief Get the next connection filtered with a type mask.
 *
 * In some cases we need the next connection available of a specific type or
 * specific different types. This function returns the next available connection
 * that is of one of the types included in the mask. The current connection can
 * be returned.
 *
 * @param current The current connection pointer.
 * @param type_mask A bit mask representing the connection types to be filtered.
 *
 * @return The next available connection of the considered types or NULL.
 */
DltConnection *dlt_connection_get_next(DltConnection *current, int type_mask)
{
    while (current && !((1 << current->type) & type_mask))
    {
        current = current->next;
    }

    return current;
}

/** @brief Get the receiver structure associated to a connection.
 *
 * The receiver structure is sometimes needed while handling the event.
 * This behavior is mainly due to the fact that it's not intended to modify
 * the whole design of the daemon while implementing the new event handling.
 * Based on the connection type provided, this function returns the pointer
 * to the DltReceiver structure corresponding.
 *
 * @param dameon_local Structure where to take the DltReceiver pointer from.
 * @param type Type of the connection.
 *
 * @return DltReceiver structure or NULL if none corresponds to the type.
 */
static DltReceiver *dlt_connection_get_receiver(DltDaemonLocal *daemon_local,
                                               DltConnectionType type,
                                               int fd)
{
    DltReceiver *ret = NULL;

    switch (type)
    {
    case DLT_CONNECTION_CLIENT_CONNECT:
    /* FALL THROUGH */
    /* There must be the same structure for this case */
    case DLT_CONNECTION_CLIENT_MSG_TCP:
        ret = &daemon_local->receiverSock;
        break;
    case DLT_CONNECTION_CLIENT_MSG_SERIAL:
        ret = &daemon_local->receiverSerial;
        break;
    case DLT_CONNECTION_APP_MSG:
        ret = &daemon_local->receiver;
        break;
    case DLT_CONNECTION_ONE_S_TIMER:
        ret = &daemon_local->timer_one_s;
        break;
    case DLT_CONNECTION_SIXTY_S_TIMER:
        ret = &daemon_local->timer_sixty_s;
        break;
#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
    case DLT_CONNECTION_SYSTEMD_TIMER:
        ret = &daemon_local->timer_wd;
        break;
#endif
    case DLT_CONNECTION_CONTROL_CONNECT:
    /* FALL THROUGH */
    /* There must be the same structure for this case */
    case DLT_CONNECTION_CONTROL_MSG:
        ret = &daemon_local->receiverCtrlSock;
        break;
    case DLT_CONNECTION_GATEWAY:
        /* FIXME: This is complete different approach compared to having the
         *        receiver as part of daemon_local structure. Approaches need
         *        to be harmonized.
         */
        ret = dlt_gateway_get_connection_receiver(&daemon_local->pGateway, fd);
        break;
    case DLT_CONNECTION_GATEWAY_TIMER:
        ret = &daemon_local->timer_gateway;
        break;
    default:
        ret = NULL;
    }

    return ret;
}

/** @brief Get the callback from a specific connection.
 *
 * The callback retrieved that way is used to handle event for this connection.
 * It as been chosen to proceed that way instead of having the callback directly
 * in the structure in order to have some way to check that the structure is
 * still valid, or at least gracefully handle errors instead of crashing.
 *
 * @param con The connection to retrieve the callback from.
 *
 * @return Function pointer or NULL.
 */
void *dlt_connection_get_callback(DltConnection *con)
{
    void *ret = NULL;
    DltConnectionType type = DLT_CONNECTION_TYPE_MAX;

    if (con)
    {
        type = con->type;
    }

    switch (type)
    {
    case DLT_CONNECTION_CLIENT_CONNECT:
        ret = dlt_daemon_process_client_connect;
        break;
    case DLT_CONNECTION_CLIENT_MSG_TCP:
        ret = dlt_daemon_process_client_messages;
        break;
    case DLT_CONNECTION_CLIENT_MSG_SERIAL:
        ret = dlt_daemon_process_client_messages_serial;
        break;
    case DLT_CONNECTION_APP_MSG:
        ret = dlt_daemon_process_user_messages;
        break;
    case DLT_CONNECTION_ONE_S_TIMER:
        ret = dlt_daemon_process_one_s_timer;
        break;
    case DLT_CONNECTION_SIXTY_S_TIMER:
        ret = dlt_daemon_process_sixty_s_timer;
        break;
#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
    case DLT_CONNECTION_SYSTEMD_TIMER:
        ret = dlt_daemon_process_systemd_timer;
        break;
#endif
    case DLT_CONNECTION_CONTROL_CONNECT:
        ret = dlt_daemon_process_control_connect;
        break;
    case DLT_CONNECTION_CONTROL_MSG:
        ret = dlt_daemon_process_control_messages;
        break;
    case DLT_CONNECTION_GATEWAY:
        ret = dlt_gateway_process_passive_node_messages;
        break;
    case DLT_CONNECTION_GATEWAY_TIMER:
        ret = dlt_gateway_process_gateway_timer;
        break;
    default:
        ret = NULL;
    }

    return ret;
}

/** @brief Destroys a connection.
 *
 * This function closes and frees the corresponding connection. This is expected
 * to be called by the connection owner: the DltEventHandler.
 * Ownership of the connection is given during the registration to
 * the DltEventHandler.
 *
 * @param to_destroy Connection to be destroyed.
 */
void dlt_connection_destroy(DltConnection *to_destroy)
{
    close(to_destroy->fd);
    free(to_destroy);
}

/** @brief Creates a connection and registers it to the DltEventHandler.
 *
 * The function will allocate memory for the connection, and give the pointer
 * to the DltEventHandler in order to register it for incoming events.
 * The connection is then destroyed later on, once it's not needed anymore or
 * it the event handler is destroyed.
 *
 * @param daemon_local Structure were some needed information is.
 * @param evh DltEventHandler to register the connection to.
 * @param fd File descriptor of the connection.
 * @param mask Event list bit mask.
 * @param type Connection type.
 *
 * @return 0 On success, -1 otherwise.
 */
int dlt_connection_create(DltDaemonLocal *daemon_local,
                         DltEventHandler *evh,
                         int fd,
                         int mask,
                         DltConnectionType type)
{
    DltConnection *temp = NULL;

    if (dlt_event_handler_find_connection(evh, fd) != NULL)
    {
        /* No need for the same client to be registered twice
         * for the same event.
         * TODO: If another mask can be expected,
         * we need it to update the epoll event here.
         */
        return 0;
    }

    temp = (DltConnection *)malloc(sizeof(DltConnection));

    if (temp == NULL)
    {
        dlt_log(LOG_CRIT, "Allocation of client handle failed\n");
        return -1;
    }

    memset(temp, 0, sizeof(DltConnection));

    temp->fd = fd;
    temp->type = type;
    temp->receiver = dlt_connection_get_receiver(daemon_local, type, fd);

    /* Now give the ownership of the newly created connection
     * to the event handler, by registering for events.
     */
    return dlt_event_handler_register_connection(evh, daemon_local, temp, mask);
}

/** @brief Creates connection from all types.
 *
 * This functions run through all connection types and tries to register
 * on the event handler the one that were not already registered.
 *
 * @param daemon_local Structure to retrieve information from.
 *
 * @return 0 on success, -1 if a failure occurs.
 */
int dlt_connection_create_remaining(DltDaemonLocal *daemon_local)
{
    DltConnectionType i = 0;
    DltEventHandler *ev = &daemon_local->pEvent;
    char local_str[DLT_DAEMON_TEXTBUFSIZE];

    for (i = 0 ; i < DLT_CONNECTION_TYPE_MAX ; i++)
    {
        int fd = 0;
        DltReceiver *rec = dlt_connection_get_receiver(daemon_local, i, fd);

        if (rec == NULL)
        {
            /* We are not interested if there is no receiver available */
            continue;
        }

        fd = rec->fd;

        /* Already created connections are ignored here */
        if ((fd > 0) && dlt_connection_create(daemon_local, ev, fd, EPOLLIN, i))
        {
            snprintf(local_str,
                     DLT_DAEMON_TEXTBUFSIZE,
                     "Unable to register type %i event handler.\n",
                     i);

            dlt_log(LOG_ERR, local_str);

            return -1;
        }
    }

    return 0;
}
