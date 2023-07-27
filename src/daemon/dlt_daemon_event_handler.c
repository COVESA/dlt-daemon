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
 * \file dlt_daemon_event_handler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <poll.h>
#include <syslog.h>

#include "dlt_common.h"

#include "dlt-daemon.h"
#include "dlt-daemon_cfg.h"
#include "dlt_daemon_common.h"
#include "dlt_daemon_connection.h"
#include "dlt_daemon_connection_types.h"
#include "dlt_daemon_event_handler.h"
#include "dlt_daemon_event_handler_types.h"

/**
 * \def DLT_EV_TIMEOUT_MSEC
 * The maximum amount of time to wait for a poll event.
 * Set to 1 second to avoid unnecessary wake ups.
 */
#define DLT_EV_TIMEOUT_MSEC 1000
#define DLT_EV_BASE_FD      16

#define DLT_EV_MASK_REJECTED (POLLERR | POLLNVAL)

/** @brief Initialize a pollfd structure
 *
 * That ensures that no event will be mis-watched.
 *
 * @param pfd The element to initialize
 */
static void init_poll_fd(struct pollfd *pfd)
{
    pfd->fd = -1;
    pfd->events = 0;
    pfd->revents = 0;
}

/** @brief Prepare the event handler
 *
 * This will create the base poll file descriptor list.
 *
 * @param ev The event handler to prepare.
 *
 * @return 0 on success, -1 otherwise.
 */
int dlt_daemon_prepare_event_handling(DltEventHandler *ev)
{
    int i = 0;

    if (ev == NULL)
        return DLT_RETURN_ERROR;

    ev->pfd = calloc(DLT_EV_BASE_FD, sizeof(struct pollfd));

    if (ev->pfd == NULL) {
        dlt_log(LOG_CRIT, "Creation of poll instance failed!\n");
        return -1;
    }

    for (i = 0; i < DLT_EV_BASE_FD; i++)
        init_poll_fd(&ev->pfd[i]);

    ev->nfds = 0;
    ev->max_nfds = DLT_EV_BASE_FD;

    return 0;
}

/** @brief Enable a file descriptor to be watched
 *
 * Adds a file descriptor to the descriptor list. If the list is to small,
 * increase its size.
 *
 * @param ev The event handler structure, containing the list
 * @param fd The file descriptor to add
 * @param mask The mask of event to be watched
 */
static void dlt_event_handler_enable_fd(DltEventHandler *ev, int fd, int mask)
{
    if (ev->max_nfds <= ev->nfds) {
        int i = ev->nfds;
        int max = 2 * ev->max_nfds;
        struct pollfd *tmp = realloc(ev->pfd, max * sizeof(*ev->pfd));

        if (!tmp) {
            dlt_log(LOG_CRIT,
                    "Unable to register new fd for the event handler.\n");
            return;
        }

        ev->pfd = tmp;
        ev->max_nfds = max;

        for (; i < max; i++)
            init_poll_fd(&ev->pfd[i]);
    }

    ev->pfd[ev->nfds].fd = fd;
    ev->pfd[ev->nfds].events = mask;
    ev->nfds++;
}

/** @brief Disable a file descriptor for watching
 *
 * The file descriptor is removed from the descriptor list, the list is
 * compressed during the process.
 *
 * @param ev The event handler structure containing the list
 * @param fd The file descriptor to be removed
 */
