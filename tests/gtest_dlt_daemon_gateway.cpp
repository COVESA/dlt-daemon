/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2016 Advanced Driver Information Technology.
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
 * \author Onkar Palkar onkar.palkar@wipro.com
 *
 * \copyright Copyright © 2015 Advanced Driver Information Technology.
 *
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file gtest_dlt_daemon_gateway.cpp
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: gtest_dlt_daemon_gateway.cpp                                  **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Onkar Palkar onkar.palkar@wipro.com                           **
**  PURPOSE   : Unit test for dlt_gateway.c                                   **
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
**  op          Onkar Palkar               Wipro                              **
*******************************************************************************/

#include <gtest/gtest.h>
#include <limits.h>
#include <syslog.h>

extern "C"
{
#include "dlt_gateway.h"
#include "dlt_gateway_internal.h"
}

/* Begin Method: dlt_gateway::t_dlt_gateway_init*/
TEST(t_dlt_gateway_init, normal)
{
    DltDaemonLocal daemon_local;
    DltGatewayConnection connections;
    daemon_local.pGateway.connections = &connections;
    daemon_local.pGateway.num_connections = 1;
    daemon_local.flags.lflag = 0;
    DltConnection connections1;
    DltReceiver receiver;
    daemon_local.pEvent.connections = &connections1;
    daemon_local.pEvent.pfd = 0;
    daemon_local.pEvent.nfds = 0;
    daemon_local.pEvent.max_nfds = 0;
    daemon_local.pEvent.connections->receiver = &receiver;
    daemon_local.pEvent.connections->next = NULL;
    memset(daemon_local.flags.gatewayConfigFile, 0, DLT_DAEMON_FLAG_MAX);
    strncpy(daemon_local.flags.gatewayConfigFile, "/tmp/dlt_gateway.conf", DLT_DAEMON_FLAG_MAX - 1);

    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_init(&daemon_local, 1));

    dlt_gateway_deinit(&daemon_local.pGateway, 0);
}

TEST(t_dlt_gateway_init, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_init(NULL, 0));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_send_control_message*/
TEST(t_dlt_gateway_send_control_message, Normal)
{
    DltDaemonLocal daemon_local;
    DltGatewayConnection connections;
    DltConnection connections1;
    DltReceiver receiver1;
    DltPassiveControlMessage p_control_msgs;
    DltServiceSetLogLevel req;
    memset(&daemon_local,0, sizeof(DltDaemonLocal));
    memset(&connections, 0, sizeof(DltGatewayConnection));
    memset(&p_control_msgs,0, sizeof(DltPassiveControlMessage));
    strcpy(req.apid,"LOG");
    strcpy(req.ctid,"TES");
    req.log_level = 1;

    daemon_local.pGateway.connections = &connections;
    daemon_local.pEvent.connections = &connections1;
    daemon_local.pGateway.connections->p_control_msgs = &p_control_msgs;
    daemon_local.pEvent.connections->next = NULL;
    daemon_local.pEvent.pfd = 0;
    daemon_local.pEvent.nfds = 0;
    daemon_local.pEvent.max_nfds = 0;
    daemon_local.pEvent.connections->receiver = &receiver1;
    memset(daemon_local.flags.gatewayConfigFile, 0, DLT_DAEMON_FLAG_MAX);
    strncpy(daemon_local.flags.gatewayConfigFile, "/tmp/dlt_gateway.conf", DLT_DAEMON_FLAG_MAX - 1);
    (void) dlt_gateway_init(&daemon_local, 0);

    daemon_local.pGateway.connections->p_control_msgs->id = DLT_SERVICE_ID_GET_LOG_INFO;
    daemon_local.pGateway.connections->p_control_msgs->type = CONTROL_MESSAGE_ON_DEMAND;
    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_send_control_message(daemon_local.pGateway.connections,
                                                              daemon_local.pGateway.connections->p_control_msgs,
                                                              NULL, 0));

    daemon_local.pGateway.connections->p_control_msgs->id = DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL;
    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_send_control_message(daemon_local.pGateway.connections,
                                                              daemon_local.pGateway.connections->p_control_msgs,
                                                              NULL, 0));

    daemon_local.pGateway.connections->p_control_msgs->id = DLT_SERVICE_ID_GET_SOFTWARE_VERSION;
    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_send_control_message(daemon_local.pGateway.connections,
                                                              daemon_local.pGateway.connections->p_control_msgs,
                                                              NULL, 0));

    daemon_local.pGateway.connections->p_control_msgs->id = DLT_SERVICE_ID_SET_LOG_LEVEL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_send_control_message(daemon_local.pGateway.connections,
                                                              daemon_local.pGateway.connections->p_control_msgs,
                                                              (void*)&req, 0));
}

