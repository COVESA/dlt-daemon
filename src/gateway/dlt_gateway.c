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
 * \file dlt_gateway.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <limits.h>
#include <errno.h>
#include "dlt_gateway.h"
#include "dlt_config_file_parser.h"
#include "dlt_common.h"
#include "dlt-daemon_cfg.h"
#include "dlt_daemon_event_handler.h"
#include "dlt_daemon_connection.h"
#include "dlt_daemon_client.h"

/**
 * Expected entries for a passive node configuration
 * Caution: after changing entries here,
 * dlt_gateway_check_params need to be updated as well
 * */
char *configuration_entries [] =
{
    "IPaddress",
    "Port",
    "EcuID",
    "Connect",
    "Timeout",
    "SendControl"
};

#define DLT_GATEWAY_NUM_PROPERTIES_MAX 6

/**
 * Check if given string is a valid IP address
 *
 * @param ip    IP address
 * @param value string to be tested
 * @return 0 on success, -1 otherwise
 */
int dlt_gateway_check_ip(char **ip, char *value)
{
    struct sockaddr_in sa;
    int ret = -1;

    if (ip == NULL || value == NULL)
    {
        return -1;
    }

    ret = inet_pton(AF_INET, value, &(sa.sin_addr));

    /* valid IP address */
    if (ret != 0)
    {
        *ip = strdup(value);

        if (*ip == NULL)
        {
            dlt_log(LOG_ERR, "Cannot copy passive node IP address string\n");
            return -1;
        }

        return 0;
    }
    else
    {
        dlt_log(LOG_ERR, "IP address is not valid\n");
    }

    return -1;
}

/**
 * Check socket domain
 *
 * This function is currently not used; only AF_INET is supported.
 *
 * @param domain    socket domain
 * @param value     string to be tested
 * @return 0 on success, -1 otherwise
 */
int dlt_gateway_check_domain(int *domain, char *value)
{
    int ret = -1;

    if (domain == NULL || value == NULL)
    {
        return -1;
    }

    ret = strncmp(value, "AF_INET", strlen("AF_INET"));

    if (ret == 0)
    {
        *domain = AF_INET;
        return 0;
    }
    else
    {
        dlt_log(LOG_ERR, "Only socket domain AF_INET supported\n");
    }

    return -1;
}

/**
 * Check socket type
 *
 * This function is currently not used; only SOCK_STREAM is supported.
 *
 * @param type      socket type
 * @param value     string to be tested
 * @return 0 on success, -1 otherwise
 */
int dlt_gateway_check_type(int *type, char *value)
{
    int ret = -1;

    if (type == NULL || value == NULL)
    {
        return -1;
    }

    ret = strncmp(value, "SOCK_STREAM", strlen("SOCK_STREAM"));

    if (ret == 0)
    {
        *type = SOCK_STREAM;
        return 0;
    }
    else
    {
        dlt_log(LOG_ERR, "Only socket type SOCK_STREAM is supported\n");
    }

    return -1;
}

/**
 * Check socket protocol
 *
 * This function is currently not used; only IPPROTO_TCP is supported.
 *
 * @param protocol  socket protocol
 * @param value     string to be tested
 * @return 0 on success, -1 otherwise
 */
int dlt_gateway_check_protocol(int *protocol, char *value)
{
    if (protocol == NULL || value == NULL)
    {
        return -1;
    }

    if (strncmp(value, "IPPROTO_TCP", strlen("IPPROTO_TCP")) == 0)
    {
        *protocol = IPPROTO_TCP;
        return 0;
    }
    else
    {
        dlt_log(LOG_ERR, "Only protocol IPPROTO_TCP is supported\n");
    }

    return -1;
}

/**
 * Check port number
 *
 * @param port      port number
 * @param value     string to be tested
 * @return 0 on success, -1 otherwise
 */
int dlt_gateway_check_port(int *port, char *value)
{
    int tmp = -1;

    if (port == NULL || value == NULL)
    {
        return -1;
    }

    tmp = (int) strtol(value, NULL, 10);

    /* port ranges for unprivileged applications */
    if (tmp > IPPORT_RESERVED && tmp <= USHRT_MAX)
    {
        *port = tmp;
        return 0;
    }
    else
    {
        dlt_log(LOG_ERR, "Port number is invalid\n");
    }

    return -1;
}

/**
 * Check ECU name
 *
 * @param ecuid     ecu name
 * @param value     string to be used as ECU identifier
 * @return 0 on success, -1 otherwise
 */
