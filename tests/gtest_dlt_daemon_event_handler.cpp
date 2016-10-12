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
 * \copyright Copyright © 2016 Advanced Driver Information Technology.
 *
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file gtest_dlt_daemon_event_handler.cpp
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: gtest_dlt_daemon_event_handler.cpp                            **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Onkar Palkar onkar.palkar@wipro.com                           **
**  PURPOSE   : Unit test connection and event handling                       **
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
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int connectServer(void);

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
    #include "dlt_daemon_client.h"
}

/* Begin Method: dlt_daemon_event_handler::t_dlt_daemon_prepare_event_handling*/
TEST(t_dlt_daemon_prepare_event_handling, normal)
{
    DltEventHandler ev;
    EXPECT_EQ(DLT_RETURN_OK, dlt_daemon_prepare_event_handling(&ev));
}

TEST(t_dlt_daemon_prepare_event_handling, nullpointer)
{
    // NULL-Pointer, expect -1
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_daemon_prepare_event_handling(NULL));
}

/* Begin Method: dlt_daemon_event_handler::t_dlt_daemon_handle_event*/
TEST(t_dlt_daemon_handle_event, normal)
{
    DltDaemonLocal daemon_local;
    DltDaemon daemon;

    EXPECT_EQ(DLT_RETURN_OK, dlt_daemon_prepare_event_handling(&daemon_local.pEvent));
    EXPECT_EQ(DLT_RETURN_OK, dlt_daemon_handle_event(&daemon_local.pEvent,
                                       &daemon,
                                       &daemon_local));
}

TEST(t_dlt_daemon_handle_event, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_daemon_handle_event(NULL,NULL,NULL));
}

/* Begin Method: dlt_daemon_event_handler::dlt_event_handler_find_connection*/
TEST(t_dlt_event_handler_find_connection, normal)
{
    int fd = 10;
    DltEventHandler ev;
    DltConnection connections;
    DltConnection *ret;
    DltReceiver receiver;
    ev.connections = &connections;
    ev.connections->receiver = &receiver;
    ev.connections->receiver->fd = fd;

    ret = dlt_event_handler_find_connection(&ev, fd);
    EXPECT_EQ(10, ret->receiver->fd);
}

/* Begin Method: dlt_daemon_event_handler::dlt_daemon_add_connection*/
TEST(t_dlt_daemon_add_connection, normal)
{
    DltEventHandler ev1;
    DltConnection *head;
    ev1.connections = (DltConnection *)malloc(sizeof(DltConnection));
    head = (DltConnection*)ev1.connections;
    memset(ev1.connections, 0, sizeof(DltConnection));
    ev1.connections->next = 0;
    ev1.connections->type = DLT_CONNECTION_CLIENT_MSG_SERIAL;

    DltConnection *connections1;
    connections1 = (DltConnection *)malloc(sizeof(DltConnection));
    memset(connections1, 0, sizeof(DltConnection));
    connections1->next = 0;
    connections1->type = DLT_CONNECTION_GATEWAY;

    DltReceiver receiver;
    connections1->receiver = &receiver;

    EXPECT_EQ(DLT_CONNECTION_CLIENT_MSG_SERIAL, ev1.connections->type);
    dlt_daemon_add_connection(&ev1, connections1);
    head = (DltConnection*)ev1.connections->next;
    EXPECT_EQ(DLT_CONNECTION_GATEWAY, head->type);

    free(ev1.connections);
    free(connections1);
}

