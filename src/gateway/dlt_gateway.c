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
 * Saya Sugiura <ssugiura@jp.adit-jv.com>
 *
 * \copyright Copyright © 2015-2018 Advanced Driver Information Technology. \n
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
#include "dlt_gateway_internal.h"
#include "dlt_config_file_parser.h"
#include "dlt_common.h"
#include "dlt-daemon_cfg.h"
#include "dlt_daemon_common_cfg.h"
#include "dlt_daemon_event_handler.h"
#include "dlt_daemon_connection.h"
#include "dlt_daemon_client.h"
#include "dlt_daemon_offline_logstorage.h"

/**
 * Check if given string is a valid IP address
 *
 * @param con   DltGatewayConnection to be updated
 * @param value string to be tested
 * @return Value from DltReturnValue enum
 */
DLT_STATIC DltReturnValue dlt_gateway_check_ip(DltGatewayConnection *con, char *value)
{
    struct sockaddr_in sa;
    int ret = DLT_RETURN_ERROR;

    if ((con == NULL) || (value == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    ret = inet_pton(AF_INET, value, &(sa.sin_addr));

    /* valid IP address */
    if (ret != 0) {
        con->ip_address = strdup(value);

        if (con->ip_address == NULL) {
            dlt_log(LOG_ERR, "Cannot copy passive node IP address string\n");
            return DLT_RETURN_ERROR;
        }

        return DLT_RETURN_OK;
    }
    else {
        dlt_log(LOG_ERR, "IP address is not valid\n");
    }

    return DLT_RETURN_ERROR;
}

/**
 * Check port number
 *
 * @param con     DltGatewayConnection to be updated
 * @param value   string to be tested
 * @return Value from DltReturnValue enum
 */
DLT_STATIC DltReturnValue dlt_gateway_check_port(DltGatewayConnection *con,
                                                 char *value)
{
    long int tmp = -1;

    if ((con == NULL) || (value == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    errno = 0;
    tmp = strtol(value, NULL, 10);
    if ((errno == ERANGE && (tmp == LONG_MAX || tmp == LONG_MIN))
         || (errno != 0 && tmp == 0)) {
        dlt_vlog(LOG_ERR, "%s: cannot convert port number\n", __func__);
        return DLT_RETURN_ERROR;
    }

    /* port ranges for unprivileged applications */
    if ((tmp > IPPORT_RESERVED) && ((unsigned)tmp <= USHRT_MAX))
    {
        con->port = (int)tmp;
        return DLT_RETURN_OK;
    }
    else {
        dlt_log(LOG_ERR, "Port number is invalid\n");
    }

    return DLT_RETURN_ERROR;
}

/**
 * Check ECU name
 *
 * @param con     DltGatewayConnection to be updated
 * @param value   string to be used as ECU identifier
 * @return Value from DltReturnValue enum
 */
DLT_STATIC DltReturnValue dlt_gateway_check_ecu(DltGatewayConnection *con,
                                                char *value)
{
    if ((con == NULL) || (value == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    con->ecuid = strdup(value);

    if (con->ecuid == NULL)
        return DLT_RETURN_ERROR;

    return DLT_RETURN_OK;
}

/**
 * Check connection trigger
 *
 * @param con     DltGatewayConnection to be updated
 * @param value   string to be tested
 * @return Value from DltReturnValue enum
 */
DLT_STATIC DltReturnValue dlt_gateway_check_connect_trigger(DltGatewayConnection *con,
                                                            char *value)
{
    if ((con == NULL) || (value == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (strncasecmp(value, "OnStartup", strlen("OnStartup")) == 0) {
        con->trigger = DLT_GATEWAY_ON_STARTUP;
    }
    else if (strncasecmp(value, "OnDemand", strlen("OnDemand")) == 0)
    {
        con->trigger = DLT_GATEWAY_ON_DEMAND;
    }
    else {
        dlt_log(LOG_ERR, "Wrong connection trigger state given.\n");
        con->trigger = DLT_GATEWAY_UNDEFINED;
        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

/**
 * Check connection timeout value
 *
 * @param con     DltGatewayConnection to be updated
 * @param value   string to be tested
 * @return Value from DltReturnValue enum
 */
DLT_STATIC DltReturnValue dlt_gateway_check_timeout(DltGatewayConnection *con,
                                                    char *value)
{
    if ((con == NULL) || (value == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    con->timeout = (int)strtol(value, NULL, 10);


    if (con->timeout >= 0)
        return DLT_RETURN_OK;

    return DLT_RETURN_ERROR;
}

/**
 * Check connection interval value in General section
 *
 * @param con     DltGateway to be updated
 * @param value   string to be tested
 * @return Value from DltReturnValue enum
 */
DLT_STATIC DltReturnValue dlt_gateway_check_interval(DltGateway *gateway,
                                                    char *value)
{
    if ((gateway == NULL) || (value == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    gateway->interval = (int)strtol(value, NULL, 10);

    if (gateway->interval > 0)
        return DLT_RETURN_OK;

    return DLT_RETURN_ERROR;
}

/**
 * Check the value for SendSerialHeader
 *
 * @param con   DltGatewayConnection to be updated
 * @param value string to be tested
 * @return Value from DltReturnValue enum
 */
DLT_STATIC DltReturnValue dlt_gateway_check_send_serial(DltGatewayConnection *con,
                                                        char *value)
{
    if ((con == NULL) || (value == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    con->send_serial = !!((int)strtol(value, NULL, 10));

    return DLT_RETURN_OK;
}

/**
 * Allocate passive control messages
 *
 * @param con   DltGatewayConnection to be updated
 * @return Value from DltReturnValue enum
 */
DLT_STATIC DltReturnValue dlt_gateway_allocate_control_messages(DltGatewayConnection *con)
{
    if (con == NULL) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (con->p_control_msgs == NULL) {
        con->p_control_msgs = calloc(1, sizeof(DltPassiveControlMessage));

        if (!con->p_control_msgs) {
            dlt_log(LOG_ERR,
                    "Passive Control Message could not be allocated\n");
            return DLT_RETURN_ERROR;
        }
    }
    else {
        con->p_control_msgs->next = calloc(1, sizeof(DltPassiveControlMessage));

        if (!con->p_control_msgs->next) {
            dlt_log(LOG_ERR,
                    "Passive Control Message could not be allocated\n");
            return DLT_RETURN_ERROR;
        }

        con->p_control_msgs = con->p_control_msgs->next;
    }

    return DLT_RETURN_OK;
}

/**
 * Check the specified control messages identifier
 *
 * @param con   DltGatewayConnection to be updated
 * @param value string to be tested
 * @return Value from DltReturnValue enum
 */
DLT_STATIC DltReturnValue dlt_gateway_check_control_messages(DltGatewayConnection *con,
                                                             char *value)
{
    /* list of allowed clients given */
    char *token = NULL;
    char *rest = NULL;
    DltPassiveControlMessage *head = NULL;

    if ((con == NULL) || (value == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (strlen(value) == 0)
        return DLT_RETURN_OK;

    /* set on startup control msg id and interval*/
    token = strtok_r(value, ",", &rest);

    while (token != NULL) {
        if (dlt_gateway_allocate_control_messages(con) != DLT_RETURN_OK) {
            dlt_log(LOG_ERR,
                    "Passive Control Message could not be allocated\n");
            return DLT_RETURN_ERROR;
        }

        con->p_control_msgs->id = strtol(token, NULL, 16);
        con->p_control_msgs->user_id = DLT_SERVICE_ID_PASSIVE_NODE_CONNECT;
        con->p_control_msgs->type = CONTROL_MESSAGE_ON_STARTUP;
        con->p_control_msgs->req = CONTROL_MESSAGE_NOT_REQUESTED;
        con->p_control_msgs->interval = -1;

        if (head == NULL)
            head = con->p_control_msgs;

        if ((errno == EINVAL) || (errno == ERANGE)) {
            dlt_vlog(LOG_ERR,
                     "Control message ID is not an integer: %s\n",
                     token);
            return DLT_RETURN_ERROR;
        }
        else if ((con->p_control_msgs->id < DLT_SERVICE_ID_SET_LOG_LEVEL) ||
                 (con->p_control_msgs->id >= DLT_SERVICE_ID_LAST_ENTRY))
        {
            dlt_vlog(LOG_ERR,
                     "Control message ID is not valid: %s\n",
                     token);
            return DLT_RETURN_ERROR;
        }

        token = strtok_r(NULL, ",", &rest);
    }

    /* get back to head */
    con->p_control_msgs = head;
    con->head = head;

    return DLT_RETURN_OK;
}

/**
 * Check the specified periodic control messages identifier
 *
 * @param con   DltGatewayConnection to be updated
 * @param value string to be tested
 * @return Value from DltReturnValue enum
 */
DLT_STATIC DltReturnValue dlt_gateway_check_periodic_control_messages(
    DltGatewayConnection *con,
    char *value)
{
    char *token = NULL;
    char *rest = NULL;
    DltPassiveControlMessage *head = NULL;

    if ((con == NULL) || (value == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (strlen(value) == 0)
        return DLT_RETURN_OK;

    /* store head address */
    head = con->p_control_msgs;

    /* set periodic control msg id and interval*/
    token = strtok_r(value, ",", &rest);

    while (token != NULL) {
        char *p_token = NULL;
        char *p_rest = NULL;
        uint32_t id = 0;

        p_token = strtok_r(token, ":", &p_rest);

        if ((p_token != NULL) && (strlen(p_token) != 0)) {
            id = strtol(p_token, NULL, 16);

            /* get back to head */
            con->p_control_msgs = head;

            /* check if there is already id set in p_control_msgs */
            while (con->p_control_msgs != NULL) {
                if (con->p_control_msgs->id == id) {
                    con->p_control_msgs->type = CONTROL_MESSAGE_BOTH;
                    con->p_control_msgs->interval = strtol(p_rest, NULL, 10);

                    if (con->p_control_msgs->interval <= 0)
                        dlt_vlog(LOG_WARNING,
                                 "%s interval is %d. It won't be send periodically.\n",
                                 dlt_get_service_name(con->p_control_msgs->id),
                                 con->p_control_msgs->interval);

                    break;
                }

                con->p_control_msgs = con->p_control_msgs->next;
            }

            /* if the id is not added yet, p_control_msgs supposed to be NULL */
            if (con->p_control_msgs == NULL) {
                /* get back to head */
                con->p_control_msgs = head;

                /* go to last pointer */
                while (con->p_control_msgs != NULL) {
                    if (con->p_control_msgs->next == NULL)
                        break;

                    con->p_control_msgs = con->p_control_msgs->next;
                }

                if (dlt_gateway_allocate_control_messages(con) != DLT_RETURN_OK) {
                    dlt_log(LOG_ERR,
                            "Passive Control Message could not be allocated\n");
                    return DLT_RETURN_ERROR;
                }

                con->p_control_msgs->id = id;
                con->p_control_msgs->user_id = DLT_SERVICE_ID_PASSIVE_NODE_CONNECT;
                con->p_control_msgs->type = CONTROL_MESSAGE_PERIODIC;
                con->p_control_msgs->req = CONTROL_MESSAGE_NOT_REQUESTED;
                con->p_control_msgs->interval = strtol(p_rest, NULL, 10);

                if (con->p_control_msgs->interval <= 0)
                    dlt_vlog(LOG_WARNING,
                             "%s interval is %d. It won't be send periodically.\n",
                             dlt_get_service_name(con->p_control_msgs->id),
                             con->p_control_msgs->interval);

                if (head == NULL)
                    head = con->p_control_msgs;
            }
        }

        if ((errno == EINVAL) || (errno == ERANGE)) {
            dlt_vlog(LOG_ERR,
                     "Control message ID is not an integer: %s\n",
                     p_token);
            return DLT_RETURN_ERROR;
        }
        else if ((con->p_control_msgs->id < DLT_SERVICE_ID_SET_LOG_LEVEL) ||
                 (con->p_control_msgs->id >= DLT_SERVICE_ID_LAST_ENTRY))
        {
            dlt_vlog(LOG_ERR,
                     "Control message ID is not valid: %s\n",
                     p_token);
            return DLT_RETURN_ERROR;
        }

        token = strtok_r(NULL, ",", &rest);
    }

    /* get back to head */
    con->p_control_msgs = head;
    con->head = head;

    return DLT_RETURN_OK;
}

/**
 * Expected entries for a passive node configuration
 * Caution: after changing entries here,
 * dlt_gateway_check_param needs to be updated as well
 * */
DLT_STATIC DltGatewayConf configuration_entries[GW_CONF_COUNT] = {
    [GW_CONF_IP_ADDRESS] = {
        .key = "IPaddress",
        .func = dlt_gateway_check_ip,
        .is_opt = 0
    },
    [GW_CONF_PORT] = {
        .key = "Port",
        .func = dlt_gateway_check_port,
        .is_opt = 1
    },
    [GW_CONF_ECUID] = {
        .key = "EcuID",
        .func = dlt_gateway_check_ecu,
        .is_opt = 0
    },
    [GW_CONF_CONNECT] = {
        .key = "Connect",
        .func = dlt_gateway_check_connect_trigger,
        .is_opt = 1
    },
    [GW_CONF_TIMEOUT] = {
        .key = "Timeout",
        .func = dlt_gateway_check_timeout,
        .is_opt = 0
    },
    [GW_CONF_SEND_CONTROL] = {
        .key = "SendControl",
        .func = dlt_gateway_check_control_messages,
        .is_opt = 1
    },
    [GW_CONF_SEND_PERIODIC_CONTROL] = {
        .key = "SendPeriodicControl",
        .func = dlt_gateway_check_periodic_control_messages,
        .is_opt = 1
    },
    [GW_CONF_SEND_SERIAL_HEADER] = {
        .key = "SendSerialHeader",
        .func = dlt_gateway_check_send_serial,
        .is_opt = 1
    }
};

DLT_STATIC DltGatewayGeneralConf general_entries[GW_CONF_COUNT] = {
    [GW_CONF_GENERAL_INTERVAL] = {
        .key = "Interval",
        .func = dlt_gateway_check_interval,
        .is_opt = 1
    }
};

#define DLT_GATEWAY_NUM_PROPERTIES_MAX GW_CONF_COUNT

/**
 * Check if gateway connection general configuration parameter is valid.
 *
 * @param gateway    DltGateway
 * @param ctype      DltGatwayGeneralConnection property
 * @param value      specified property value from configuration file
 * @return Value from DltReturnValue enum
 */
DLT_STATIC DltReturnValue dlt_gateway_check_general_param(DltGateway *gateway,
                                                  DltGatewayGeneralConfType ctype,
                                                  char *value)
{
    if ((gateway == NULL) || (value == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (ctype < GW_CONF_GENEREL_COUNT)
        return general_entries[ctype].func(gateway, value);

    return DLT_RETURN_ERROR;
}

/**
 * Check if gateway connection configuration parameter is valid.
 *
 * @param gateway    DltGateway
 * @param con        DltGatewayConnection
 * @param ctype      DltGatwayConnection property
 * @param value      specified property value from configuration file
 * @return Value from DltReturnValue enum
 */
DLT_STATIC DltReturnValue dlt_gateway_check_param(DltGateway *gateway,
                                                  DltGatewayConnection *con,
                                                  DltGatewayConfType ctype,
                                                  char *value)
{
    if ((gateway == NULL) || (con == NULL) || (value == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (ctype < GW_CONF_COUNT)
        return configuration_entries[ctype].func(con, value);

    return DLT_RETURN_ERROR;
}

/**
 * Store gateway connection in internal data structure
 *
 * @param gateway    DltGatway
 * @param tmp        DltGatewayConnection
 * @param verbose    verbose flag
 * @return 0 on success, -1 otherwise
 */
int dlt_gateway_store_connection(DltGateway *gateway,
                                 DltGatewayConnection *tmp,
                                 int verbose)
{
    int i = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((gateway == NULL) || (tmp == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* find next free entry in connection array */
    while (i < gateway->num_connections) {
        if (gateway->connections[i].status == DLT_GATEWAY_UNINITIALIZED)
            break;

        i++;
    }

    if (&(gateway->connections[i]) == NULL)
        return DLT_RETURN_ERROR;

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
    gateway->connections[i].p_control_msgs = tmp->p_control_msgs;
    gateway->connections[i].head = tmp->head;
    gateway->connections[i].send_serial = tmp->send_serial;

    if (dlt_client_init_port(&gateway->connections[i].client,
                             gateway->connections[i].port,
                             verbose) != 0) {
        free(gateway->connections[i].ip_address);
        gateway->connections[i].ip_address = NULL;
        free(gateway->connections[i].ecuid);
        gateway->connections[i].ecuid = NULL;
        free(gateway->connections[i].p_control_msgs);
        gateway->connections[i].p_control_msgs = NULL;
        dlt_log(LOG_CRIT, "dlt_client_init_port() failed for gateway connection\n");
        return DLT_RETURN_ERROR;
    }

    /* setup DltClient Structure */
    if (dlt_client_set_server_ip(&gateway->connections[i].client,
                                 gateway->connections[i].ip_address) == -1) {
        dlt_log(LOG_ERR,
                "dlt_client_set_server_ip() failed for gateway connection \n");
        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

/**
 * Read configuration file and initialize connection data structures
 *
 * @param gateway       DltGateway
 * @param config_file   Gateway configuration
 * @param verbose       verbose flag
 * @return 0 on success, -1 otherwise
 */
int dlt_gateway_configure(DltGateway *gateway, char *config_file, int verbose)
{
    int ret = 0;
    int i = 0;
    DltConfigFile *file = NULL;
    int num_sections = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((gateway == NULL) || (config_file == 0) || (config_file[0] == '\0')) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* read configuration file */
    file = dlt_config_file_init(config_file);
    if(file == NULL) {
        return DLT_RETURN_ERROR;
    }

    /* get number of entries and allocate memory to store information */
    ret = dlt_config_file_get_num_sections(file, &num_sections);
    if (ret != 0) {
        dlt_config_file_release(file);
        dlt_log(LOG_ERR, "Invalid number of sections in configuration file\n");
        return DLT_RETURN_ERROR;
    }

    ret = dlt_config_file_check_section_name_exists(file, DLT_GATEWAY_GENERAL_SECTION_NAME);
    if (ret == -1) {
        /*
         * No General section in configuration file.
         * Try to use default for interval.
         */
        gateway->num_connections = num_sections;
        dlt_vlog(LOG_WARNING,
                "Missing General section in gateway. Using default interval %d (secs)\n",
                gateway->interval);
    }
    else {
        /*
         * Since the General section is also counted in num_sections,
         * so number of connections must be number of sections - 1.
         */
        gateway->num_connections = num_sections - 1;
    }

    gateway->connections = calloc(gateway->num_connections,
                                sizeof(DltGatewayConnection));

    if (gateway->connections == NULL) {
        dlt_config_file_release(file);
        dlt_log(LOG_CRIT, "Memory allocation for gateway connections failed\n");
        return DLT_RETURN_ERROR;
    }

    for (i = 0; i < num_sections; i++) {
        DltGatewayConnection tmp;
        int invalid = 0;
        DltGatewayConfType j = 0;
        DltGatewayGeneralConfType g = 0;
        char section[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = { '\0' };
        char value[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = { '\0' };

        memset(&tmp, 0, sizeof(tmp));

        /* Set default */
        tmp.send_serial = gateway->send_serial;
        tmp.port = DLT_DAEMON_TCP_PORT;

        ret = dlt_config_file_get_section_name(file, i, section);
        if (ret != 0) {
            dlt_log(LOG_WARNING, "Get section name failed\n");
            continue;
        }

        if (strncmp(section, DLT_GATEWAY_GENERAL_SECTION_NAME,
                sizeof(DLT_GATEWAY_GENERAL_SECTION_NAME)) == 0) {
            for (g = 0; g < GW_CONF_GENEREL_COUNT; g++) {
                ret = dlt_config_file_get_value(file,
                                                section,
                                                general_entries[g].key,
                                                value);

                if ((ret != 0) && general_entries[g].is_opt) {
                    /* Use default values for this key */
                    dlt_vlog(LOG_WARNING,
                             "Using default for %s.\n",
                             general_entries[g].key);
                    continue;
                }
                else if (ret != 0)
                {
                    dlt_vlog(LOG_WARNING,
                             "Missing configuration for %s.\n",
                             general_entries[g].key);
                    break;
                }

                /* check value and store general configuration */
                ret = dlt_gateway_check_general_param(gateway, g, value);

                if (ret != 0)
                    dlt_vlog(LOG_ERR,
                             "Configuration %s = %s is invalid. Using default.\n",
                             general_entries[g].key, value);
            }
        }
        else {
            for (j = 0; j < GW_CONF_COUNT; j++) {
                ret = dlt_config_file_get_value(file,
                                                section,
                                                configuration_entries[j].key,
                                                value);

                if ((ret != 0) && configuration_entries[j].is_opt) {
                    /* Use default values for this key */
                    dlt_vlog(LOG_WARNING,
                             "Using default for %s.\n",
                             configuration_entries[j].key);
                    continue;
                }
                else if (ret != 0)
                {
                    dlt_vlog(LOG_WARNING,
                             "Missing configuration for %s.\n",
                             configuration_entries[j].key);
                    invalid = 1;
                    break;
                }

                /* check value and store temporary */
                ret = dlt_gateway_check_param(gateway, &tmp, j, value);

                if (ret != 0)
                    dlt_vlog(LOG_ERR,
                             "Configuration %s = %s is invalid.\n"
                             "Using default.\n",
                             configuration_entries[j].key, value);
            }

            if (!tmp.ip_address) {
                invalid = 1;
            }

            if (invalid) {
                dlt_vlog(LOG_ERR,
                         "%s configuration is invalid.\n"
                         "Ignoring.\n",
                         section);
            }
            else {
                ret = dlt_gateway_store_connection(gateway, &tmp, verbose);

                if (ret != 0)
                    dlt_log(LOG_ERR, "Storing gateway connection data failed\n");
            }
        }

        /* strdup used inside some get_value function */
        if (tmp.ecuid != NULL) {
            free(tmp.ecuid);
            tmp.ecuid = NULL;
        }
        if (tmp.ip_address != NULL) {
            free(tmp.ip_address);
            tmp.ip_address = NULL;
        }
    }

    dlt_config_file_release(file);
    return ret;
}

int dlt_gateway_init(DltDaemonLocal *daemon_local, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon_local == NULL) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    DltGateway *gateway = &daemon_local->pGateway;

    if (gateway != NULL) {
        /* Get default value from daemon_local */
        gateway->send_serial = daemon_local->flags.lflag;
        gateway->interval = DLT_GATEWAY_TIMER_DEFAULT_INTERVAL;

        if (dlt_gateway_configure(gateway,
                                  daemon_local->flags.gatewayConfigFile,
                                  verbose) != 0) {
            dlt_log(LOG_ERR, "Gateway initialization failed\n");
            return DLT_RETURN_ERROR;
        }
    }
    else {
        dlt_log(LOG_CRIT, "Pointer to Gateway structure is NULL\n");
        return DLT_RETURN_ERROR;
    }

    /* ignore return value */
    dlt_gateway_establish_connections(gateway, daemon_local, verbose);

    return DLT_RETURN_OK;
}

void dlt_gateway_deinit(DltGateway *gateway, int verbose)
{
    DltPassiveControlMessage *msg;
    int i = 0;

    if (gateway == NULL) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return;
    }

    PRINT_FUNCTION_VERBOSE(verbose);

    for (i = 0; i < gateway->num_connections; i++) {
        DltGatewayConnection *c = &gateway->connections[i];
        dlt_client_cleanup(&c->client, verbose);
        free(c->ip_address);
        c->ip_address = NULL;
        free(c->ecuid);
        c->ecuid = NULL;

        while (c->p_control_msgs != NULL) {
            msg = c->p_control_msgs->next;
            free(c->p_control_msgs);
            c->p_control_msgs = msg;
        }
    }

    free(gateway->connections);
    gateway->connections = NULL;
}

/**
 * If connection to passive node established, add to event loop
 *
 * @param daemon_local  DltDaemonLocal
 * @param con           DltGatewayConnection
 * @param verbose       verbose flag
 * @return 0 on success, -1 otherwise
 */
DLT_STATIC int dlt_gateway_add_to_event_loop(DltDaemonLocal *daemon_local,
                                             DltGatewayConnection *con,
                                             int verbose)
{
    DltPassiveControlMessage *control_msg = NULL;
    int sendtime = 1;

    if ((daemon_local == NULL) || (con == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* connection to passive node established, add to event loop */
    con->status = DLT_GATEWAY_CONNECTED;
    con->reconnect_cnt = 0;
    con->timeout_cnt = 0;
    con->sendtime_cnt = 0;

    /* setup dlt connection and add to poll event loop here */
    if (dlt_connection_create(daemon_local,
                              &daemon_local->pEvent,
                              con->client.sock,
                              POLLIN,
                              DLT_CONNECTION_GATEWAY) != 0) {
        dlt_log(LOG_ERR, "Gateway connection creation failed\n");
        return DLT_RETURN_ERROR;
    }

    /* immediately send configured control messages */
    control_msg = con->p_control_msgs;

    while (control_msg != NULL) {
        if ((control_msg->type == CONTROL_MESSAGE_ON_STARTUP) ||
            (control_msg->type == CONTROL_MESSAGE_BOTH)) {
            if (dlt_gateway_send_control_message(con,
                                                 control_msg,
                                                 NULL,
                                                 verbose) == DLT_RETURN_OK)
                control_msg->req = CONTROL_MESSAGE_REQUESTED;
        }

        /* multiply periodic sending time */
        if (((control_msg->type == CONTROL_MESSAGE_PERIODIC) ||
             (control_msg->type == CONTROL_MESSAGE_BOTH)) &&
            (control_msg->interval > 0))
            sendtime *= control_msg->interval;

        control_msg = control_msg->next;
    }

    /* set periodic sending time */
    con->sendtime = sendtime;
    con->sendtime_cnt = con->sendtime;

    return DLT_RETURN_OK;
}

int dlt_gateway_establish_connections(DltGateway *gateway,
                                      DltDaemonLocal *daemon_local,
                                      int verbose)
{
    int i = 0;
    int ret = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((gateway == NULL) || (daemon_local == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    for (i = 0; i < gateway->num_connections; i++) {
        DltGatewayConnection *con = &(gateway->connections[i]);
        DltPassiveControlMessage *control_msg = NULL;

        if (con == NULL) {
            dlt_log(LOG_CRIT, "Cannot retrieve gateway connection details\n");
            return DLT_RETURN_ERROR;
        }

        if ((con->status != DLT_GATEWAY_CONNECTED) &&
            (con->trigger != DLT_GATEWAY_ON_DEMAND) &&
            (con->trigger != DLT_GATEWAY_DISABLED)) {
            ret = dlt_client_connect(&con->client, verbose);

            if (ret == 0) {
                /* setup dlt connection and add to poll event loop here */
                if (dlt_gateway_add_to_event_loop(daemon_local, con, verbose) != DLT_RETURN_OK) {
                    dlt_log(LOG_ERR, "Gateway connection creation failed\n");
                    return DLT_RETURN_ERROR;
                }
            }
            else {
                dlt_log(LOG_DEBUG,
                        "Passive Node is not up. Connection failed.\n");

                con->timeout_cnt++;

                if (con->timeout > 0) {
                    if (con->timeout_cnt > con->timeout) {
                        con->trigger = DLT_GATEWAY_DISABLED;
                        dlt_log(LOG_WARNING,
                                "Passive Node connection retry timed out. "
                                "Give up.\n");
                    }
                }
                else if (con->timeout == 0) {
                    dlt_vlog(LOG_DEBUG, "Retried [%d] times\n", con->timeout_cnt);
                }
            }
        }
        else if ((con->status == DLT_GATEWAY_CONNECTED) &&
                 (con->trigger != DLT_GATEWAY_DISABLED))
        {
            /* setup dlt connection and add to poll event loop here */
            if (dlt_connection_create(daemon_local,
                                      &daemon_local->pEvent,
                                      con->client.sock,
                                      POLLIN,
                                      DLT_CONNECTION_GATEWAY) != 0) {
                dlt_log(LOG_ERR, "Gateway connection creation failed\n");
                return DLT_RETURN_ERROR;
            }

            /* immediately send periodic configured control messages */
            control_msg = con->p_control_msgs;

            while (control_msg != NULL) {
                if ((control_msg->type == CONTROL_MESSAGE_PERIODIC) ||
                    (control_msg->type == CONTROL_MESSAGE_BOTH)) {
                    if (dlt_gateway_send_control_message(con,
                                                         control_msg,
                                                         NULL,
                                                         verbose) == DLT_RETURN_OK)
                        control_msg->req = CONTROL_MESSAGE_REQUESTED;
                }

                control_msg = control_msg->next;
            }

            /* check sendtime counter */
            if (con->sendtime_cnt > 0)
                con->sendtime_cnt--;

            if (con->sendtime_cnt == 0)
                con->sendtime_cnt = con->sendtime;
        }
    }

    return DLT_RETURN_OK;
}

DltReceiver *dlt_gateway_get_connection_receiver(DltGateway *gateway, int fd)
{
    int i = 0;

    if (gateway == NULL) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return NULL;
    }

    for (i = 0; i < gateway->num_connections; i++) {
        DltGatewayConnection *c = &gateway->connections[i];

        if ((c->status == DLT_GATEWAY_CONNECTED) && (c->client.sock == fd))
            return &c->client.receiver;
    }

    return NULL;
}

/**
 * Parse GET_LOG_INFO
 *
 * @param daemon          DltDaemon
 * @param ecu             Ecu ID
 * @param msg             DltMessage
 * @param req             1 if requested from gateway, 0 otherwise
 * @param verbose verbose flag
 * @return Value from DltReturnValue enum
 */
DLT_STATIC DltReturnValue dlt_gateway_parse_get_log_info(DltDaemon *daemon,
                                                         char *ecu,
                                                         DltMessage *msg,
                                                         int req,
                                                         int verbose)
{
    char resp_text[DLT_RECEIVE_BUFSIZE] = { '\0' };
    DltServiceGetLogInfoResponse *resp = NULL;
    AppIDsType app;
    ContextIDsInfoType con;
    int i = 0;
    int j = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((msg == NULL) || (msg->databuffer == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (dlt_check_rcv_data_size(msg->datasize, sizeof(DltServiceGetLogInfoResponse)) < 0)
        return DLT_RETURN_ERROR;

    /* if the request was send from gateway, clear all application and context list */
    if (req == CONTROL_MESSAGE_REQUESTED) {
        /* clear application list */
        if (dlt_daemon_applications_clear(daemon, ecu, verbose) == DLT_RETURN_ERROR) {
            dlt_log(LOG_ERR, "Cannot clear applications list\n");
            return DLT_RETURN_ERROR;
        }

        /* clear context list */
        if (dlt_daemon_contexts_clear(daemon, ecu, verbose) == DLT_RETURN_ERROR) {
            dlt_log(LOG_ERR, "Cannot clear contexts list\n");
            return DLT_RETURN_ERROR;
        }
    }

    /* check response */
    if (dlt_message_payload(msg,
                            resp_text,
                            DLT_RECEIVE_BUFSIZE,
                            DLT_OUTPUT_ASCII, 0) != DLT_RETURN_OK) {
        dlt_log(LOG_ERR, "GET_LOG_INFO payload failed\n");
        return DLT_RETURN_ERROR;
    }

    /* prepare pointer to message request */
    resp = (DltServiceGetLogInfoResponse *)calloc(1, sizeof(DltServiceGetLogInfoResponse));

    if (resp == NULL) {
        dlt_log(LOG_ERR,
                "Get Log Info Response could not be allocated\n");
        return DLT_RETURN_ERROR;
    }

    if (dlt_set_loginfo_parse_service_id(resp_text, &resp->service_id, &resp->status) != DLT_RETURN_OK) {
        dlt_log(LOG_ERR, "Parsing GET_LOG_INFO failed\n");
        dlt_client_cleanup_get_log_info(resp);
        return DLT_RETURN_ERROR;
    }

    if (dlt_client_parse_get_log_info_resp_text(resp, resp_text) != DLT_RETURN_OK) {
        dlt_log(LOG_ERR, "Parsing GET_LOG_INFO failed\n");
        dlt_client_cleanup_get_log_info(resp);
        return DLT_RETURN_ERROR;
    }

    for (i = 0; i < resp->log_info_type.count_app_ids; i++) {
        app = resp->log_info_type.app_ids[i];

        /* add application */
        if (dlt_daemon_application_add(daemon,
                                       app.app_id,
                                       0,
                                       app.app_description,
                                       -1,
                                       ecu,
                                       verbose) == 0) {
            dlt_vlog(LOG_WARNING,
                     "%s: dlt_daemon_application_add failed\n",
                     __func__);
            dlt_client_cleanup_get_log_info(resp);
            return DLT_RETURN_ERROR;
        }

        for (j = 0; j < app.count_context_ids; j++) {
            con = app.context_id_info[j];

            /* add context */
            if (dlt_daemon_context_add(daemon,
                                       app.app_id,
                                       con.context_id,
                                       con.log_level,
                                       con.trace_status,
                                       0,
                                       -1,
                                       con.context_description,
                                       ecu,
                                       verbose) == 0) {
                dlt_vlog(LOG_WARNING,
                         "%s: dlt_daemon_context_add failed for %4s\n",
                         __func__,
                         app.app_id);
                dlt_client_cleanup_get_log_info(resp);
                return DLT_RETURN_ERROR;
            }
        }
    }

    /* free response */
    dlt_client_cleanup_get_log_info(resp);

    return DLT_RETURN_OK;
}

/**
 * Parse GET_DEFAULT_LOG_LEVEL
 *
 * @param daemon          DltDaemon
 * @param daemon_local    DltDaemonLocal
 * @param ecu             Ecu ID
 * @param msg             DltMessage
 * @param verbose verbose flag
 * @return 0 on success, -1 otherwise
 */
DLT_STATIC int dlt_gateway_parse_get_default_log_level(DltDaemon *daemon,
                                                       DltDaemonLocal *daemon_local,
                                                       char *ecu,
                                                       DltMessage *msg,
                                                       int verbose)
{
    DltServiceGetDefaultLogLevelResponse *resp = NULL;
    DltGatewayConnection *con = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (daemon_local == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (dlt_check_rcv_data_size(msg->datasize,
                                sizeof(DltServiceGetDefaultLogLevelResponse)) < 0) {
        dlt_log(LOG_ERR, "Received data incomplete.\n");
        return DLT_RETURN_ERROR;
    }

    /* prepare pointer to message request */
    resp = (DltServiceGetDefaultLogLevelResponse *)(msg->databuffer);

    con = dlt_gateway_get_connection(&daemon_local->pGateway,
                                     ecu,
                                     verbose);

    if (con == NULL) {
        dlt_vlog(LOG_ERR, "No information about passive ECU: %s\n",
                 ecu);

        return DLT_RETURN_ERROR;
    }

    con->default_log_level = resp->log_level;

    return DLT_RETURN_OK;
}

/**
 * Service offline logstorage
 *
 * @param daemon       DltDaemon
 * @param daemon_local DltDaemonLocal
 * @param verbose      int
 * @return 0 on success, -1 otherwise
 */
DLT_STATIC int dlt_gateway_control_service_logstorage(DltDaemon *daemon,
                                                      DltDaemonLocal *daemon_local,
                                                      int verbose)
{
    unsigned int connection_type = 0;
    int i = 0;

    if (daemon_local->flags.offlineLogstorageMaxDevices <= 0) {
        dlt_log(LOG_INFO,
                "Logstorage functionality not enabled or MAX device set is 0\n");
        return DLT_RETURN_ERROR;
    }

    for (i = 0; i < daemon_local->flags.offlineLogstorageMaxDevices; i++) {
        connection_type = daemon->storage_handle[i].connection_type;

        if (connection_type == DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED)
            /* Check if log level of running application needs an update */
            dlt_daemon_logstorage_update_application_loglevel(daemon,
                                                              daemon_local,
                                                              i,
                                                              verbose);
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_gateway_process_passive_node_messages(DltDaemon *daemon,
                                                         DltDaemonLocal *daemon_local,
                                                         DltReceiver *receiver,
                                                         int verbose)
{
    int i = 0;
    DltGateway *gateway = NULL;
    DltGatewayConnection *con = NULL;
    DltMessage msg = { 0 };
    bool b_reset_receiver = false;

    if ((daemon == NULL) || (daemon_local == NULL) || (receiver == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    PRINT_FUNCTION_VERBOSE(verbose);

    gateway = &daemon_local->pGateway;

    if (gateway == NULL) {
        dlt_log(LOG_ERR, "Gateway structure is NULL\n");
        return DLT_RETURN_ERROR;
    }

    for (i = 0; i < gateway->num_connections; i++)
        if ((gateway->connections[i].status == DLT_GATEWAY_CONNECTED) && (gateway->connections[i].client.sock == receiver->fd)) {
            con = &gateway->connections[i];
            break;
        }

    if (con == NULL) {
        dlt_log(LOG_ERR, "Cannot associate fd to passive Node connection\n");
        return DLT_RETURN_ERROR;
    }

    /* now the corresponding passive node connection is available */
    if (dlt_message_init(&msg, verbose) == -1) {
        dlt_log(LOG_ERR,
                "Cannot initialize DLT message for passive node forwarding\n");
        return DLT_RETURN_ERROR;
    }

    /* nearly copy and paste of dlt_client_main_loop function */
    if (dlt_receiver_receive(receiver) <= 0) {
        /* No more data to be received */
        if (dlt_message_free(&msg, verbose) < 0) {
            dlt_log(LOG_ERR, "Cannot free DLT message\n");
            return DLT_RETURN_ERROR;
        }

        dlt_log(LOG_WARNING, "Connection to passive node lost\n");

        if (con->reconnect_cnt < DLT_GATEWAY_RECONNECT_MAX) {
            dlt_log(LOG_WARNING, "Try to reconnect.\n");
            con->reconnect_cnt += 1;
            con->timeout_cnt = 0;
        }
        else {
            con->status = DLT_GATEWAY_DISCONNECTED;

            if (dlt_event_handler_unregister_connection(&daemon_local->pEvent,
                                                        daemon_local,
                                                        receiver->fd) != 0)
                dlt_log(LOG_ERR, "Remove passive node Connection failed\n");
        }

        return DLT_RETURN_OK;
    }

    while (dlt_message_read(&msg,
                            (unsigned char *)receiver->buf,
                            receiver->bytesRcvd,
                            0,
                            verbose) == DLT_MESSAGE_ERROR_OK) {
        DltStandardHeaderExtra *header = (DltStandardHeaderExtra *)
            (msg.headerbuffer +
             sizeof(DltStorageHeader) +
             sizeof(DltStandardHeader));

        /* only forward messages if the received ECUid is the expected one */
        if (strncmp(header->ecu, con->ecuid, DLT_ID_SIZE) == 0) {
            uint32_t id;
            uint32_t id_tmp;
            DltPassiveControlMessage *control_msg = con->p_control_msgs;

            dlt_vlog(LOG_DEBUG,
                     "Received ECUid (%.*s) similar to configured ECUid(%s). "
                     "Forwarding message (%s).\n",
                     DLT_ID_SIZE,
                     header->ecu,
                     con->ecuid,
                     msg.databuffer);

            id_tmp = *((uint32_t *)(msg.databuffer));
            id = DLT_ENDIAN_GET_32(msg.standardheader->htyp, id_tmp);

            /* if ID is GET_LOG_INFO, parse msg */
            if (id == DLT_SERVICE_ID_GET_LOG_INFO) {
                while (control_msg) {
                    if (control_msg->id == id) {
                        if (dlt_gateway_parse_get_log_info(daemon,
                                                           header->ecu,
                                                           &msg,
                                                           control_msg->req,
                                                           verbose) == DLT_RETURN_ERROR)
                            dlt_log(LOG_WARNING, "Parsing GET_LOG_INFO message failed!\n");

                        /* Check for logstorage */
                        dlt_gateway_control_service_logstorage(daemon,
                                                               daemon_local,
                                                               verbose);

                        /* initialize the flag */
                        control_msg->req = CONTROL_MESSAGE_NOT_REQUESTED;
                        break;
                    }

                    control_msg = control_msg->next;
                }
            }
            else if (id == DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL)
            {
                if (dlt_gateway_parse_get_default_log_level(
                        daemon,
                        daemon_local,
                        header->ecu,
                        &msg,
                        verbose) == DLT_RETURN_ERROR)
                    dlt_log(LOG_WARNING,
                            "Parsing GET_DEFAULT_LOG_LEVEL message failed!\n");
            }

            /* prepare storage header */
            if (dlt_set_storageheader(msg.storageheader,
                                      msg.headerextra.ecu) == DLT_RETURN_ERROR) {
                dlt_vlog(LOG_ERR, "%s: Can't set storage header\n", __func__);
                return DLT_RETURN_ERROR;
            }

            dlt_daemon_client_send(DLT_DAEMON_SEND_TO_ALL,
                                       daemon,
                                       daemon_local,
                                       msg.headerbuffer,
                                       sizeof(DltStorageHeader),
                                       msg.headerbuffer + sizeof(DltStorageHeader),
                                       msg.headersize - sizeof(DltStorageHeader),
                                       msg.databuffer,
                                       msg.datasize,
                                       verbose);
        } else { /* otherwise remove this connection and do not connect again */
            dlt_vlog(LOG_WARNING,
                     "Received ECUid (%.*s) differs to configured ECUid(%s). "
                     "Discard this message.\n",
                     DLT_ID_SIZE,
                     header->ecu,
                     con->ecuid);

            /* disconnect from passive node */
            con->status = DLT_GATEWAY_DISCONNECTED;
            con->trigger = DLT_GATEWAY_DISABLED;

            if (dlt_event_handler_unregister_connection(&daemon_local->pEvent,
                                                        daemon_local,
                                                        receiver->fd)
                != 0)
                dlt_log(LOG_ERR, "Remove passive node Connection failed\n");

            dlt_log(LOG_WARNING,
                    "Disconnect from passive node due to invalid ECUid\n");

            /* it is possible that a partial log was received through the last recv call */
            /* however, the rest will never be received since the socket will be closed by above method */
            /* as such, we need to reset the receiver to prevent permanent corruption */
            b_reset_receiver = true;
        }

        if (msg.found_serialheader) {
            if (dlt_receiver_remove(receiver,
                                    msg.headersize +
                                    msg.datasize -
                                    sizeof(DltStorageHeader) +
                                    sizeof(dltSerialHeader)) == -1) {
                /* Return value ignored */
                dlt_message_free(&msg, verbose);
                return DLT_RETURN_ERROR;
            }
        }
        else if (dlt_receiver_remove(receiver,
                                     msg.headersize +
                                     msg.datasize -
                                     sizeof(DltStorageHeader)) == -1) {
            /* Return value ignored */
            dlt_message_free(&msg, verbose);
            return DLT_RETURN_ERROR;
        }
    }

    if (b_reset_receiver)
        dlt_receiver_remove(receiver, receiver->bytesRcvd);

    if (dlt_receiver_move_to_begin(receiver) == -1) {
        /* Return value ignored */
        dlt_message_free(&msg, verbose);
        return DLT_RETURN_ERROR;
    }

    if (dlt_message_free(&msg, verbose) == -1)
        return DLT_RETURN_ERROR;

    return DLT_RETURN_OK;
}

int dlt_gateway_process_gateway_timer(DltDaemon *daemon,
                                      DltDaemonLocal *daemon_local,
                                      DltReceiver *receiver,
                                      int verbose)
{
    uint64_t expir = 0;
    ssize_t res = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon_local == NULL) || (daemon == NULL) || (receiver == NULL)) {
        dlt_vlog(LOG_ERR,
                 "%s: invalid parameters\n",
                 __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    res = read(receiver->fd, &expir, sizeof(expir));

    if (res < 0)
        dlt_vlog(LOG_WARNING,
                 "%s: Fail to read timer (%s)\n",
                 __func__,
                 strerror(errno));
        /* Activity received on timer_wd, but unable to read the fd:
         * let's go on sending notification */

    /* try to connect to passive nodes */
    dlt_gateway_establish_connections(&daemon_local->pGateway,
                                      daemon_local,
                                      verbose);

    dlt_log(LOG_DEBUG, "Gateway Timer\n");

    return DLT_RETURN_OK;
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
    uint32_t id_tmp;
    uint32_t id;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((gateway == NULL) || (daemon_local == NULL) || (msg == NULL) || (ecu == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    for (i = 0; i < gateway->num_connections; i++)
        if (strncmp(gateway->connections[i].ecuid,
                    ecu,
                    DLT_ID_SIZE) == 0) {
            con = &gateway->connections[i];
            break;
        }



    if (con == NULL) {
        dlt_log(LOG_WARNING, "Unknown passive node identifier\n");
        return DLT_RETURN_ERROR;
    }

    if (con->status != DLT_GATEWAY_CONNECTED) {
        dlt_log(LOG_INFO, "Passive node is not connected\n");
        return DLT_RETURN_ERROR;
    }

    if (con->send_serial) { /* send serial header */
        ret = send(con->client.sock,
                   (void *)dltSerialHeader,
                   sizeof(dltSerialHeader),
                   0);

        if (ret == -1) {
            dlt_log(LOG_ERR, "Sending message to passive DLT Daemon failed\n");
            return DLT_RETURN_ERROR;
        }
    }

    ret = send(con->client.sock,
               msg->headerbuffer + sizeof(DltStorageHeader),
               msg->headersize - sizeof(DltStorageHeader),
               0);

    if (ret == -1) {
        dlt_log(LOG_ERR, "Sending message to passive DLT Daemon failed\n");
        return DLT_RETURN_ERROR;
    }
    else {
        ret = send(con->client.sock, msg->databuffer, msg->datasize, 0);

        if (ret == -1) {
            dlt_log(LOG_ERR, "Sending message to passive DLT Daemon failed\n");
            return DLT_RETURN_ERROR;
        }
    }

    id_tmp = *((uint32_t *)(msg->databuffer));
    id = DLT_ENDIAN_GET_32(msg->standardheader->htyp, id_tmp);

    dlt_vlog(LOG_INFO,
             "Control message forwarded : %s\n",
             dlt_get_service_name(id));
    return DLT_RETURN_OK;
}

int dlt_gateway_process_on_demand_request(DltGateway *gateway,
                                          DltDaemonLocal *daemon_local,
                                          char node_id[DLT_ID_SIZE],
                                          int connection_status,
                                          int verbose)
{
    int i = 0;
    DltGatewayConnection *con = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((gateway == NULL) || (daemon_local == NULL) || (node_id == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* find connection by ECU id */
    for (i = 0; i < gateway->num_connections; i++) {
        if (strncmp(node_id, gateway->connections[i].ecuid, DLT_ID_SIZE) == 0) {
            con = &gateway->connections[i];
            break;
        }
    }

    if (con == NULL) {
        dlt_log(LOG_WARNING, "Specified ECUid not found\n");
        return DLT_RETURN_ERROR;
    }

    if (connection_status == 1) { /* try to connect */

        if (con->status != DLT_GATEWAY_CONNECTED) {
            if (dlt_client_connect(&con->client, verbose) == 0) {
                /* setup dlt connection and add to poll event loop here */
                if (dlt_gateway_add_to_event_loop(daemon_local, con, verbose) != DLT_RETURN_OK) {
                    dlt_log(LOG_ERR, "Gateway connection creation failed\n");
                    return DLT_RETURN_ERROR;
                }
            }
            else {
                dlt_log(LOG_ERR, "Could not connect to passive node\n");
                return DLT_RETURN_ERROR;
            }
        }
        else {
            dlt_log(LOG_INFO, "Passive node already connected\n");
        }
    }
    else if (connection_status == 0) /* disconnect*/
    {

        con->status = DLT_GATEWAY_DISCONNECTED;
        con->trigger = DLT_GATEWAY_ON_DEMAND;

        if (dlt_event_handler_unregister_connection(&daemon_local->pEvent,
                                                    daemon_local,
                                                    con->client.sock) != 0)
            dlt_log(LOG_ERR,
                    "Remove passive node event handler connection failed\n");
    }
    else {
        dlt_log(LOG_ERR, "Unknown command (connection_status)\n");
        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

int dlt_gateway_send_control_message(DltGatewayConnection *con,
                                     DltPassiveControlMessage *control_msg,
                                     void *data,
                                     int verbose)
{
    int ret = DLT_RETURN_OK;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (con == NULL) {
        dlt_vlog(LOG_WARNING,
                 "%s: Invalid parameter given\n",
                 __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* no (more) control message to be send */
    if (control_msg->id == 0)
        return DLT_RETURN_ERROR;

    /* check sendtime counter and message interval */
    /* sendtime counter is 0 on startup, otherwise positive value */
    if ((control_msg->type != CONTROL_MESSAGE_ON_DEMAND) && (con->sendtime_cnt > 0)) {
        if (control_msg->interval <= 0)
            return DLT_RETURN_ERROR;

        if ((control_msg->type == CONTROL_MESSAGE_PERIODIC) ||
            (control_msg->type == CONTROL_MESSAGE_BOTH)) {
            if ((con->sendtime_cnt - 1) % control_msg->interval != 0)
                return DLT_RETURN_ERROR;
        }
    }

    if (con->send_serial) { /* send serial header */
        ret = send(con->client.sock,
                   (void *)dltSerialHeader,
                   sizeof(dltSerialHeader),
                   0);

        if (ret == -1) {
            dlt_log(LOG_ERR, "Sending message to passive DLT Daemon failed\n");
            return DLT_RETURN_ERROR;
        }
    }

    switch (control_msg->id) {
    case DLT_SERVICE_ID_GET_LOG_INFO:
        return dlt_client_get_log_info(&con->client);
        break;
    case DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL:
        return dlt_client_get_default_log_level(&con->client);
        break;
    case DLT_SERVICE_ID_GET_SOFTWARE_VERSION:
        return dlt_client_get_software_version(&con->client);
        break;
    case DLT_SERVICE_ID_SET_LOG_LEVEL:

        if (data == NULL) {
            dlt_vlog(LOG_WARNING,
                     "Insufficient data for %s received. Send control request failed.\n",
                     dlt_get_service_name(control_msg->id));
            return DLT_RETURN_ERROR;
        }

        DltServiceSetLogLevel *req = (DltServiceSetLogLevel *)data;
        return dlt_client_send_log_level(&con->client,
                                         req->apid,
                                         req->ctid,
                                         req->log_level);
        break;
    default:
        dlt_vlog(LOG_WARNING,
                 "Cannot forward request: %s.\n",
                 dlt_get_service_name(control_msg->id));
    }

    return DLT_RETURN_OK;
}

DltGatewayConnection *dlt_gateway_get_connection(DltGateway *gateway,
                                                 char *ecu,
                                                 int verbose)
{
    DltGatewayConnection *con = NULL;
    int i = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((gateway == NULL) || (ecu == NULL)) {
        dlt_vlog(LOG_ERR, "%s: wrong parameter\n", __func__);
        return con;
    }

    for (i = 0; i < gateway->num_connections; i++) {
        con = &gateway->connections[i];

        if (strncmp(con->ecuid, ecu, DLT_ID_SIZE) == 0)
            return con;
    }

    dlt_vlog(LOG_ERR, "%s: No connection found\n", ecu);

    return con;
}