int dlt_gateway_check_ecu(char **ecuid, char *value)
{
    if (ecuid == NULL || value == NULL)
    {
        return -1;
    }

    *ecuid = strdup(value);

    if (*ecuid == NULL)
    {
        return -1;
    }

    return 0;
}

/**
 * Check connection trigger
 *
 * @param trigger   connection setup trigger
 * @param value     string to be tested
 * @return 0 on success, -1 otherwise
 */
int dlt_gateway_check_connect_trigger(int *trigger, char *value)
{
    if (trigger == NULL || value == NULL)
    {
        return -1;
    }

    if (strncasecmp(value, "OnStartup", strlen("OnStartup")) == 0)
    {
        *trigger = DLT_GATEWAY_ON_STARTUP;
    }
    else if (strncasecmp(value, "OnDemand", strlen("OnDemand")) == 0)
    {
        *trigger = DLT_GATEWAY_ON_DEMAND;
    }
    else
    {
        dlt_log(LOG_ERR, "Wrong connection trigger state given.\n");
        *trigger = DLT_GATEWAY_UNDEFINED;
        return -1;
    }

    return 0;
}

/**
 * Check connection timeout value
 *
 * @param timeout   connection timeout
 * @param value     string to be tested
 * @return 0 on success, -1 otherwise
 */
int dlt_gateway_check_timeout(int *timeout, char *value)
{
    if (timeout == NULL || value == NULL)
    {
        return -1;
    }

    *timeout = (int) strtol(value, NULL, 10);

    if (*timeout > 0)
    {
        return 0;
    }

    return -1;
}

/**
 * Check the specified control messages identifier
 *
 * @param ids   ids of control messages to be send after connection is established
 * @param value string to be tested
 * @return 0 on success, -1 otherwise
 */
int dlt_gateway_check_control_messages(int *ids, char *value)
{
    /* list of allowed clients given */
    char *token = NULL;
    char *rest = NULL;
    int i = 0;
    char error_msg[DLT_DAEMON_TEXTBUFSIZE];

    if (ids == NULL || value == NULL)
    {
        return -1;
    }

    if (strlen(value) == 0)
    {
        memset(ids, 0, sizeof(int) * DLT_GATEWAY_MAX_STARTUP_CTRL_MSG);
        return 0;
    }

    token = strtok_r(value, ",", &rest);
    while (token != NULL && i < DLT_GATEWAY_MAX_STARTUP_CTRL_MSG)
    {

        ids[i] = strtol(token, NULL, 16);

        if (errno == EINVAL || errno == ERANGE)
        {
            snprintf(error_msg,
                     DLT_DAEMON_TEXTBUFSIZE-1,
                     "Control message ID is not an integer: %s\n", token);
            dlt_log(LOG_ERR, error_msg);
            return -1;
        }
        else if (ids[i] < DLT_SERVICE_ID_SET_LOG_LEVEL ||
                 ids[i] >= DLT_SERVICE_ID_LAST_ENTRY)
        {
            snprintf(error_msg,
                     DLT_DAEMON_TEXTBUFSIZE-1,
                     "Control message ID is not valid: %s\n", token);
            dlt_log(LOG_ERR, error_msg);
            return -1;
        }

        token = strtok_r(NULL, ",", &rest);
        i++;
    }

    return 0;
}

/**
 * Check if gateway connection configuration parameter is valid.
 *
 * @param g     DltGateway
 * @param c     DltGatewayConnection
 * @param key   DltGatwayConnection property
 * @param value specified property value from configuration file
 * @return 0 on success, -1 otherwise
 */
int dlt_gateway_check_param(DltGateway *gateway,
                            DltGatewayConnection *con,
                            char *key,
                            char *value)
{
    if (gateway == NULL || con == NULL || key == NULL || value == NULL)
    {
        return -1;
    }

    if (strncmp(key, "IPaddress", sizeof("IPaddress")) == 0)
    {
        return dlt_gateway_check_ip(&con->ip_address, value);
    }
    else if (strncmp(key, "SockDomain", sizeof("SockDomain")) == 0)
    {
        return dlt_gateway_check_domain(&(con->sock_domain), value);
    }
    else if (strncmp(key, "SockType", sizeof("SockType")) == 0)
    {
        return dlt_gateway_check_type(&(con->sock_type), value);
    }
    else if (strncmp(key, "SockProtocol", sizeof("SockProtocol")) == 0)
    {
        return dlt_gateway_check_protocol(&(con->sock_protocol), value);
    }
    else if (strncmp(key, "Port", sizeof("Port")) == 0)
    {
        return dlt_gateway_check_port(&(con->port), value);
    }
    else if (strncmp(key, "EcuID", sizeof("EcuID")) == 0)
    {
        return dlt_gateway_check_ecu(&con->ecuid,value);
    }
    else if (strncmp(key, "Connect", sizeof("Connect")) == 0)
    {
        return dlt_gateway_check_connect_trigger(&(con->trigger), value);
    }
    else if (strncmp(key, "Timeout", sizeof("Timeout")) == 0)
    {
        return dlt_gateway_check_timeout(&(con->timeout), value);
    }
    else if (strncmp(key, "SendControl", sizeof("SendControl")) == 0)
    {
        return dlt_gateway_check_control_messages(con->control_msgs, value);
    }
    else
    {
        return -1;
    }
}