/* Begin Method: dlt_daemon_event_handler::dlt_daemon_remove_connection*/
TEST(t_dlt_daemon_remove_connection, normal)
{
    DltEventHandler ev1;
    DltConnection *head;
    ev1.connections = (DltConnection *)malloc(sizeof(DltConnection));
    head = (DltConnection*)ev1.connections;
    memset(ev1.connections, 0, sizeof(DltConnection));
    ev1.connections->next = 0;
    ev1.connections->type = DLT_CONNECTION_CLIENT_MSG_SERIAL;

    DltConnection *connections1;
    connections1 = (DltConnection *)malloc(sizeof(DltConnection));
    memset(connections1, 0, sizeof(DltConnection));
    connections1->next = 0;
    connections1->type = DLT_CONNECTION_GATEWAY;

    DltReceiver receiver;
    connections1->receiver = &receiver;

    EXPECT_EQ(DLT_CONNECTION_CLIENT_MSG_SERIAL, ev1.connections->type);

    dlt_daemon_add_connection(&ev1, connections1);
    head = (DltConnection*)ev1.connections->next;

    EXPECT_EQ(DLT_CONNECTION_GATEWAY, head->type);

    EXPECT_EQ(DLT_RETURN_OK, dlt_daemon_remove_connection(&ev1, connections1));
}

/* Begin Method: dlt_daemon_event_handler::dlt_event_handler_cleanup_connections*/
TEST(t_dlt_event_handler_cleanup_connections, normal)
{
    DltEventHandler ev1;
    ev1.connections = (DltConnection *)malloc(sizeof(DltConnection));
    memset(ev1.connections, 0, sizeof(DltConnection));
    ev1.connections->next = 0;
    ev1.connections->type = DLT_CONNECTION_GATEWAY;
    DltReceiver receiver1;
    ev1.connections->receiver = &receiver1;

    dlt_event_handler_cleanup_connections(&ev1);
}

/* Begin Method: dlt_daemon_event_handler::dlt_connection_check_activate*/
TEST(t_dlt_connection_check_activate, normal)
{
    int ret;
    DltEventHandler evhdl;
    DltConnection con;
    DltReceiver receiver;
    con.receiver = &receiver;
    con.status = INACTIVE;
    con.type = DLT_CONNECTION_CLIENT_MSG_TCP;
    con.receiver->fd = 1;
    evhdl.epfd = epoll_create(1);
    ret = dlt_connection_check_activate(&evhdl, &con, ACTIVATE);

    EXPECT_EQ(DLT_RETURN_OK, ret);

    ret = dlt_connection_check_activate(&evhdl, &con, DEACTIVATE);
    EXPECT_EQ(DLT_RETURN_OK, ret);

}

TEST(t_dlt_connection_check_activate, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_connection_check_activate(NULL, NULL, DEACTIVATE));
}

/* Begin Method: dlt_daemon_event_handler::dlt_event_handler_register_connection*/
TEST(t_dlt_event_handler_register_connection, normal)
{
    int ret = 0;
    DltDaemonLocal daemon_local;
    int mask = 0;
    DltEventHandler ev1;
    ev1.connections = (DltConnection *)malloc(sizeof(DltConnection));
    memset(ev1.connections, 0, sizeof(DltConnection));
    ev1.connections->next = 0;
    ev1.connections->type = DLT_CONNECTION_CLIENT_MSG_SERIAL;
    DltConnection *connections1;
    connections1 = (DltConnection *)malloc(sizeof(DltConnection));
    memset(connections1, 0, sizeof(DltConnection));
    connections1->next = 0;
    connections1->type = DLT_CONNECTION_GATEWAY;

    DltReceiver receiver;
    connections1->receiver = &receiver;

    EXPECT_EQ(DLT_CONNECTION_CLIENT_MSG_SERIAL, ev1.connections->type);
    ev1.epfd = epoll_create(1);
    ret = dlt_event_handler_register_connection(&ev1, &daemon_local, connections1, mask);

    EXPECT_EQ(DLT_RETURN_OK, ret);
    close(ev1.epfd);
    free(ev1.connections);
    free(connections1);
}

TEST(t_dlt_event_handler_register_connection, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_event_handler_register_connection(NULL, NULL, NULL, 1));
}