TEST(t_dlt_gateway_send_control_message, nullpointer)
{
    DltDaemonLocal daemon_local;
    DltGatewayConnection connections;
    DltConnection connections1;
    DltReceiver receiver1;
    DltPassiveControlMessage p_control_msgs;
    memset(&daemon_local,0, sizeof(DltDaemonLocal));
    memset(&connections, 0, sizeof(DltGatewayConnection));
    memset(&p_control_msgs,0, sizeof(DltPassiveControlMessage));
    daemon_local.pGateway.connections = &connections;
    daemon_local.pEvent.connections = &connections1;
    daemon_local.pGateway.connections->p_control_msgs = &p_control_msgs;
    daemon_local.pEvent.connections->next = NULL;
    daemon_local.pEvent.connections->receiver = &receiver1;
    memset(daemon_local.flags.gatewayConfigFile,0,DLT_DAEMON_FLAG_MAX);
    strncpy(daemon_local.flags.gatewayConfigFile, "/tmp/dlt_gateway.conf", DLT_DAEMON_FLAG_MAX - 1);

    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_send_control_message(NULL, NULL, NULL, 0));

    daemon_local.pGateway.connections->p_control_msgs->id = DLT_SERVICE_ID_SET_LOG_LEVEL;
    EXPECT_EQ(DLT_RETURN_ERROR,dlt_gateway_send_control_message(daemon_local.pGateway.connections,
                                                                daemon_local.pGateway.connections->p_control_msgs,
                                                                NULL, 0));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_store_connection*/
TEST(t_dlt_gateway_store_connection, normal)
{
    char ip_address[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = "10.113.100.100";
    char ecuid[] = "1234";
    DltGateway gateway;
    DltGatewayConnection tmp;
    DltGatewayConnection tmp1;
    gateway.num_connections = 1;
    gateway.connections = &tmp1;
    gateway.connections->status = DLT_GATEWAY_UNINITIALIZED;
    tmp.ip_address = ip_address;
    tmp.ecuid = ecuid;
    tmp.sock_domain = 1;
    tmp.sock_type = 2;
    tmp.sock_protocol = 1;
    tmp.port = 1;
    tmp.trigger = DLT_GATEWAY_ON_STARTUP;
    tmp.timeout = 500;

    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_store_connection(&gateway, &tmp, 0));
    EXPECT_EQ(gateway.connections->sock_domain, tmp.sock_domain);
    EXPECT_EQ(gateway.connections->sock_type, tmp.sock_type);
    EXPECT_EQ(gateway.connections->port, tmp.port);
}

TEST(t_dlt_gateway_store_connection, nullpointer)
{
    DltGateway gateway;

    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_store_connection(NULL, NULL, 0));
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_store_connection(&gateway, NULL, 0));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_check_ip*/
TEST(t_dlt_gateway_check_ip, normal)
{
    DltGatewayConnection tmp;
    DltGatewayConnection *con;
    char value[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = "10.113.100.100";
    con = &tmp;

    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_check_ip(con, value));
}

TEST(t_dlt_gateway_check_ip, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_check_ip(NULL, NULL));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_check_send_serial*/
TEST(t_dlt_gateway_check_send_serial, normal)
{
    DltGatewayConnection tmp;
    DltGatewayConnection *con;
    char value[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = "134dltgatway";
    con = &tmp;

    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_check_send_serial(con, value));
}