/**
 * Store gateway connection in internal data structure
 *
 * @param g     DltGatway
 * @param tmp   DltGatewayConnection
 * @param verbose verbose flag
 * @return 0 on success, -1 otherwise
 */
int dlt_gateway_store_connection(DltGateway *gateway,
                                 DltGatewayConnection *tmp,
                                 int verbose)
{
    int i = 0;
    int ret = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (gateway == NULL || tmp == NULL)
    {
        return -1;
    }

    /* find next free entry in connection array */
    while (i < gateway->num_connections)
    {
        if (gateway->connections[i].status == DLT_GATEWAY_UNINITIALIZED)
        {
            break;
        }

        i++;
    }

    if (&gateway->connections[i] == NULL)
    {
        return -1;
    }

    /* store values */
    gateway->connections[i].ip_address = strdup(tmp->ip_address);
    gateway->connections[i].ecuid = strdup(tmp->ecuid);
    gateway->connections[i].sock_domain = tmp->sock_domain;
    gateway->connections[i].sock_type = tmp->sock_type;
    gateway->connections[i].sock_protocol = tmp->sock_protocol;
    gateway->connections[i].port = tmp->port;
    gateway->connections[i].trigger = tmp->trigger;
    gateway->connections[i].timeout = tmp->timeout;
    gateway->connections[i].handle = 0;
    gateway->connections[i].status = DLT_GATEWAY_INITIALIZED;
    memcpy(gateway->connections[i].control_msgs,
           tmp->control_msgs,
           sizeof(tmp->control_msgs));

    if (dlt_client_init_port(&gateway->connections[i].client,
                             gateway->connections[i].port,
                             verbose) != 0)
    {
        free(gateway->connections[i].ip_address);
        free(gateway->connections[i].ecuid);
        dlt_log(LOG_CRIT, "dlt_client_init() failed for gateway connection\n");
        return -1;
    }
    dlt_receiver_init(&gateway->connections[i].client.receiver,
                      gateway->connections[i].client.sock,
                      DLT_DAEMON_RCVBUFSIZESOCK);
    /* setup DltClient Structure */
    gateway->connections[i].client.servIP =
        strdup(gateway->connections[i].ip_address);
    // TODO: modify DltClient structure to make port configurable
    // g->connections[i].client.servPort = g->connections[i].port;

    if (ret != 0)
    {
        free(gateway->connections[i].ip_address);
        free(gateway->connections[i].ecuid);
        dlt_log(LOG_ERR, "Gateway: DltClient initialization failed\n");
        return -1;
    }

    return 0;
}

/**
 * Read configuration file and initialize connection data structures
 *
 * @param g DltGateway
 * @param verbose verbose flag
 * @return 0 on success, -1 otherwise
 */
