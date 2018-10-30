/*
 * @licence app begin@
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2016 Advanced Driver Information Technology.
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
#include "dlt-daemon.h"
#include "dlt_user.h"
#include "dlt_user_shared.h"
#include "dlt_user_shared_cfg.h"
#include "dlt_user_cfg.h"
#include "dlt-daemon_cfg.h"
#include "dlt_version.h"
#include "dlt_gateway.h"
#include "dlt_daemon_common.h"
#include "dlt_daemon_connection_types.h"
#include "dlt_daemon_event_handler.h"
#include "dlt_daemon_connection.h"
#include "dlt_daemon_event_handler_types.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <limits.h>
#include <errno.h>
#include "dlt_config_file_parser.h"
#include "dlt_common.h"
#include "dlt-daemon_cfg.h"
#include "dlt_daemon_event_handler.h"
#include "dlt_daemon_connection.h"
#include "dlt_daemon_client.h"
}

/* Begin Method: dlt_gateway::t_dlt_gateway_init*/
TEST(t_dlt_gateway_init, normal)
{
    DltDaemonLocal daemon_local;
    DltGatewayConnection connections;
    daemon_local.pGateway.connections = &connections;
    daemon_local.pGateway.num_connections = 1;

    DltConnection connections1;
    DltReceiver receiver;
    daemon_local.pEvent.connections = &connections1;
    daemon_local.pEvent.connections->receiver = &receiver;
    daemon_local.pEvent.connections->next = NULL;

    memset(daemon_local.flags.gatewayConfigFile,0,DLT_DAEMON_FLAG_MAX);
    strncpy(daemon_local.flags.gatewayConfigFile, "/tmp/dlt_gateway.conf", DLT_DAEMON_FLAG_MAX - 1);

    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_init(&daemon_local, 1));
}

TEST(t_dlt_gateway_init, nullpointer)
{
   // NULL-Pointer, expect -1
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_init(NULL, 0));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_send_control_message*/
TEST(t_dlt_gateway_send_control_message, Normal)
{
    int ret = 0;
    DltDaemonLocal daemon_local;
    DltGatewayConnection connections;
    DltConnection connections1;
    DltReceiver receiver1;
    daemon_local.pGateway.connections = &connections;
    daemon_local.pEvent.connections = &connections1;
    daemon_local.pEvent.connections->next = NULL;
    daemon_local.pEvent.connections->receiver = &receiver1;
    memset(daemon_local.flags.gatewayConfigFile,0,DLT_DAEMON_FLAG_MAX);
    strncpy(daemon_local.flags.gatewayConfigFile, "/tmp/dlt_gateway.conf", DLT_DAEMON_FLAG_MAX - 1);

    ret = dlt_gateway_init(&daemon_local, 0);
    EXPECT_EQ(DLT_RETURN_OK, ret);

    dlt_gateway_send_control_message(daemon_local.pGateway.connections, &daemon_local.pGateway, &daemon_local, 0);
}

TEST(t_dlt_gateway_send_control_message, nullpointer)
{
    //NULL-Pointer, expect -1
    dlt_gateway_send_control_message(NULL, NULL, NULL, 0);
}

/* Begin Method: dlt_gateway::t_dlt_gateway_store_connection*/
TEST(t_dlt_gateway_store_connection, normal)
{
    char ip_address[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = "10.113.100.100";
    char ecuid[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = "1234";
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
}

TEST(t_dlt_gateway_store_connection, nullpointer)
{
    // NULL-Pointer, expect -1
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_store_connection(NULL , NULL, 0));
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
    // NULL-Pointer, expect -1
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_check_ip(NULL,NULL));
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
    // NULL-Pointer, expect -1
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_check_send_serial(NULL,NULL));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_check_control_messages*/
TEST(t_dlt_gateway_check_control_messages, normal)
{
    DltGatewayConnection tmp;
    DltGatewayConnection *con;
    char value[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = "1,2,3,4,5";
    con = &tmp;
    con->control_msgs[0] = DLT_SERVICE_ID_LAST_ENTRY;
    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_check_control_messages(con, value));
}

TEST(t_dlt_gateway_check_control_messages, nullpointer)
{
    // NULL-Pointer, expect -1
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_check_control_messages(NULL,NULL));
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
    // NULL-Pointer, expect -1
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_check_port(NULL,NULL));
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
    // NULL-Pointer, expect -1
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_check_ecu(NULL,NULL));
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
    // NULL-Pointer, expect -1
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_check_connect_trigger(NULL,NULL));
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
    char value[DLT_CONFIG_FILE_ENTRY_MAX_LEN] = "0";
    con = &tmp;
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_check_timeout(con, value));
}

TEST(t_dlt_gateway_check_timeout, nullpointer)
{
    // NULL-Pointer, expect -1
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_check_timeout(NULL,NULL));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_establish_connections*/
TEST(t_dlt_gateway_establish_connections, normal)
{
    DltDaemonLocal daemon_local;
    DltGateway *gateway = &daemon_local.pGateway;
    DltGatewayConnection connections;
    gateway->num_connections = 1;
    gateway->connections = &connections;
    gateway->connections->status = DLT_GATEWAY_CONNECTED;
    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_establish_connections(gateway, &daemon_local, 0));
}