TEST(t_dlt_gateway_check_send_serial, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_check_send_serial(NULL, NULL));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_allocate_control_messages*/
TEST(t_dlt_gateway_allocate_control_messages, normal)
{
    DltGatewayConnection tmp;
    DltGatewayConnection *con;
    tmp.p_control_msgs = NULL;
    con = &tmp;
    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_allocate_control_messages(con));
}

TEST(t_dlt_gateway_allocate_control_messages, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_allocate_control_messages(NULL));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_check_control_messages*/
TEST(t_dlt_gateway_check_control_messages, normal)
{
    DltGatewayConnection tmp;
    DltGatewayConnection *con;
    char value[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = "1,2,3,4,5";
    tmp.p_control_msgs = NULL;
    con = &tmp;
    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_check_control_messages(con, value));
}

TEST(t_dlt_gateway_check_control_messages, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_check_control_messages(NULL, NULL));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_check_periodic_control_messages*/
TEST(t_dlt_gateway_check_periodic_control_messages, normal)
{
    DltGatewayConnection tmp;
    DltGatewayConnection *con;
    char value[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = "1:5,2:10";
    tmp.p_control_msgs = NULL;
    con = &tmp;
    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_check_periodic_control_messages(con, value));
}

TEST(t_dlt_gateway_check_periodic_control_messages, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_check_periodic_control_messages(NULL, NULL));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_check_port*/
TEST(t_dlt_gateway_check_port, normal)
{
    DltGatewayConnection tmp;
    DltGatewayConnection *con;
    char value[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = "3490";
    con = &tmp;

    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_check_port(con, value));
}

TEST(t_dlt_gateway_check_port, abnormal)
{
    DltGatewayConnection tmp;
    DltGatewayConnection *con;
    char value[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = "9999999999";
    con = &tmp;

    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_check_port(con, value));
}

TEST(t_dlt_gateway_check_port, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_check_port(NULL, NULL));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_check_ecu*/
TEST(t_dlt_gateway_check_ecu, normal)
{
    DltGatewayConnection tmp;
    DltGatewayConnection *con;
    char value[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = "ECU2";
    con = &tmp;

    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_check_ecu(con, value));
}

TEST(t_dlt_gateway_check_ecu, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_check_ecu(NULL, NULL));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_check_connect_trigger*/
TEST(t_dlt_gateway_check_connect_trigger, normal)
{
    DltGatewayConnection tmp;
    DltGatewayConnection *con;
    char value[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = "OnStartup";
    con = &tmp;

    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_check_connect_trigger(con, value));
}

TEST(t_dlt_gateway_check_connect_trigger, abnormal)
{
    DltGatewayConnection tmp;
    DltGatewayConnection *con;
    char value[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = "wrong_parameter";
    con = &tmp;

    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_check_connect_trigger(con, value));
}

TEST(t_dlt_gateway_check_connect_trigger, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_check_connect_trigger(NULL, NULL));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_check_timeout*/
TEST(t_dlt_gateway_check_timeout, normal)
{
    DltGatewayConnection tmp;
    DltGatewayConnection *con;
    char value[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = "10";
    con = &tmp;

    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_check_timeout(con, value));
}

TEST(t_dlt_gateway_check_timeout, abnormal)
{
    DltGatewayConnection tmp;
    DltGatewayConnection *con;
    char value[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = "-1";
    con = &tmp;

    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_check_timeout(con, value));
}

TEST(t_dlt_gateway_check_timeout, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_check_timeout(NULL, NULL));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_establish_connections*/
TEST(t_dlt_gateway_establish_connections, normal)
{
    char ip[] = "127.0.0.1";
    int port = 3491;
    DltDaemonLocal daemon_local;
    DltGateway *gateway = &daemon_local.pGateway;
    DltGatewayConnection connections;
    gateway->num_connections = 1;
    gateway->connections = &connections;
    gateway->connections->status = DLT_GATEWAY_INITIALIZED;
    gateway->connections->trigger = DLT_GATEWAY_ON_STARTUP;
    gateway->connections->client.mode = DLT_CLIENT_MODE_TCP;
    gateway->connections->client.servIP = ip;
    gateway->connections->client.port = port;


    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_establish_connections(gateway, &daemon_local, 0));
}