static void dlt_event_handler_disable_fd(DltEventHandler *ev, int fd)
{
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int nfds = ev->nfds;

    for (; i < nfds; i++, j++) {
        if (ev->pfd[i].fd == fd) {
            init_poll_fd(&ev->pfd[i]);
            j++;
            ev->nfds--;
        }

        if (i == j)
            continue;

        /* Compressing the table */
        if (i < ev->nfds) {
            ev->pfd[i].fd = ev->pfd[j].fd;
            ev->pfd[i].events = ev->pfd[j].events;
            ev->pfd[i].revents = ev->pfd[j].revents;
        }
        else {
            init_poll_fd(&ev->pfd[i]);
        }
    }
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
    int ret = 0;
    unsigned int i = 0;
    int (*callback)(DltDaemon *, DltDaemonLocal *, DltReceiver *, int) = NULL;

    if ((pEvent == NULL) || (daemon == NULL) || (daemon_local == NULL))
        return DLT_RETURN_ERROR;

    ret = poll(pEvent->pfd, pEvent->nfds, DLT_EV_TIMEOUT_MSEC);

    if (ret <= 0) {
        /* We are not interested in EINTR has it comes
         * either from timeout or signal.
         */
        if (errno == EINTR)
            ret = 0;

        if (ret < 0)
            dlt_vlog(LOG_CRIT, "poll() failed: %s\n", strerror(errno));

        return ret;
    }

    for (i = 0; i < pEvent->nfds; i++) {
        int fd = 0;
        DltConnection *con = NULL;
        DltConnectionType type = DLT_CONNECTION_TYPE_MAX;

        if (pEvent->pfd[i].revents == 0)
            continue;

        con = dlt_event_handler_find_connection(pEvent, pEvent->pfd[i].fd);

        if (con && con->receiver) {
            type = con->type;
            fd = con->receiver->fd;
        }
        else { /* connection might have been destroyed in the meanwhile */
            dlt_event_handler_disable_fd(pEvent, pEvent->pfd[i].fd);
            continue;
        }

        /* First of all handle error events */
        if (pEvent->pfd[i].revents & DLT_EV_MASK_REJECTED) {
            /* An error occurred, we need to clean-up the concerned event
             */
            if (type == DLT_CONNECTION_CLIENT_MSG_TCP)
                /* To transition to BUFFER state if this is final TCP client connection,
                 * call dedicated function. this function also calls
                 * dlt_event_handler_unregister_connection() inside the function.
                 */
                dlt_daemon_close_socket(fd, daemon, daemon_local, 0);
            else
                dlt_event_handler_unregister_connection(pEvent,
                                                        daemon_local,
                                                        fd);

            continue;
        }

        /* Get the function to be used to handle the event */
        callback = dlt_connection_get_callback(con);

        if (!callback) {
            dlt_vlog(LOG_CRIT, "Unable to find function for %u handle type.\n",
                     type);
            return -1;
        }

        /* From now on, callback is correct */
        if (callback(daemon,
                     daemon_local,
                     con->receiver,
                     daemon_local->flags.vflag) == -1) {
            dlt_vlog(LOG_CRIT, "Processing from %u handle type failed!\n",
                     type);
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
DltConnection *dlt_event_handler_find_connection(DltEventHandler *ev, int fd)
{
    DltConnection *temp = ev->connections;

    while (temp != NULL) {
        if ((temp->receiver != NULL) && (temp->receiver->fd == fd))
            return temp;
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
DLT_STATIC int dlt_daemon_remove_connection(DltEventHandler *ev,
                                            DltConnection *to_remove)
{
    if ((ev == NULL) || (to_remove == NULL))
        return DLT_RETURN_ERROR;

    DltConnection *curr = ev->connections;
    DltConnection *prev = curr;

    /* Find the address where to_remove value is registered */
    while (curr && (curr != to_remove)) {
        prev = curr;
        curr = curr->next;
    }

    if (!curr) {
        /* Must not be possible as we check for existence before */
        dlt_log(LOG_CRIT, "Connection not found for removal.\n");
        return -1;
    }
    else if (curr == ev->connections)
    {
        ev->connections = curr->next;
    }
    else {
        prev->next = curr->next;
    }

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
    unsigned int i = 0;

    if (ev == NULL)
        /* Nothing to do. */
        return;

    while (ev->connections != NULL)
        /* We don really care on failure */
        (void)dlt_daemon_remove_connection(ev, ev->connections);

    for (i = 0; i < ev->nfds; i++)
        init_poll_fd(&ev->pfd[i]);

    free(ev->pfd);
}

/** @brief Add a new connection to the list.
 *
 * The connection is added at the tail of the list.
 *
 * @param ev The event handler structure where the connection list is.
 * @param connection The connection to be added.
 */
DLT_STATIC void dlt_daemon_add_connection(DltEventHandler *ev,
                                          DltConnection *connection)
{

    DltConnection **temp = &ev->connections;

    while (*temp != NULL)
        temp = &(*temp)->next;

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
    if (!evhdl || !con || !con->receiver) {
        dlt_vlog(LOG_ERR, "%s: wrong parameters.\n", __func__);
        return -1;
    }

    switch (con->status) {
    case ACTIVE:

        if (activation_type == DEACTIVATE) {
            dlt_vlog(LOG_INFO, "Deactivate connection type: %u\n", con->type);

            dlt_event_handler_disable_fd(evhdl, con->receiver->fd);

            if (con->type == DLT_CONNECTION_CLIENT_CONNECT)
                con->receiver->fd = -1;

            con->status = INACTIVE;
        }

        break;
    case INACTIVE:

        if (activation_type == ACTIVATE) {
            dlt_vlog(LOG_INFO, "Activate connection type: %u\n", con->type);

            dlt_event_handler_enable_fd(evhdl,
                                        con->receiver->fd,
                                        con->ev_mask);

            con->status = ACTIVE;
        }

        break;
    default:
        dlt_vlog(LOG_ERR, "Unknown connection status: %u\n", con->status);
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
        daemon_local->client_connections++;

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
    if ((evhdl == NULL) || (daemon_local == NULL))
        return DLT_RETURN_ERROR;

    /* Look for the pointer in the client list.
     * There shall be only one event handler with the same fd.
     */
    DltConnection *temp = dlt_event_handler_find_connection(evhdl, fd);

    if (!temp) {
        dlt_log(LOG_ERR, "Connection not found for unregistration.\n");
        return -1;
    }

    if ((temp->type == DLT_CONNECTION_CLIENT_MSG_TCP) ||
        (temp->type == DLT_CONNECTION_CLIENT_MSG_SERIAL)) {
        daemon_local->client_connections--;

        if (daemon_local->client_connections < 0) {
            daemon_local->client_connections = 0;
            dlt_log(LOG_CRIT, "Unregistering more client than registered!\n");
        }
    }

    if (dlt_connection_check_activate(evhdl,
                                      temp,
                                      DEACTIVATE) < 0)
        dlt_log(LOG_ERR, "Unable to unregister event.\n");

    /* Cannot fail as far as dlt_daemon_find_connection succeed */
    return dlt_daemon_remove_connection(evhdl, temp);
}