int dlt_gateway_configure(DltGateway *gateway, int verbose)
{
    int ret = 0;
    int i = 0;
    DltGatewayConnection tmp;
    DltConfigFile *file = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (gateway == NULL)
    {
        return -1;
    }

    memset(&tmp, 0, sizeof(tmp));

    /* read configuration file */
    file = dlt_config_file_init(DLT_GATEWAY_CONFIG_PATH);

    /* get number of entries and allocate memory to store information */
    ret = dlt_config_file_get_num_sections(file, &gateway->num_connections);

    if (ret != 0)
    {
        dlt_config_file_release(file);
        dlt_log(LOG_ERR, "Invalid number of sections in configuration file\n");
        return -1;
    }

    gateway->connections = calloc(sizeof(DltGatewayConnection),
                                  gateway->num_connections);

    if (gateway->connections == NULL)
    {
        dlt_config_file_release(file);
        dlt_log(LOG_CRIT, "Memory allocation for gateway connections failed\n");
        return -1;
    }

    for (i = 0; i < gateway->num_connections; i++)
    {
        int j = 0;
        char section[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = {'\0'};
        char value[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = {'\0'};
        int is_node_valid = 1; /* 1 - valid, <= 0 invalid */

        ret = dlt_config_file_get_section_name(file, i, section);

        for (j = 0; j < DLT_GATEWAY_NUM_PROPERTIES_MAX; j++)
        {
            ret = dlt_config_file_get_value(file,
                                            section,
                                            configuration_entries[j],
                                            value);

            if (ret != 0)
            {
                is_node_valid = -1;
                break;
            }

            /* check value and store temporary */
            ret = dlt_gateway_check_param(gateway,
                                          &tmp,
                                          configuration_entries[j],
                                          value);

            if (ret != 0)
            {
                char error_str[DLT_DAEMON_TEXTBUFSIZE] = {'\0'};
                sprintf(error_str,
                        "Configuration %s = %s is invalid\n",
                        configuration_entries[j], value);
                dlt_log(LOG_ERR, error_str);
                is_node_valid = -1;
                break;
            }
        }

        if (is_node_valid >= 1)
        {
            ret = dlt_gateway_store_connection(gateway, &tmp, verbose);

            if (ret != 0)
            {
                dlt_log(LOG_ERR, "Storing gateway connection data failed\n");
            }
        }

        /* strdup used inside some get_value function */
        free(tmp.ecuid);
        free(tmp.ip_address);
    }

    dlt_config_file_release(file);
    return ret;
}

int dlt_gateway_init(DltDaemonLocal *daemon_local, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon_local == NULL)
    {
        return -1;
    }

    DltGateway *gateway = &daemon_local->pGateway;

    if (gateway != NULL)
    {
        if (dlt_gateway_configure(gateway, verbose) != 0)
        {
            dlt_log(LOG_ERR, "Gateway initialization failed\n");
            return -1;
        }
    }
    else
    {
        dlt_log(LOG_CRIT, "Pointer to Gateway structure is NULL\n");
        return -1;
    }

    /* ignore return value */
    dlt_gateway_establish_connections(gateway, daemon_local, verbose);

    return 0;
}

void dlt_gateway_deinit(DltGateway *gateway, int verbose)
{
    int i = 0;

    if (gateway == NULL)
    {
        return;
    }

    PRINT_FUNCTION_VERBOSE(verbose);

    for (i = 0; i < gateway->num_connections; i++)
    {
        DltGatewayConnection *c = &gateway->connections[i];
        dlt_client_cleanup(&c->client, verbose);
        free(c->ip_address);
        free(c->ecuid);
    }

    free(gateway->connections);
    free(gateway);
}

int dlt_gateway_establish_connections(DltGateway *gateway,
                                      DltDaemonLocal *daemon_local,
                                      int verbose)
{
    int i = 0;
    int ret = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (gateway == NULL || daemon_local == NULL)
    {
        return -1;
    }

    for (i = 0; i < gateway->num_connections; i++)
    {
        DltGatewayConnection *con = &(gateway->connections[i]);
        if (con == NULL)
        {
            dlt_log(LOG_CRIT, "Cannot retrieve gateway connection details\n");
            return -1;
        }

        if (con->status != DLT_GATEWAY_CONNECTED &&
            con->trigger != DLT_GATEWAY_ON_DEMAND &&
            con->trigger != DLT_GATEWAY_DISABLED)
        {
            ret = dlt_client_connect(&con->client, verbose);

            if (ret == 0)
            {
                /* connection to passive node established, add to event loop */
                con->status = DLT_GATEWAY_CONNECTED;
                con->reconnect_cnt = 0;
                con->timeout_cnt = 0;

                /* setup dlt connection and add to epoll event loop here */
                if (dlt_connection_create(daemon_local,
                                     &daemon_local->pEvent,
                                     con->client.sock,
                                     EPOLLIN,
                                     DLT_CONNECTION_GATEWAY) != 0)
                {
                    dlt_log(LOG_ERR, "Gateway connection creation failed\n");
                    return -1;
                }

                /* immediately send configured control messages */
                dlt_gateway_send_control_message(con,
                                                 gateway,
                                                 daemon_local,
                                                 verbose);

            }
            else
            {
                dlt_log(LOG_DEBUG,
                        "Passive Node is not up. Connection failed.\n");

                con->timeout_cnt++;
                if (con->timeout_cnt > con->timeout)
                {
                    con->trigger = DLT_GATEWAY_DISABLED;
                    dlt_log(LOG_WARNING,
                            "Passive Node connection retry timed out. "
                            "Give up.\n");
                }
            }
        }
    }

    return 0;
}

DltReceiver *dlt_gateway_get_connection_receiver(DltGateway *gateway, int fd)
{
    int i = 0;

    if (gateway == NULL)
    {
        return NULL;
    }

    for (i = 0; i < gateway->num_connections; i++)
    {
        DltGatewayConnection *c = &gateway->connections[i];
        if (c->status == DLT_GATEWAY_CONNECTED && c->client.sock == fd)
        {
            return &c->client.receiver;
        }
    }

    return NULL;
}

int dlt_gateway_process_passive_node_messages(DltDaemon *daemon,
                                              DltDaemonLocal *daemon_local,
                                              DltReceiver *receiver,
                                              int verbose)
{
    int i = 0;
    DltGateway *gateway = NULL;
    DltGatewayConnection *con = NULL;
    DltMessage msg;
    char local_str[DLT_DAEMON_TEXTBUFSIZE];

    if (daemon == NULL || daemon_local == NULL || receiver == NULL)
    {
        return -1;
    }

    PRINT_FUNCTION_VERBOSE(verbose);

    gateway = &daemon_local->pGateway;
    if (gateway == NULL)
    {
        dlt_log(LOG_ERR, "Gateway structure is NULL\n");
        return -1;
    }

    for (i = 0; i < gateway->num_connections; i++)
    {
        if (gateway->connections[i].client.sock == receiver->fd)
        {
            con = &gateway->connections[i];
            break;
        }
    }

    if (con == NULL)
    {
        dlt_log(LOG_ERR, "Cannot associate fd to passive Node connection\n");
        return -1;
    }

    /* now the corresponding passive node connection is available */
    if (dlt_message_init(&msg, verbose) == -1)
    {
        dlt_log(LOG_ERR,
                "Cannot initialize DLT message for passive node forwarding\n");
        return -1;
    }

    /* nearly copy and paste of dlt_client_main_loop function */
    if (dlt_receiver_receive_socket(receiver) <= 0)
    {
        /* No more data to be received */
        if (dlt_message_free(&msg, verbose) < 0)
        {
            dlt_log(LOG_ERR, "Cannot free DLT message\n");
            return -1;
        }

        dlt_log(LOG_WARNING, "Connection to passive node lost\n");

        if (con->reconnect_cnt < DLT_GATEWAY_RECONNECT_MAX)
        {
            dlt_log(LOG_WARNING, "Try to reconnect.\n");
            con->reconnect_cnt += 1;
            con->timeout_cnt = 0;
        }
        else
        {
            con->status = DLT_GATEWAY_DISCONNECTED;
            if (dlt_event_handler_unregister_connection(&daemon_local->pEvent,
                                                       daemon_local,
                                                       receiver->fd,
                                                       DLT_CONNECTION_GATEWAY) != 0)
            {
                dlt_log(LOG_ERR, "Remove passive node Connection failed\n");
            }
        }
        return 0;
    }

    while (dlt_message_read(&msg,
                            (unsigned char *)receiver->buf,
                            receiver->bytesRcvd,
                            0,
                            verbose) == DLT_MESSAGE_ERROR_OK)
    {
        DltStandardHeaderExtra *header = (DltStandardHeaderExtra *)
                                         (msg.headerbuffer +
                                         sizeof(DltStorageHeader) +
                                         sizeof(DltStandardHeader));

        /* only forward messages if the received ECUid is the expected one */
        if (strncmp(header->ecu, con->ecuid, strlen(con->ecuid)) == 0)
        {
            snprintf(local_str,
                     DLT_DAEMON_TEXTBUFSIZE,
                     "Received ECUid (%s) similar to configured ECUid(%s). "
                     "Forwarding message (%s).\n",
                     header->ecu,
                     con->ecuid,
                     msg.databuffer);
            dlt_log(LOG_DEBUG, local_str);

            if (dlt_daemon_client_send(DLT_DAEMON_SEND_TO_ALL,
                                   daemon,
                                   daemon_local,
                                   msg.headerbuffer,
                                   sizeof(DltStorageHeader),
                                   msg.headerbuffer + sizeof(DltStorageHeader),
                                   msg.headersize - sizeof(DltStorageHeader),
                                   msg.databuffer,
                                   msg.datasize,
                                   verbose) != DLT_DAEMON_ERROR_OK)
            {
                dlt_log(LOG_WARNING, "Forward message to clients failed!\n");
            }
        }
        else /* otherwise remove this connection and do not connect again */
        {
            snprintf(local_str,
                     DLT_DAEMON_TEXTBUFSIZE,
                     "Received ECUid (%s) differs to configured ECUid(%s). "
                     "Discard this message.\n",
                     header->ecu,
                     con->ecuid);
                     dlt_log(LOG_WARNING, local_str);

             /* disconnect from passive node */
             con->status = DLT_GATEWAY_DISCONNECTED;
             con->trigger = DLT_GATEWAY_DISABLED;
             if (dlt_event_handler_unregister_connection(&daemon_local->pEvent,
                                                        daemon_local,
                                                        receiver->fd,
                                                        DLT_CONNECTION_GATEWAY)
                 != 0)
             {
                 dlt_log(LOG_ERR, "Remove passive node Connection failed\n");
             }

             dlt_log(LOG_WARNING,
                     "Disconnect from passive node due to invalid ECUid\n");
        }

        if (msg.found_serialheader)
        {
            if (dlt_receiver_remove(receiver,
                                    msg.headersize +
                                    msg.datasize -
                                    sizeof(DltStorageHeader) +
                                    sizeof(dltSerialHeader)) == -1)
            {
                /* Return value ignored */
                dlt_message_free(&msg,verbose);
                return -1;
             }
        }
        else
        {
            if (dlt_receiver_remove(receiver,
                                    msg.headersize +
                                    msg.datasize -
                                    sizeof(DltStorageHeader)) == -1)
            {
                /* Return value ignored */
                dlt_message_free(&msg,verbose);
                return -1;
            }
        }
    }

    if (dlt_receiver_move_to_begin(receiver) == -1)
    {
        /* Return value ignored */
        dlt_message_free(&msg, verbose);
        return -1;
    }

    if (dlt_message_free(&msg, verbose) == -1)
    {
        return -1;
    }

    return 0;
}

int dlt_gateway_process_gateway_timer(DltDaemon *daemon,
                                      DltDaemonLocal *daemon_local,
                                      DltReceiver *receiver,
                                      int verbose)
{
    uint64_t expir = 0;
    ssize_t res = 0;
    char local_str[DLT_DAEMON_TEXTBUFSIZE];

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon_local == NULL) || (daemon == NULL) || (receiver == NULL))
    {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "%s: invalid parameters",
                 __func__);
        dlt_log(LOG_ERR, local_str);
        return -1;
    }

    res = read(receiver->fd, &expir, sizeof(expir));

    if(res < 0)
    {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "%s: Fail to read timer (%s)\n", __func__, strerror(errno));
        dlt_log(LOG_WARNING, local_str);
        /* Activity received on timer_wd, but unable to read the fd:
           let's go on sending notification */
    }

    /* try to connect to passive nodes */
    dlt_gateway_establish_connections(&daemon_local->pGateway,
                                      daemon_local,
                                      verbose);

    dlt_log(LOG_DEBUG, "Gateway Timer\n");

    return 0;
}