/* Begin Method: dlt_daemon_event_handler::dlt_event_handler_unregister_connection*/
TEST(t_dlt_event_handler_unregister_connection, normal)
{
    int ret = 0;
    int fd = 0;
    int mask = 0;
    DltDaemonLocal daemon_local;
    DltEventHandler ev1;
    ev1.connections = (DltConnection *)malloc(sizeof(DltConnection));
    memset(ev1.connections, 0, sizeof(DltConnection));
    ev1.connections->next = 0;
    ev1.connections->type = DLT_CONNECTION_GATEWAY;
    DltConnection *connections1;
    connections1 = (DltConnection *)malloc(sizeof(DltConnection));
    memset(connections1, 0, sizeof(DltConnection));
    connections1->next = 0;
    connections1->type = DLT_CONNECTION_GATEWAY;
    DltReceiver receiver;
    connections1->receiver = &receiver;


    DltReceiver receiver1;
    ev1.connections->receiver = &receiver1;
    ev1.connections->receiver->fd = fd;
    ev1.epfd = epoll_create(1);
    ret = dlt_event_handler_register_connection(&ev1, &daemon_local, connections1, mask);
    EXPECT_EQ(DLT_RETURN_OK, ret);

    ret = dlt_event_handler_unregister_connection(&ev1, &daemon_local, fd);
    EXPECT_EQ(DLT_RETURN_OK, ret);
    close(ev1.epfd);
    free(connections1);
}

/* Begin Method: dlt_daemon_connections::dlt_connection_create*/
TEST(t_dlt_connection_create, normal)
{
    int fd = 1;
    int ret = 0;

    DltDaemonLocal daemon_local;
    DltConnection connections;
    DltReceiver receiver;
    daemon_local.pEvent.connections = &connections;
    daemon_local.pEvent.connections->receiver = &receiver;
    daemon_local.pEvent.epfd =  epoll_create(5);

    ret = dlt_connection_create(&daemon_local,
                                &daemon_local.pEvent,
                                fd,
                                EPOLLIN,
                                DLT_CONNECTION_CLIENT_MSG_SERIAL);
    EXPECT_EQ(DLT_RETURN_OK, ret);
}

/* Begin Method: dlt_daemon_connections::dlt_connection_destroy*/
TEST(t_dlt_connection_destroy, normal)
{
    DltConnection *to_destroy;

    to_destroy = (DltConnection *)malloc(sizeof(DltConnection));
    memset(to_destroy, 0, sizeof(DltConnection));
    to_destroy->next = 0;
    to_destroy->type = DLT_CONNECTION_CLIENT_MSG_SERIAL;

    to_destroy->receiver = (DltReceiver *)malloc(sizeof(DltReceiver));
    memset(to_destroy->receiver, 0, sizeof(DltReceiver));
    to_destroy->receiver->fd = -1;
    to_destroy->receiver->buffer = NULL;
    dlt_connection_destroy(to_destroy);
}

/* Begin Method: dlt_daemon_connections::t_dlt_connection_destroy_receiver*/
TEST(t_dlt_connection_destroy_receiver, normal)
{
    DltConnection *to_destroy;

    to_destroy = (DltConnection *)malloc(sizeof(DltConnection));
    memset(to_destroy, 0, sizeof(DltConnection));
    to_destroy->next = 0;
    to_destroy->type = DLT_CONNECTION_CLIENT_MSG_SERIAL;

    to_destroy->receiver = (DltReceiver *)malloc(sizeof(DltReceiver));
    memset(to_destroy->receiver, 0, sizeof(DltReceiver));
    to_destroy->receiver->fd = -1;
    to_destroy->receiver->buffer = NULL;

    dlt_connection_destroy_receiver(to_destroy);
    free(to_destroy);
}

/* Begin Method: dlt_daemon_connections::t_dlt_connection_get_receiver*/
TEST(t_dlt_connection_get_receiver, normal)
{
    int fd = 10;
    DltReceiver *ret;
    DltDaemonLocal daemon_local;
    ret = dlt_connection_get_receiver(&daemon_local, DLT_CONNECTION_CLIENT_MSG_TCP, fd);

    EXPECT_EQ(fd, ret->fd);
}