TEST(t_dlt_gateway_establish_connections, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_establish_connections(NULL, NULL, 0));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_get_connection_receiver*/
TEST(t_dlt_gateway_get_connection_receiver, normal)
{
    DltReceiver *ret = NULL;
    DltGateway gateway;
    DltGatewayConnection connections;
    memset(&gateway, 0, sizeof(DltGateway));
    memset(&connections, 0, sizeof(DltGatewayConnection));
    gateway.connections = &connections;
    int fd = 10;
    gateway.num_connections = 1;
    gateway.connections->client.sock = fd;
    gateway.connections->client.receiver.fd = 12;
    gateway.connections->status = DLT_GATEWAY_CONNECTED;
    ret = dlt_gateway_get_connection_receiver(&gateway, fd);

    EXPECT_EQ(12, ret->fd);
}

TEST(t_dlt_gateway_get_connection_receiver, abnormal)
{
    DltReceiver *ret = NULL;
    DltGateway gateway;
    DltGatewayConnection connections;
    memset(&gateway, 0, sizeof(DltGateway));
    memset(&connections, 0, sizeof(DltGatewayConnection));
    gateway.connections = &connections;
    int fd = 10;
    gateway.num_connections = 1;
    gateway.connections->client.sock = fd;
    gateway.connections->client.receiver.fd = 12;
    ret = dlt_gateway_get_connection_receiver(&gateway, fd);

    EXPECT_EQ(NULL, ret);
}

TEST(t_dlt_gateway_get_connection_receiver, nullpointer)
{
    DltReceiver *ret;
    ret = dlt_gateway_get_connection_receiver(NULL, 0);

    EXPECT_EQ(NULL, ret);
}