int dlt_gateway_forward_control_message(DltGateway *gateway,
                                        DltDaemonLocal *daemon_local,
                                        DltMessage *msg,
                                        char *ecu,
                                        int verbose)
{
    int i = 0;
    int ret = 0;
    DltGatewayConnection *con = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (gateway == NULL || daemon_local == NULL || msg == NULL || ecu == NULL)
    {
        return -1;
    }

    for (i = 0; i < gateway->num_connections; i++)
    {
        if (strncmp(gateway->connections[i].ecuid,
                    ecu,
                    strlen(gateway->connections[i].ecuid)) == 0)
        {
            con = &gateway->connections[i];
            break;
        }
    }

    if (con == NULL)
    {
        dlt_log(LOG_WARNING, "Unknown passive node identifier\n");
        return -1;
    }

    if (con->status != DLT_GATEWAY_CONNECTED)
    {
        dlt_log(LOG_INFO, "Passive node is not connected\n");
        return -1;
    }

    if (daemon_local->flags.lflag) /* send serial header */
    {
        ret = send(con->client.sock,
                   (void *)dltSerialHeader,
                   sizeof(dltSerialHeader),
                   0);

        if (ret == -1)
        {
            dlt_log(LOG_ERR, "Sending message to passive DLT Daemon failed\n");
            return -1;
        }
    }

    ret = send(con->client.sock,
         msg->headerbuffer + sizeof(DltStorageHeader),
         msg->headersize - sizeof(DltStorageHeader),
         0);
    if (ret == -1)
    {
        dlt_log(LOG_ERR, "Sending message to passive DLT Daemon failed\n");
        return -1;
    }
    else
    {
        ret = send(con->client.sock, msg->databuffer, msg->datasize, 0);
        if (ret == -1)
        {
            dlt_log(LOG_ERR, "Sending message to passive DLT Daemon failed\n");
            return -1;
        }
    }

    dlt_log(LOG_INFO, "Control message forwarded\n");
    return 0;
}