/* Begin Method: dlt_daemon_connections::(t_dlt_connection_get_next*/
TEST(t_dlt_connection_get_next, normal)
{
    int type_mask =
        (DLT_CON_MASK_CLIENT_MSG_TCP | DLT_CON_MASK_CLIENT_MSG_SERIAL);
    DltConnection *ret;

    DltConnection *current;
    current = (DltConnection *)malloc(sizeof(DltConnection));
    memset(current, 0, sizeof(DltConnection));
    current->next = NULL;

    DltConnection *node;
    node = (DltConnection *)malloc(sizeof(DltConnection));
    memset(node, 0, sizeof(DltConnection));
    node->next = NULL;

    current->next = node;

    current->type = DLT_CONNECTION_ONE_S_TIMER;
    node->type = DLT_CONNECTION_CLIENT_MSG_TCP;

    ret = dlt_connection_get_next(current, type_mask);

    EXPECT_EQ(DLT_CONNECTION_CLIENT_MSG_TCP, ret->type);
    EXPECT_NE(DLT_CONNECTION_ONE_S_TIMER, ret->type);

    free(current);
    free(node);
}

/* Begin Method: dlt_daemon_connections::(t_dlt_connection_get_next*/
TEST(t_dlt_connection_get_next, abnormal)
{
    int type_mask =
        (DLT_CON_MASK_CLIENT_MSG_TCP | DLT_CON_MASK_CLIENT_MSG_SERIAL);
    DltConnection *ret;

    DltConnection *current;
    current = (DltConnection *)malloc(sizeof(DltConnection));
    memset(current, 0, sizeof(DltConnection));
    current->next = NULL;

    DltConnection *node;
    node = (DltConnection *)malloc(sizeof(DltConnection));
    memset(node, 0, sizeof(DltConnection));
    node->next = NULL;

    current->next = node;

    current->type = DLT_CONNECTION_CLIENT_MSG_SERIAL;
    node->type = DLT_CONNECTION_CLIENT_MSG_TCP;

    ret = dlt_connection_get_next(current, type_mask);

    EXPECT_NE(DLT_CONNECTION_CLIENT_MSG_TCP, ret->type);
    EXPECT_EQ(DLT_CONNECTION_CLIENT_MSG_SERIAL, ret->type);

    free(current);
    free(node);
}

/* Begin Method: dlt_daemon_connections::t_dlt_connection_send*/
TEST(t_dlt_connection_send, normal_1)
{
    int ret = 0;
    DltConnection conn;
    DltReceiver receiver;
    receiver.fd = connectServer();
    EXPECT_NE(-1,receiver.fd);
    conn.receiver = &receiver;
    conn.type = DLT_CONNECTION_CLIENT_MSG_TCP;
    void *data1;
    int size1 = 0;
    DltDaemonLocal daemon_local;
    data1 = daemon_local.msg.databuffer;
    size1 = daemon_local.msg.datasize;

    ret = dlt_connection_send(&conn, data1, size1);

    EXPECT_EQ(DLT_RETURN_OK, ret);
}

TEST(t_dlt_connection_send, normal_2)
{
    int ret = 0;
    DltConnection conn;
    DltReceiver receiver;
    receiver.fd = 1;
    conn.receiver = &receiver;
    conn.type = DLT_CONNECTION_CLIENT_MSG_SERIAL;
    ret = dlt_connection_send(&conn,
                             (void *)dltSerialHeader,
                             sizeof(dltSerialHeader));
    EXPECT_EQ(sizeof(dltSerialHeader), ret);
}

TEST(t_dlt_connection_send, abnormal)
{
    int ret = 0;
    DltConnection conn;
    DltReceiver receiver;
    conn.receiver = &receiver;
    conn.type = DLT_CONNECTION_TYPE_MAX;
    ret = dlt_connection_send(&conn,
                             (void *)dltSerialHeader,
                             sizeof(dltSerialHeader));
    EXPECT_EQ(DLT_RETURN_ERROR, ret);
}