TEST(t_dlt_gateway_establish_connections, nullpointer)
{
    // NULL-Pointer, expect -1
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_establish_connections(NULL,NULL,0));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_get_connection_receiver*/
TEST(t_dlt_gateway_get_connection_receiver, normal)
{
    DltReceiver *ret = NULL;
    DltGateway gateway;
    DltGatewayConnection connections;
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
    // NULL-Pointer, expect -1
    DltReceiver *ret;
    ret = dlt_gateway_get_connection_receiver(NULL, 0);
    EXPECT_EQ(NULL, ret);
}

/* Begin Method: dlt_gateway::t_dlt_gateway_process_passive_node_messages*/
TEST(t_dlt_gateway_process_passive_node_messages, normal)
{
    DltDaemon daemon;
    DltDaemonLocal daemon_local;
    DltReceiver receiver;
    DltGatewayConnection connections;
    daemon_local.pGateway.connections = &connections;
    daemon_local.pGateway.num_connections = 1;
    daemon_local.pGateway.connections->status = DLT_GATEWAY_CONNECTED;
    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_process_passive_node_messages(&daemon,&daemon_local,&receiver,1));
}

TEST(t_dlt_gateway_process_passive_node_messages, nullpointer)
{
    // NULL-Pointer, expect -1
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_process_passive_node_messages(NULL, NULL, NULL, 0));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_process_gateway_timer*/
TEST(t_dlt_gateway_process_gateway_timer, normal)
{
    char ECUVersionString[] = "12.34";
    DltDaemon daemon;
    DltDaemonLocal daemon_local;
    DltReceiver receiver;
    DltGatewayConnection connections;
    DltConnection connections1;
    daemon_local.pGateway.connections = &connections;
    daemon_local.pGateway.num_connections = 1;
    DltLogStorage storage_handle;
    daemon_local.pGateway.connections->status = DLT_GATEWAY_CONNECTED;

    daemon_local.pEvent.connections = &connections1;
    daemon_local.pEvent.connections->receiver = &receiver;
    daemon.ECUVersionString = ECUVersionString;
    daemon.storage_handle = &storage_handle;

    daemon_local.pEvent.connections->receiver->fd = -1;

    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_process_gateway_timer(&daemon,&daemon_local,daemon_local.pEvent.connections->receiver,1));
}

TEST(t_dlt_gateway_process_gateway_timer, nullpointer)
{
    // NULL-Pointer, expect -1
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_process_gateway_timer(NULL, NULL, NULL, 0));
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
    // NULL-Pointer, expect -1
    char node_id[DLT_ID_SIZE] = "123";
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_process_on_demand_request(NULL, NULL, node_id, 1, 0));
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
    // NULL-Pointer, expect -1
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_check_param(NULL, NULL, GW_CONF_PORT, 0));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_configure*/
TEST(t_dlt_gateway_configure, Normal)
{
    DltGateway gateway;
    DltGatewayConnection tmp;
    gateway.connections = &tmp;
    gateway.num_connections = 1;
    char gatewayConfigFile[DLT_DAEMON_FLAG_MAX];
    strncpy(gatewayConfigFile, DLT_GATEWAY_CONFIG_PATH, DLT_DAEMON_FLAG_MAX - 1);
    EXPECT_EQ(DLT_RETURN_OK, dlt_gateway_configure(&gateway, gatewayConfigFile, 0));
}

TEST(t_dlt_gateway_configure, nullpointer)
{
    // NULL-Pointer, expect -1
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_configure(NULL, NULL, 0));
}

/* Begin Method: dlt_gateway::t_dlt_gateway_forward_control_message*/
TEST(t_dlt_gateway_forward_control_message, normal)
{
    int ret = 0;
    char ecu[DLT_ID_SIZE] = {'E', 'C', 'U', '1'};
    DltDaemonLocal daemon_local;
    DltGatewayConnection connections;
    DltConnection connections1;
    DltReceiver receiver1;

    daemon_local.pGateway.connections = &connections;
    daemon_local.pEvent.connections = &connections1;
    daemon_local.pEvent.connections->receiver = &receiver1;
    daemon_local.pEvent.connections->receiver->fd = 1;
    daemon_local.pEvent.connections->next = NULL;
    daemon_local.pGateway.num_connections = 1;
    daemon_local.pEvent.connections->type = DLT_CONNECTION_CLIENT_MSG_TCP;

    DltMessage msg;
    memset(daemon_local.flags.gatewayConfigFile,0,DLT_DAEMON_FLAG_MAX);
    strncpy(daemon_local.flags.gatewayConfigFile, "/tmp/dlt_gateway.conf", DLT_DAEMON_FLAG_MAX - 1);

    ret = dlt_gateway_init(&daemon_local, 0);
    EXPECT_EQ(DLT_RETURN_OK, ret);

    ret = dlt_gateway_forward_control_message(&daemon_local.pGateway,
                                                       &daemon_local,
                                                       &msg,
                                                       ecu,
                                                       0);
    EXPECT_EQ(DLT_RETURN_OK, ret);
}

TEST(t_dlt_gateway_forward_control_message, nullpointer)
{
    // NULL-Pointer, expect -1
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_gateway_forward_control_message(NULL, NULL, NULL, NULL, 0));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
//    ::testing::FLAGS_gtest_break_on_failure = true;
//    ::testing::FLAGS_gtest_filter = "t_dlt_gateway_forward_control_message*";
    return RUN_ALL_TESTS();
}