int dlt_gateway_process_on_demand_request(DltGateway *gateway,
                                          DltDaemonLocal *daemon_local,
                                          char *node_id,
                                          int connection_status,
                                          int verbose)
{
    int i = 0;
    DltGatewayConnection *con = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (gateway == NULL || daemon_local == NULL || node_id == NULL)
    {
        return -1;
    }

    /* find connection by ECU id */
    for (i = 0; i < gateway->num_connections; i++)
    {
        if (strncmp(node_id, gateway->connections->ecuid, DLT_ID_SIZE) == 0)
        {
            con = &gateway->connections[i];
            break;
        }
    }

    if (con == NULL)
    {
        dlt_log(LOG_WARNING, "Specified ECUid not found\n");
        return -1;
    }

    if (connection_status == 1) /* try to connect */
    {
        if (con->status != DLT_GATEWAY_CONNECTED)
        {
            if (dlt_client_connect(&con->client, verbose) == 0)
            {
                con->status = DLT_GATEWAY_CONNECTED;

                /* setup dlt connection and add to epoll event loop here */
                if (dlt_connection_create(daemon_local,
                                     &daemon_local->pEvent,
                                     con->client.sock,
                                     EPOLLIN,
                                     DLT_CONNECTION_GATEWAY) != 0)
                {
                    dlt_log(LOG_ERR, "Gateway connection creation failed\n");
                    return -1;
                }
            }
            else
            {
                dlt_log(LOG_ERR, "Could not connect to passive node\n");
                return -1;
            }
        }
        else
        {
            dlt_log(LOG_INFO, "Passive node already connected\n");
        }
    }
    else if (connection_status == 0) /* disconnect*/
    {

        con->status = DLT_GATEWAY_DISCONNECTED;
        if (dlt_event_handler_unregister_connection(&daemon_local->pEvent,
                                                   daemon_local,
                                                   con->client.sock,
                                                   DLT_CONNECTION_GATEWAY) != 0)
        {
            dlt_log(LOG_ERR,
                    "Remove passive node event handler connection failed\n");
        }

        if (dlt_client_cleanup(&con->client, verbose) != 0)
        {
            dlt_log(LOG_ERR, "Could not cleanup DltClient structure\n");
            return -1;
        }
    }
    else
    {
        dlt_log(LOG_ERR, "Unknown command (connection_status)\n");
        return -1;
    }

    return 0;
}