/* Begin Method: dlt_daemon_connections::t_dlt_connection_send_multiple*/
TEST(t_dlt_connection_send_multiple, normal_1)
{
    int ret = 0;
    void *data1, *data2;
    int size1 = 0;
    int size2 = 0;
    DltDaemonLocal daemon_local;

    data1 = daemon_local.msg.headerbuffer + sizeof(DltStorageHeader);
    size1 = daemon_local.msg.headersize - sizeof(DltStorageHeader);
    data2 = daemon_local.msg.databuffer;
    size2 = daemon_local.msg.datasize;

    DltConnection conn;
    DltReceiver receiver;
    receiver.fd = connectServer();
    EXPECT_NE(-1,receiver.fd);
    conn.receiver = &receiver;
    conn.type = DLT_CONNECTION_CLIENT_MSG_TCP;
    ret = dlt_connection_send_multiple(&conn,
                                      data1,
                                      size1,
                                      data2,
                                      size2,
                                      1);
    EXPECT_EQ(DLT_RETURN_OK, ret);
    close(receiver.fd);
}

TEST(t_dlt_connection_send_multiple, normal_2)
{
    int ret = 0;
    void *data1, *data2;
    int size1 = 0;
    int size2 = 0;
    DltDaemonLocal daemon_local;

    data1 = daemon_local.msg.headerbuffer + sizeof(DltStorageHeader);
    size1 = daemon_local.msg.headersize - sizeof(DltStorageHeader);
    data2 = daemon_local.msg.databuffer;
    size2 = daemon_local.msg.datasize;

    DltConnection conn;
    DltReceiver receiver;
    receiver.fd = connectServer();
    EXPECT_NE(-1,receiver.fd);
    conn.receiver = &receiver;
    conn.type = DLT_CONNECTION_CLIENT_MSG_TCP;

    ret = dlt_connection_send_multiple(&conn,
                                      data1,
                                      size1,
                                      data2,
                                      size2,
                                      0);
    EXPECT_EQ(DLT_RETURN_OK, ret);
    close(receiver.fd);
}

TEST(t_dlt_connection_send_multiple, nullpointer)
{
    int ret = 0;
    void *data1, *data2;
    int size1 = 0;
    int size2 = 0;
    DltDaemonLocal daemon_local;

    data1 = daemon_local.msg.headerbuffer + sizeof(DltStorageHeader);
    size1 = daemon_local.msg.headersize - sizeof(DltStorageHeader);
    data2 = daemon_local.msg.databuffer;
    size2 = daemon_local.msg.datasize;

    ret = dlt_connection_send_multiple(NULL,
                                      data1,
                                      size1,
                                      data2,
                                      size2,
                                      0);
    EXPECT_EQ(DLT_RETURN_ERROR, ret);
}

int connectServer(void)
{
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    portno = 8080;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server = gethostbyname("127.0.0.1");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        printf("Error in connect socket\n");
        return -1;
    }
    return sockfd;
}

int main(int argc, char **argv)
{
    pid_t cpid;
    cpid = fork();
    if(cpid == -1)
    {
        printf("fork fail\n");
        return -1;
    }
    if (cpid)
    {
        int i = 2;
        int j;
        char buffer[256];
        int sockfd, newsockfd, portno;
        socklen_t clilen;
        struct sockaddr_in serv_addr, cli_addr;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd == -1)
        {
            printf("Error in creating socket\n");
            return -1;
        }
        bzero((char *) &serv_addr, sizeof(serv_addr));
        portno = 8080;

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
        j = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
        if(j == -1)
        {
            perror("Bind Error\n");
            return -1;
        }
        listen(sockfd,5);
        while(i)
        {
            clilen = sizeof(cli_addr);
            newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
            if(newsockfd == -1)
            {
                printf("Error in accept");
                return -1;
            }
            bzero(buffer,256);
            (void)(read(newsockfd,buffer,255) + 1); /* just ignore result */
            i--;
            close(newsockfd);
        }
        close(sockfd);
    }
    else
    {

        ::testing::InitGoogleTest(&argc, argv);
        ::testing::FLAGS_gtest_break_on_failure = false;
        //::testing::FLAGS_gtest_filter = "t_dlt_connection_send_multiple*";
        return RUN_ALL_TESTS();
    }
    return 0;
}
