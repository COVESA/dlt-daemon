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
 * \file dlt_daemon_event_handler.c
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/epoll.h>
#include <sys/syslog.h>

#include "dlt_common.h"

#include "dlt-daemon.h"
#include "dlt-daemon_cfg.h"
#include "dlt_daemon_common.h"
#include "dlt_daemon_connection.h"
#include "dlt_daemon_connection_types.h"
#include "dlt_daemon_event_handler.h"
#include "dlt_daemon_event_handler_types.h"

/**
 * \def DLT_EPOLL_TIMEOUT_MSEC
 * The maximum amount of time to wait for an epoll event.
 * Set to 1 second to avoid unnecessary wake ups.
 */
#define DLT_EPOLL_TIMEOUT_MSEC 1000

/** @brief Prepare the event handler
 *
 * This will create the epoll file descriptor.
 *
 * @param ev The event handler to prepare.
 *
 * @return 0 on success, -1 otherwise.
 */
int dlt_daemon_prepare_event_handling(DltEventHandler *ev)
{
    if (ev == NULL)
    {
        return DLT_RETURN_ERROR;
    }
    ev->epfd = epoll_create(DLT_EPOLL_MAX_EVENTS);

    if (ev->epfd < 0)
    {
        dlt_log(LOG_CRIT, "Creation of epoll instance failed!\n");
        return -1;
    }

    return 0;
}

/** @brief Catch and process incoming events.
 *
 * This function waits for events on all connections. Once an event raise,
 * the callback for the specific connection is called, or the connection is
 * destroyed if a hangup occurs.
 *
 * @param daemon Structure to be passed to the callback.
 * @param daemon_local Structure containing needed information.
 * @param pEvent Event handler structure.
 *
 * @return 0 on success, -1 otherwise. May be interrupted.
 */
int dlt_daemon_handle_event(DltEventHandler *pEvent,
                            DltDaemon *daemon,
                            DltDaemonLocal *daemon_local)
{
    if ((pEvent == NULL) || (daemon  == NULL) || (daemon_local == NULL))
    {
        return DLT_RETURN_ERROR;
    }
    int nfds = 0;
    int i = 0;
    char str[DLT_DAEMON_TEXTBUFSIZE];
    int (*callback)(DltDaemon *, DltDaemonLocal *, DltReceiver *, int) = NULL;

    /*CM Change begin*/
    nfds = epoll_wait(pEvent->epfd,
                      pEvent->events,
                      DLT_EPOLL_MAX_EVENTS,
                      DLT_EPOLL_TIMEOUT_MSEC);

    if (nfds < 0)
    {
        /* We are not interested in EINTR has it comes
         * either from timeout or signal.
         */
        if (errno != EINTR)
        {
            snprintf(str,
                     DLT_DAEMON_TEXTBUFSIZE,
                     "epoll_wait() failed: %s\n",
                     strerror(errno));
            dlt_log(LOG_CRIT, str);
            return -1;
        }

        return 0;
    }

    for (i = 0 ; i < nfds ; i++)
    {
        struct epoll_event *ev = &pEvent->events[i];
        DltConnectionId id = (DltConnectionId)ev->data.ptr;
        DltConnection *con = dlt_event_handler_find_connection_by_id(pEvent,
                                                                     id);
        int fd = 0;
        DltConnectionType type = DLT_CONNECTION_TYPE_MAX;

        if (con && con->receiver)
        {
            type = con->type;
            fd = con->receiver->fd;
        }
        else /* connection might have been destroyed in the meanwhile */
        {
            continue;
        }

        /* First of all handle epoll error events
         * We only expect EPOLLIN or EPOLLOUT
         */
        if ((ev->events != EPOLLIN) && (ev->events != EPOLLOUT))
        {
            /* epoll reports an error, we need to clean-up the concerned event
             */
            dlt_event_handler_unregister_connection(pEvent,
                                                   daemon_local,
                                                   fd);
            continue;
        }

        /* Get the function to be used to handle the event */
        callback = dlt_connection_get_callback(con);

        if (!callback)
        {
            snprintf(str,
                     DLT_DAEMON_TEXTBUFSIZE,
                     "Unable to find function for %d handle type.\n",
                     type);
            dlt_log(LOG_CRIT, str);
            return -1;
        }

        /* From now on, callback is correct */
        if (callback(daemon,
                     daemon_local,
                     con->receiver,
                     daemon_local->flags.vflag) == -1)
        {
            snprintf(str,
                     DLT_DAEMON_TEXTBUFSIZE,
                     "Processing from %d handle type failed!\n",
                     type );
            dlt_log(LOG_CRIT, str);
            return -1;
        }
    }
    return 0;
}