void dlt_gateway_send_control_message(DltGatewayConnection *con,
                                      DltGateway *gateway,
                                      DltDaemonLocal *daemon_local,
                                      int verbose)
{
    int i = 0;
    uint32_t len = 0;
    DltMessage msg;
    char local_str[DLT_DAEMON_TEXTBUFSIZE];

    PRINT_FUNCTION_VERBOSE(verbose);

    if (con == NULL || gateway == NULL || daemon_local == NULL)
    {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "%s: Invalid parameter given\n", __func__);
        dlt_log(LOG_WARNING, local_str);
        return;
    }

    for (i = 0; i < DLT_GATEWAY_MAX_STARTUP_CTRL_MSG; i++)
    {
        if (con->control_msgs[i] == 0)
        {
            break; /* no (more) control message to be send */
        }

        memset(&msg, 0, sizeof(msg));

        if (dlt_message_init(&msg, verbose) == -1)
        {
            snprintf(local_str,
                     DLT_DAEMON_TEXTBUFSIZE,
                     "Initialization of message failed\n");
            dlt_log(LOG_WARNING, local_str);
            return;
        }

        if (con->control_msgs[i] == DLT_SERVICE_ID_GET_LOG_INFO)
        {
            DltServiceGetLogInfoRequest *req;
            msg.databuffer = (uint8_t *)
                             malloc(sizeof(DltServiceGetLogInfoRequest));
            if (msg.databuffer == NULL)
            {
                snprintf(local_str,
                         DLT_DAEMON_TEXTBUFSIZE,
                         "Initialization of 'GetLogInfo' failed\n");
                dlt_log(LOG_WARNING, local_str);
                dlt_message_free(&msg, verbose);
                return;
            }

            req = (DltServiceGetLogInfoRequest *)msg.databuffer;
            req->service_id = DLT_SERVICE_ID_GET_LOG_INFO;
            req->options = 7;
            dlt_set_id(req->apid, "");
            dlt_set_id(req->ctid, "");
            dlt_set_id(req->com, "remo");

            msg.databuffersize = sizeof(DltServiceGetLogInfoRequest);
            msg.datasize = msg.databuffersize;
        }
        else if (con->control_msgs[i] == DLT_SERVICE_ID_GET_SOFTWARE_VERSION)
        {
            DltServiceGetSoftwareVersion *req;

            msg.databuffer = (uint8_t *)
                             malloc(sizeof(DltServiceGetSoftwareVersion));
            if (msg.databuffer == NULL)
            {
                snprintf(local_str,
                         DLT_DAEMON_TEXTBUFSIZE,
                         "Initialization of 'GetSoftwareVersion' failed\n");
                dlt_log(LOG_WARNING, local_str);
                dlt_message_free(&msg, verbose);
                return;
            }

            req = (DltServiceGetSoftwareVersion *)msg.databuffer;
            req->service_id = DLT_SERVICE_ID_GET_SOFTWARE_VERSION;

            msg.databuffersize = sizeof(DLT_SERVICE_ID_GET_SOFTWARE_VERSION);
            msg.datasize = msg.databuffersize;
        }
        else
        {
            snprintf(local_str,
                     DLT_DAEMON_TEXTBUFSIZE,
                     "Unknown control message. Skip.\n");
            dlt_log(LOG_WARNING, local_str);
            dlt_message_free(&msg, verbose);
            return;
        }

        /* prepare storage header */
        msg.storageheader = (DltStorageHeader*)msg.headerbuffer;

        if (dlt_set_storageheader(msg.storageheader,"") == DLT_RETURN_ERROR)
        {
            dlt_message_free(&msg,0);
            return;
        }

        /* prepare standard header */
        msg.standardheader = (DltStandardHeader*)(msg.headerbuffer + sizeof(DltStorageHeader));
        msg.standardheader->htyp = DLT_HTYP_WEID | DLT_HTYP_WTMS | DLT_HTYP_UEH | DLT_HTYP_PROTOCOL_VERSION1 ;

        #if (BYTE_ORDER==BIG_ENDIAN)
            msg.standardheader->htyp = (msg.standardheader->htyp | DLT_HTYP_MSBF);
        #endif

        msg.standardheader->mcnt = 0;

        /* Set header extra parameters */
        dlt_set_id(msg.headerextra.ecu,"");
        //msg.headerextra.seid = 0;
        msg.headerextra.tmsp = dlt_uptime();

        /* Copy header extra parameters to headerbuffer */
        if (dlt_message_set_extraparameters(&msg,0) == DLT_RETURN_ERROR)
        {
            dlt_message_free(&msg,0);
            return;
        }

        /* prepare extended header */
        msg.extendedheader = (DltExtendedHeader*)(msg.headerbuffer +
                             sizeof(DltStorageHeader) +
                             sizeof(DltStandardHeader) +
                             DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp) );

        msg.extendedheader->msin = DLT_MSIN_CONTROL_REQUEST;

        msg.extendedheader->noar = 1; /* number of arguments */

        dlt_set_id(msg.extendedheader->apid, "APP");
        dlt_set_id(msg.extendedheader->ctid, "CON");

        /* prepare length information */
        msg.headersize = sizeof(DltStorageHeader) +
                         sizeof(DltStandardHeader) +
                         sizeof(DltExtendedHeader) +
                         DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp);

        len=msg.headersize - sizeof(DltStorageHeader) + msg.datasize;
        if (len>UINT16_MAX)
        {
            fprintf(stderr,"Critical: Huge injection message discarded!\n");
            dlt_message_free(&msg,0);

            return;
        }

        msg.standardheader->len = DLT_HTOBE_16(len);

        /* forward message to passive node */
        if (dlt_gateway_forward_control_message(gateway,
                                                daemon_local,
                                                &msg,
                                                con->ecuid,
                                                verbose) != 0)
        {
            snprintf(local_str,
                     DLT_DAEMON_TEXTBUFSIZE,
                     "Failed to forward message to passive node.\n");
            dlt_log(LOG_WARNING, local_str);
        }

        dlt_message_free(&msg, verbose);
    }
}