/* Begin Method: dlt_gateway::t_dlt_gateway_parse_get_log_info*/
TEST(t_dlt_gateway_parse_get_log_info, normal)
{
    int32_t len;
    int32_t ret = DLT_RETURN_ERROR;
    DltDaemon daemon;
    DltGateway gateway;
    DltMessage msg;
    char ecuid[] = "ECU2";
    uint32_t sid = DLT_SERVICE_ID_GET_LOG_INFO;
    uint8_t status = 7;
    uint16_t count_app_ids = 1;
    uint16_t count_context_ids = 1;
    const char *apid = "LOG";
    const char *ctid = "TEST";
    const char *com = "remo";
    char app_description[] = "Test Application for Logging";
    char context_description[] = "Test Context for Logging";
    uint16_t len_app = 0;
    uint16_t len_con = 0;
    int8_t log_level = -1;
    int8_t trace_status = -1;
    int offset = 0;

    memset(&daemon, 0, sizeof(DltDaemon));
    dlt_set_id(daemon.ecuid, ecuid);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));

    ret = strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE);
    ASSERT_EQ(DLT_RETURN_OK, ret);

    /* create response message */
    msg.datasize = sizeof(DltServiceGetLogInfoResponse) +
        sizeof(AppIDsType) +
        sizeof(ContextIDsInfoType) +
        strlen(app_description) +
        strlen(context_description);
    msg.databuffer = (uint8_t *)malloc(msg.datasize);
    msg.databuffersize = msg.datasize;
    memset(msg.databuffer, 0, msg.datasize);

    memcpy(msg.databuffer, &sid, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(msg.databuffer + offset, &status, sizeof(int8_t));
    offset += sizeof(int8_t);

    memcpy(msg.databuffer + offset, &count_app_ids, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    dlt_set_id((char *)(msg.databuffer + offset), apid);
    offset += sizeof(ID4);

    memcpy(msg.databuffer + offset, &count_context_ids, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    dlt_set_id((char *)(msg.databuffer + offset), ctid);
    offset += sizeof(ID4);

    memcpy(msg.databuffer + offset, &log_level, sizeof(int8_t));
    offset += sizeof(int8_t);

    memcpy(msg.databuffer + offset, &trace_status, sizeof(int8_t));
    offset += sizeof(int8_t);

    len_con = strlen(context_description);
    memcpy(msg.databuffer + offset, &len_con, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    memcpy(msg.databuffer + offset, context_description, strlen(context_description));
    offset += strlen(context_description);

    len_app = strlen(app_description);
    memcpy(msg.databuffer + offset, &len_app, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    memcpy(msg.databuffer + offset, app_description, strlen(app_description));
    offset += strlen(app_description);

    dlt_set_id((char *)(msg.databuffer + offset), com);

    msg.storageheader = (DltStorageHeader *)msg.headerbuffer;
    dlt_set_storageheader(msg.storageheader, "");

    msg.standardheader = (DltStandardHeader *)(msg.headerbuffer + sizeof(DltStorageHeader));
    msg.standardheader->htyp = DLT_HTYP_WEID | DLT_HTYP_WTMS | DLT_HTYP_UEH | DLT_HTYP_PROTOCOL_VERSION1;
    msg.standardheader->mcnt = 0;

    dlt_set_id(msg.headerextra.ecu, ecuid);
    msg.headerextra.tmsp = dlt_uptime();
    dlt_message_set_extraparameters(&msg, 0);

    msg.extendedheader = (DltExtendedHeader *)(msg.headerbuffer +
                                               sizeof(DltStorageHeader) +
                                               sizeof(DltStandardHeader) +
                                               DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp));
    msg.extendedheader->msin = DLT_MSIN_CONTROL_RESPONSE;
    msg.extendedheader->noar = 1;
    dlt_set_id(msg.extendedheader->apid, "");
    dlt_set_id(msg.extendedheader->ctid, "");

    msg.headersize = sizeof(DltStorageHeader) +
        sizeof(DltStandardHeader) +
        sizeof(DltExtendedHeader) +
        DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp);
    len = msg.headersize - sizeof(DltStorageHeader) + msg.datasize;
    msg.standardheader->len = DLT_HTOBE_16(len);

    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_parse_get_log_info(&daemon, ecuid, &msg, CONTROL_MESSAGE_NOT_REQUESTED, 0));
}

TEST(t_dlt_gateway_parse_get_log_info, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_parse_get_log_info(NULL, NULL, NULL, 0, 0));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_process_passive_node_messages*/
TEST(t_dlt_gateway_process_passive_node_messages, normal)
{
    DltDaemon daemon;
    DltDaemonLocal daemon_local;
    DltReceiver receiver;
    DltGatewayConnection connections;
    memset(&daemon, 0, sizeof(DltDaemon));
    memset(&daemon_local, 0, sizeof(DltDaemonLocal));
    memset(&receiver, 0, sizeof(DltReceiver));
    memset(&connections, 0, sizeof(DltGatewayConnection));
    daemon_local.pGateway.connections = &connections;
    daemon_local.pGateway.num_connections = 1;
    daemon_local.pGateway.connections->status = DLT_GATEWAY_CONNECTED;

    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_process_passive_node_messages(&daemon, &daemon_local, &receiver, 1));
}

TEST(t_dlt_gateway_process_passive_node_messages, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_process_passive_node_messages(NULL, NULL, NULL, 0));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_process_gateway_timer*/
TEST(t_dlt_gateway_process_gateway_timer, normal)
{
    char ECUVersionString[] = "12.34";
    char ip[] = "127.0.0.1";
    int port = 3491;
    DltDaemon daemon;
    DltDaemonLocal daemon_local;
    DltReceiver receiver;
    DltGatewayConnection connections;
    DltConnection connections1;
    daemon_local.pGateway.connections = &connections;
    daemon_local.pGateway.num_connections = 1;
    DltLogStorage storage_handle;
    daemon_local.pGateway.connections->status = DLT_GATEWAY_INITIALIZED;
    daemon_local.pGateway.connections->trigger = DLT_GATEWAY_ON_STARTUP;
    daemon_local.pGateway.connections->client.mode = DLT_CLIENT_MODE_TCP;
    daemon_local.pGateway.connections->client.servIP = ip;
    daemon_local.pGateway.connections->client.port = port;


    daemon_local.pEvent.connections = &connections1;
    daemon_local.pEvent.connections->receiver = &receiver;
    daemon.ECUVersionString = ECUVersionString;
    daemon.storage_handle = &storage_handle;
    daemon_local.pEvent.connections->receiver->fd = -1;

    EXPECT_EQ(DLT_RETURN_OK,
              dlt_gateway_process_gateway_timer(&daemon, &daemon_local, daemon_local.pEvent.connections->receiver, 1));
}