/** @brief Find connection with a specific \a fd in the connection list.
 *
 * There can be only one event per \a fd. We can then find a specific connection
 * based on this \a fd. That allows to check if a specific \a fd has already been
 * registered.
 *
 * @param ev The event handler structure where the list of connection is.
 * @param fd The file descriptor of the connection to be found.
 *
 * @return The found connection pointer, NULL otherwise.
 */
DltConnection *dlt_event_handler_find_connection(DltEventHandler *ev,
                                               int fd)
{

    DltConnection *temp = ev->connections;

    while ((temp != NULL) && (temp->receiver->fd != fd))
    {
        temp = temp->next;
    }

    return temp;
}

/** @brief Find connection with a specific \a id in the connection list.
 *
 * There can be only one event per \a fd. We can then find a specific connection
 * based on this \a fd. That allows to check if a specific \a fd has already been
 * registered.
 *
 * @param ev The event handler structure where the list of connection is.
 * @param id The identifier of the connection to be found.
 *
 * @return The found connection pointer, NULL otherwise.
 */
DltConnection *dlt_event_handler_find_connection_by_id(DltEventHandler *ev,
                                                       DltConnectionId id)
{

    DltConnection *temp = ev->connections;

    while ((temp != NULL) && (temp->id != id))
    {
        temp = temp->next;
    }

    return temp;
}

/** @brief Remove a connection from the list and destroy it.
 *
 * This function will first look for the connection in the event handler list,
 * remove it from the list and then destroy it.
 *
 * @param ev The event handler structure where the list of connection is.
 * @param to_remove The connection to remove from the list.
 *
 * @return 0 on success, -1 if the connection is not found.
 */
STATIC int dlt_daemon_remove_connection(DltEventHandler *ev,
                                       DltConnection *to_remove)
{
    if (ev == NULL || to_remove == NULL)
    {
        return DLT_RETURN_ERROR;
    }
    DltConnection **curr = &ev->connections;

    /* Find the address where to_remove value is registered */
    while (*curr && (*curr != to_remove))
    {
        curr = &(*curr)->next;
    }

    if (!*curr)
    {
        /* Must not be possible as we check for existence before */
        dlt_log(LOG_CRIT, "Connection not found for removal.\n");
        return -1;
    }

    /* Replace the content of the address by the next value */
    *curr = (*curr)->next;

    /* Now we can destroy our pointer */
    dlt_connection_destroy(to_remove);

    return 0;
}

/** @brief Destroy the connection list.
 *
 * This function runs through the connection list and destroy them one by one.
 *
 * @param ev Pointer to the event handler structure.
 */
void dlt_event_handler_cleanup_connections(DltEventHandler *ev)
{
    if (ev == NULL)
    {
        /* Nothing to do. */
        return;
    }

    while (ev->connections != NULL)
    {
        /* We don really care on failure */
        (void)dlt_daemon_remove_connection(ev, ev->connections);
    }
}

/** @brief Add a new connection to the list.
 *
 * The connection is added at the tail of the list.
 *
 * @param ev The event handler structure where the connection list is.
 * @param connection The connection to be added.
 */
STATIC void dlt_daemon_add_connection(DltEventHandler *ev,
                                     DltConnection *connection)
{

    DltConnection **temp = &ev->connections;

    while (*temp != NULL)
    {
        temp = &(*temp)->next;
    }
    *temp = connection;
}

/** @brief Check for connection activation
 *
 * If the connection is active and it's not allowed anymore or it the user
 * ask for deactivation, the connection will be deactivated.
 * If the connection is inactive, the user asks for activation and it's
 * allowed for it to be activated, the connection will be activated.
 *
 * @param evhdl The event handler structure.
 * @param con The connection to act on
 * @param activation_type The type of activation requested ((DE)ACTIVATE)
 *
 * @return 0 on success, -1 otherwise
 */
int dlt_connection_check_activate(DltEventHandler *evhdl,
                                  DltConnection *con,
                                  int activation_type)
{
    char local_str[DLT_DAEMON_TEXTBUFSIZE] = { '\0' };

    if (!evhdl || !con || !con->receiver)
    {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "%s: wrong parameters (%p %p).\n",
                 __func__,
                 evhdl,
                 con);
            dlt_log(LOG_ERR, local_str);
            return -1;
    }

    switch (con->status)
    {
    case ACTIVE:
        if (activation_type == DEACTIVATE)
        {
            snprintf(local_str,
                     DLT_DAEMON_TEXTBUFSIZE,
                     "Deactivate connection type: %d\n",
                     con->type);
            dlt_log(LOG_INFO, local_str);

            if (epoll_ctl(evhdl->epfd,
                          EPOLL_CTL_DEL,
                          con->receiver->fd,
                          NULL) == -1)
            {
                dlt_log(LOG_ERR, "epoll_ctl() in deactivate failed!\n");
                return -1;
            }

            con->status = INACTIVE;
        }
        break;
    case INACTIVE:
        if (activation_type == ACTIVATE)
        {
            struct epoll_event ev; /* Content will be copied by the kernel */
            ev.events = con->ev_mask;
            ev.data.ptr = (void *)con->id;

            snprintf(local_str,
                     DLT_DAEMON_TEXTBUFSIZE,
                     "Activate connection type: %d\n",
                     con->type);
            dlt_log(LOG_INFO, local_str);


            if (epoll_ctl(evhdl->epfd,
                          EPOLL_CTL_ADD,
                          con->receiver->fd,
                          &ev) == -1)
            {
                dlt_log(LOG_ERR, "epoll_ctl() in activate failed!\n");
                return -1;
            }
            con->status = ACTIVE;
        }
        break;
    default:
            snprintf(local_str,
                     DLT_DAEMON_TEXTBUFSIZE,
                     "Unknown connection status: %d\n",
                     con->status);
            dlt_log(LOG_ERR, local_str);
            return -1;
    }

    return 0;
}

/** @brief Registers a connection for event handling and takes its ownership.
 *
 * As we add the connection to the list of connection, we take its ownership.
 * That's the only place where the connection pointer is stored.
 * The connection is then used to create a new event trigger.
 * If the connection is of type DLT_CONNECTION_CLIENT_MSG_TCP, we increase
 * the daemon_local->client_connections counter. TODO: Move this counter inside
 * the event handler structure.
 *
 * @param evhdl The event handler structure where the connection list is.
 * @param daemon_local Structure containing needed information.
 * @param connection The connection to be registered.
 * @param mask The bit mask of event to be registered.
 *
 * @return 0 on success, -1 otherwise.
 */
int dlt_event_handler_register_connection(DltEventHandler *evhdl,
                                         DltDaemonLocal *daemon_local,
                                         DltConnection *connection,
                                         int mask)
{
    if (!evhdl || !connection || !connection->receiver) {
        dlt_log(LOG_ERR, "Wrong parameters when registering connection.\n");
        return -1;
    }

    dlt_daemon_add_connection(evhdl, connection);

    if ((connection->type == DLT_CONNECTION_CLIENT_MSG_TCP) ||
        (connection->type == DLT_CONNECTION_CLIENT_MSG_SERIAL))
    {
        daemon_local->client_connections++;
    }

    /* On creation the connection is not active by default */
    connection->status = INACTIVE;

    connection->next = NULL;
    connection->ev_mask = mask;

    return dlt_connection_check_activate(evhdl,
                                         connection,
                                         ACTIVATE);
}

/** @brief Unregisters a connection from the event handler and destroys it.
 *
 * We first look for the connection to be unregistered, delete the event
 * corresponding and then destroy the connection.
 * If the connection is of type DLT_CONNECTION_CLIENT_MSG_TCP, we decrease
 * the daemon_local->client_connections counter. TODO: Move this counter inside
 * the event handler structure.
 *
 * @param evhdl The event handler structure where the connection list is.
 * @param daemon_local Structure containing needed information.
 * @param fd The file descriptor of the connection to be unregistered.
 *
 * @return 0 on success, -1 otherwise.
 */
int dlt_event_handler_unregister_connection(DltEventHandler *evhdl,
                                           DltDaemonLocal *daemon_local,
                                           int fd)
{
    if (evhdl == NULL || daemon_local == NULL)
    {
        return DLT_RETURN_ERROR;
    }

    /* Look for the pointer in the client list.
     * There shall be only one event handler with the same fd.
     */
    DltConnection *temp = dlt_event_handler_find_connection(evhdl, fd);

    if (!temp)
    {
        dlt_log(LOG_ERR, "Connection not found for unregistration.\n");
        return -1;
    }

    if ((temp->type == DLT_CONNECTION_CLIENT_MSG_TCP) ||
        (temp->type == DLT_CONNECTION_CLIENT_MSG_SERIAL))
    {
        daemon_local->client_connections--;

        if (daemon_local->client_connections < 0)
        {
            daemon_local->client_connections = 0;
            dlt_log(LOG_CRIT, "Unregistering more client than registered!\n");
        }
    }

    if (dlt_connection_check_activate(evhdl,
                                      temp,
                                      DEACTIVATE) < 0)
    {
        dlt_log(LOG_ERR, "Unable to unregister event.\n");
    }

    /* Cannot fail as far as dlt_daemon_find_connection succeed */
    return dlt_daemon_remove_connection(evhdl, temp);
}