TEST(t_dlt_gateway_process_gateway_timer, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_process_gateway_timer(NULL, NULL, NULL, 0));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_process_on_demand_request*/
TEST(t_dlt_gateway_process_on_demand_request, normal)
{
    char node_id[DLT_ID_SIZE] = "123";
    uint32_t connection_status = 1;
    DltDaemonLocal daemon_local;
    DltGatewayConnection connections;
    daemon_local.pGateway.connections = &connections;
    daemon_local.pGateway.num_connections = 1;
    connections.status = DLT_GATEWAY_CONNECTED;
    connections.trigger = DLT_GATEWAY_ON_STARTUP;
    connections.ecuid = node_id;

    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_process_on_demand_request(&daemon_local.pGateway,
                                                                   &daemon_local,
                                                                   node_id,
                                                                   connection_status,
                                                                   1));
}

TEST(t_dlt_gateway_process_on_demand_request, abnormal)
{
    char node_id[DLT_ID_SIZE] = "123";
    uint32_t connection_status = 1;
    DltDaemonLocal daemon_local;
    DltGatewayConnection connections;
    daemon_local.pGateway.connections = &connections;
    daemon_local.pGateway.num_connections = 1;
    connections.status = DLT_GATEWAY_INITIALIZED;
    connections.ecuid = node_id;

    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_process_on_demand_request(&daemon_local.pGateway,
                                                                      &daemon_local,
                                                                      node_id,
                                                                      connection_status,
                                                                      0));
}

TEST(t_dlt_gateway_process_on_demand_request, nullpointer)
{
    char node_id[DLT_ID_SIZE] = "123";
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_process_on_demand_request(NULL, NULL, node_id, 1, 0));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_check_param*/
TEST(t_dlt_gateway_check_param, normal)
{
    char value_1[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = "10.11.22.33";
    char value_2[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = "3490";
    DltGateway gateway;
    DltGatewayConnection tmp;
    gateway.connections = &tmp;

    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_check_param(&gateway,
                                                     &tmp,
                                                     GW_CONF_IP_ADDRESS,
                                                     value_1));

    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_check_param(&gateway,
                                                     &tmp,
                                                     GW_CONF_PORT,
                                                     value_2));
}

TEST(t_dlt_gateway_check_param, abnormal)
{
    char value_1[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = "10.11.22.33";
    DltGateway gateway;
    DltGatewayConnection tmp;
    gateway.connections = &tmp;

    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_check_param(&gateway,
                                                        &tmp,
                                                        GW_CONF_PORT,
                                                        value_1));
}

TEST(t_dlt_gateway_check_param, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_check_param(NULL, NULL, GW_CONF_PORT, 0));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_configure*/
TEST(t_dlt_gateway_configure, Normal)
{
    DltGateway gateway;
    DltGatewayConnection tmp;
    gateway.connections = &tmp;
    gateway.num_connections = 1;
    char gatewayConfigFile[DLT_DAEMON_FLAG_MAX];
    strncpy(gatewayConfigFile, "/tmp/dlt_gateway.conf", DLT_DAEMON_FLAG_MAX - 1);

    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_configure(&gateway, gatewayConfigFile, 0));
}

TEST(t_dlt_gateway_configure, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_gateway_configure(NULL, NULL, 0));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
/*    ::testing::FLAGS_gtest_break_on_failure = true; */
/*    ::testing::FLAGS_gtest_filter = "t_dlt_gateway_process_passive_node_messages*"; */
    return RUN_ALL_TESTS();
}
