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

int connectServer(void);

extern "C"
{
    #include "dlt_daemon_event_handler.h"
    #include "dlt_daemon_connection.h"
    #include <netdb.h>
    #include <netinet/in.h>
    #include <sys/types.h>
    #include <sys/socket.h>
}

/* Begin Method: dlt_daemon_event_handler::t_dlt_daemon_prepare_event_handling*/
TEST(t_dlt_daemon_prepare_event_handling, normal)
{
    DltEventHandler ev;

    EXPECT_EQ(DLT_RETURN_OK, dlt_daemon_prepare_event_handling(&ev));
}

TEST(t_dlt_daemon_prepare_event_handling, nullpointer)
{
    /* NULL-Pointer, expect -1 */
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
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_daemon_handle_event(NULL, NULL, NULL));
}

/* Begin Method: dlt_daemon_event_handler::dlt_event_handler_find_connection*/
TEST(t_dlt_event_handler_find_connection, normal)
{
    int fd = 10;
    DltEventHandler ev;
    DltConnection connections;
    DltConnection *ret = nullptr;
    DltReceiver receiver;

    memset(&ev, 0, sizeof(DltEventHandler));
    memset(&connections, 0, sizeof(DltConnection));
    memset(&receiver, 0, sizeof(DltReceiver));

    receiver.fd = fd;

    ev.connections = &connections;
    ev.connections->receiver = &receiver;

    ret = dlt_event_handler_find_connection(&ev, fd);
    EXPECT_EQ(10, ret->receiver->fd);
}

/* Begin Method: dlt_daemon_event_handler::dlt_daemon_add_connection*/
TEST(t_dlt_daemon_add_connection, normal)
{
    DltEventHandler ev1;
    DltConnection *head = nullptr;
    DltConnection *connections1 = nullptr;
    DltReceiver receiver;

    memset(&ev1, 0, sizeof(DltEventHandler));
    memset(&receiver, 0, sizeof(DltReceiver));

    ev1.connections = (DltConnection *)malloc(sizeof(DltConnection));
    head = (DltConnection *)ev1.connections;
    memset(ev1.connections, 0, sizeof(DltConnection));
    ev1.connections->next = 0;
    ev1.connections->type = DLT_CONNECTION_CLIENT_MSG_SERIAL;

    connections1 = (DltConnection *)malloc(sizeof(DltConnection));
    memset(connections1, 0, sizeof(DltConnection));
    connections1->next = 0;
    connections1->type = DLT_CONNECTION_GATEWAY;
    connections1->receiver = &receiver;

    EXPECT_EQ(DLT_CONNECTION_CLIENT_MSG_SERIAL, ev1.connections->type);

    dlt_daemon_add_connection(&ev1, connections1);
    head = (DltConnection *)ev1.connections->next;

    EXPECT_EQ(DLT_CONNECTION_GATEWAY, head->type);

    free(ev1.connections);
    free(connections1);
}

/* Begin Method: dlt_daemon_event_handler::dlt_daemon_remove_connection*/
TEST(t_dlt_daemon_remove_connection, normal)
{
    DltEventHandler ev1;
    DltConnection *head = nullptr;
    DltConnection *connections1 = nullptr;

    memset(&ev1, 0, sizeof(DltEventHandler));

    ev1.connections = (DltConnection *)malloc(sizeof(DltConnection));
    head = (DltConnection *)ev1.connections;
    memset(ev1.connections, 0, sizeof(DltConnection));
    ev1.connections->next = 0;
    ev1.connections->type = DLT_CONNECTION_CLIENT_MSG_SERIAL;

    connections1 = (DltConnection *)malloc(sizeof(DltConnection));
    memset(connections1, 0, sizeof(DltConnection));
    connections1->next = 0;
    connections1->type = DLT_CONNECTION_GATEWAY;
    DltReceiver receiver;
    connections1->receiver = &receiver;

    EXPECT_EQ(DLT_CONNECTION_CLIENT_MSG_SERIAL, ev1.connections->type);

    dlt_daemon_add_connection(&ev1, connections1);
    head = (DltConnection *)ev1.connections->next;

    EXPECT_EQ(DLT_CONNECTION_GATEWAY, head->type);

    EXPECT_EQ(DLT_RETURN_OK, dlt_daemon_remove_connection(&ev1, connections1));
}

/* Begin Method: dlt_daemon_event_handler::dlt_event_handler_cleanup_connections*/
TEST(t_dlt_event_handler_cleanup_connections, normal)
{
    DltEventHandler ev1;
    DltReceiver receiver1;

    memset(&ev1, 0, sizeof(DltEventHandler));
    memset(&receiver1, 0, sizeof(DltReceiver));

    ev1.connections = (DltConnection *)malloc(sizeof(DltConnection));
    memset(ev1.connections, 0, sizeof(DltConnection));
    ev1.connections->next = 0;
    ev1.connections->type = DLT_CONNECTION_GATEWAY;
    ev1.connections->receiver = &receiver1;

    dlt_event_handler_cleanup_connections(&ev1);
}

/* Begin Method: dlt_daemon_event_handler::dlt_connection_check_activate*/
TEST(t_dlt_connection_check_activate, normal)
{
    int ret;
    DltEventHandler evhdl;
    DltReceiver receiver;
    DltConnection con;

    memset(&evhdl, 0, sizeof(DltEventHandler));
    memset(&receiver, 0, sizeof(DltReceiver));
    memset(&con, 0, sizeof(DltConnection));

    receiver.fd = 1;
    con.receiver = &receiver;
    con.status = INACTIVE;
    con.type = DLT_CONNECTION_CLIENT_MSG_TCP;

    EXPECT_EQ(DLT_RETURN_OK, dlt_daemon_prepare_event_handling(&evhdl));

    ret = dlt_connection_check_activate(&evhdl, &con, ACTIVATE);
    EXPECT_EQ(DLT_RETURN_OK, ret);

    ret = dlt_connection_check_activate(&evhdl, &con, DEACTIVATE);
    EXPECT_EQ(DLT_RETURN_OK, ret);

    free(evhdl.pfd);
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
    DltConnection *connections1 = nullptr;
    DltReceiver receiver;

    memset(&daemon_local, 0, sizeof(DltDaemonLocal));
    memset(&ev1, 0, sizeof(DltEventHandler));
    memset(&receiver, 0, sizeof(DltReceiver));

    EXPECT_EQ(DLT_RETURN_OK, dlt_daemon_prepare_event_handling(&ev1));

    connections1 = (DltConnection *)malloc(sizeof(DltConnection));
    memset(connections1, 0, sizeof(DltConnection));
    connections1->next = 0;
    connections1->type = DLT_CONNECTION_GATEWAY;
    connections1->receiver = &receiver;


    ret = dlt_event_handler_register_connection(&ev1,
                                                &daemon_local,
                                                connections1,
                                                mask);

    EXPECT_EQ(DLT_RETURN_OK, ret);
    EXPECT_EQ(DLT_CONNECTION_GATEWAY, ev1.connections->type);

    free(ev1.pfd);
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
    DltDaemonLocal daemon_local;
    int mask = 0;
    DltEventHandler ev1;
    DltConnection *connections1 = nullptr;
    DltReceiver receiver;

    memset(&daemon_local, 0, sizeof(DltDaemonLocal));
    memset(&ev1, 0, sizeof(DltEventHandler));
    memset(&receiver, 0, sizeof(DltReceiver));

    EXPECT_EQ(DLT_RETURN_OK, dlt_daemon_prepare_event_handling(&ev1));

    connections1 = (DltConnection *)malloc(sizeof(DltConnection));
    memset(connections1, 0, sizeof(DltConnection));

    connections1->next = 0;
    connections1->type = DLT_CONNECTION_GATEWAY;
    connections1->receiver = &receiver;
    receiver.fd = 100;

    ret = dlt_event_handler_register_connection(&ev1,
                                                &daemon_local,
                                                connections1,
                                                mask);
    EXPECT_EQ(DLT_RETURN_OK, ret);
    EXPECT_EQ(DLT_CONNECTION_GATEWAY, ev1.connections->type);

    ret = dlt_event_handler_unregister_connection(&ev1, &daemon_local, receiver.fd);
    EXPECT_EQ(DLT_RETURN_OK, ret);

    free(ev1.pfd);
}

/* Begin Method: dlt_daemon_connections::dlt_connection_create*/
TEST(t_dlt_connection_create, normal)
{
    int fd = 100;
    int ret = 0;
    DltDaemonLocal daemon_local;

    memset(&daemon_local, 0, sizeof(DltDaemonLocal));

    EXPECT_EQ(DLT_RETURN_OK,
              dlt_daemon_prepare_event_handling(&daemon_local.pEvent));

    ret = dlt_connection_create(&daemon_local,
                                &daemon_local.pEvent,
                                fd,
                                POLLIN,
                                DLT_CONNECTION_CLIENT_MSG_SERIAL);
    EXPECT_EQ(DLT_RETURN_OK, ret);

    dlt_event_handler_unregister_connection(&daemon_local.pEvent,
                                            &daemon_local,
                                            fd);

    free(daemon_local.pEvent.pfd);
}

/* Begin Method: dlt_daemon_connections::dlt_connection_destroy*/
TEST(t_dlt_connection_destroy, normal)
{
    DltConnection *to_destroy = (DltConnection *)malloc(sizeof(DltConnection));

    memset(to_destroy, 0, sizeof(DltConnection));
    to_destroy->next = 0;
    to_destroy->type = DLT_CONNECTION_CLIENT_MSG_SERIAL;

    to_destroy->receiver = (DltReceiver *)malloc(sizeof(DltReceiver));
    memset(to_destroy->receiver, 0, sizeof(DltReceiver));

    to_destroy->receiver->fd = -1;
    to_destroy->receiver->buffer = nullptr;

    dlt_connection_destroy(to_destroy);
}

/* Begin Method: dlt_daemon_connections::t_dlt_connection_destroy_receiver*/
TEST(t_dlt_connection_destroy_receiver, normal)
{
    DltConnection to_destroy;

    memset(&to_destroy, 0, sizeof(DltConnection));

    to_destroy.receiver = (DltReceiver *)malloc(sizeof(DltReceiver));
    memset(to_destroy.receiver, 0, sizeof(DltReceiver));

    to_destroy.receiver->fd = -1;
    to_destroy.receiver->buffer = nullptr;

    dlt_connection_destroy_receiver(&to_destroy);
}

/* Begin Method: dlt_daemon_connections::t_dlt_connection_get_receiver*/
TEST(t_dlt_connection_get_receiver, normal)
{
    int fd = 10;
    DltReceiver *ret;
    DltDaemonLocal daemon_local;

    memset(&daemon_local, 0, sizeof(DltDaemonLocal));

    ret = dlt_connection_get_receiver(&daemon_local,
                                      DLT_CONNECTION_CLIENT_MSG_TCP,
                                      fd);

    ASSERT_NE(ret, nullptr);
    EXPECT_EQ(fd, ret->fd);
}

/* Begin Method: dlt_daemon_connections::(t_dlt_connection_get_next*/
TEST(t_dlt_connection_get_next, normal)
{
    int type_mask =
        (DLT_CON_MASK_CLIENT_MSG_TCP | DLT_CON_MASK_CLIENT_MSG_SERIAL);
    DltConnection *ret = nullptr;
    DltConnection node;
    DltConnection current;

    memset(&node, 0, sizeof(DltConnection));
    memset(&current, 0, sizeof(DltConnection));

    node.type = DLT_CONNECTION_CLIENT_MSG_TCP;
    current.type = DLT_CONNECTION_ONE_S_TIMER;
    current.next = &node;

    ret = dlt_connection_get_next(&current, type_mask);
    ASSERT_NE(ret, nullptr);
    EXPECT_EQ(DLT_CONNECTION_CLIENT_MSG_TCP, ret->type);
    EXPECT_NE(DLT_CONNECTION_ONE_S_TIMER, ret->type);
}

/* Begin Method: dlt_daemon_connections::(t_dlt_connection_get_next*/
TEST(t_dlt_connection_get_next, abnormal)
{
    int type_mask =
        (DLT_CON_MASK_CLIENT_MSG_TCP | DLT_CON_MASK_CLIENT_MSG_SERIAL);
    DltConnection *ret;
    DltConnection node;
    DltConnection current;

    memset(&node, 0, sizeof(DltConnection));
    memset(&current, 0, sizeof(DltConnection));

    node.type = DLT_CONNECTION_CLIENT_MSG_TCP;
    current.type = DLT_CONNECTION_CLIENT_MSG_SERIAL;
    current.next = &node;

    ret = dlt_connection_get_next(&current, type_mask);
    ASSERT_NE(ret, nullptr);
    EXPECT_NE(DLT_CONNECTION_CLIENT_MSG_TCP, ret->type);
    EXPECT_EQ(DLT_CONNECTION_CLIENT_MSG_SERIAL, ret->type);
}

/* Begin Method: dlt_daemon_connections::t_dlt_connection_send*/
TEST(t_dlt_connection_send, normal_1)
{
    int ret = 0;
    DltConnection conn;
    DltReceiver receiver;
    void *data1 = nullptr;
    int size1 = 0;
    DltDaemonLocal daemon_local;

    memset(&conn, 0, sizeof(DltConnection));
    memset(&receiver, 0, sizeof(DltReceiver));
    memset(&daemon_local, 0, sizeof(DltDaemonLocal));

    receiver.fd = connectServer();
    EXPECT_NE(-1, receiver.fd);
    conn.receiver = &receiver;
    conn.type = DLT_CONNECTION_CLIENT_MSG_TCP;

    daemon_local.msg.databuffer = (uint8_t *)malloc(sizeof(uint8_t));

    if (daemon_local.msg.databuffer == NULL)
        close(receiver.fd);

    EXPECT_NE((uint8_t *)NULL, daemon_local.msg.databuffer);

    memset(daemon_local.msg.databuffer, 1, sizeof(uint8_t));
    daemon_local.msg.datasize = sizeof(uint8_t);

    data1 = daemon_local.msg.databuffer;
    size1 = daemon_local.msg.datasize;

    ret = dlt_connection_send(&conn, data1, size1);

    EXPECT_EQ(DLT_RETURN_OK, ret);

    close(receiver.fd);
    free(daemon_local.msg.databuffer);
}

TEST(t_dlt_connection_send, normal_2)
{
    int ret = 0;
    DltConnection conn;
    DltReceiver receiver;

    memset(&conn, 0, sizeof(DltConnection));
    memset(&receiver, 0, sizeof(DltReceiver));

    receiver.fd = 1;
    conn.receiver = &receiver;
    conn.type = DLT_CONNECTION_CLIENT_MSG_SERIAL;

    ret = dlt_connection_send(&conn,
                              (void *)dltSerialHeader,
                              sizeof(dltSerialHeader));

    EXPECT_EQ(DLT_RETURN_OK, ret);
}

TEST(t_dlt_connection_send, abnormal)
{
    int ret = 0;
    DltConnection conn;
    DltReceiver receiver;

    memset(&conn, 0, sizeof(DltConnection));
    memset(&receiver, 0, sizeof(DltReceiver));

    receiver.fd = -1;
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
    void *data1 = nullptr;
    void *data2 = nullptr;
    int size1 = 0;
    int size2 = 0;
    DltConnection conn;
    DltReceiver receiver;
    DltDaemonLocal daemon_local;

    memset(&conn, 0, sizeof(DltConnection));
    memset(&receiver, 0, sizeof(DltReceiver));
    memset(&daemon_local, 0, sizeof(DltDaemonLocal));

    data1 = daemon_local.msg.headerbuffer + sizeof(DltStorageHeader);
    size1 = daemon_local.msg.headersize - sizeof(DltStorageHeader);
    data2 = daemon_local.msg.databuffer;
    size2 = daemon_local.msg.datasize;

    receiver.fd = connectServer();
    EXPECT_NE(-1, receiver.fd);

    conn.receiver = &receiver;
    conn.type = DLT_CONNECTION_CLIENT_MSG_TCP;

    daemon_local.msg.headersize = sizeof(DltStorageHeader) +
        sizeof(DltStandardHeader) +
        sizeof(DltStandardHeaderExtra) +
        sizeof(DltExtendedHeader);

    memset(daemon_local.msg.headerbuffer, 0, daemon_local.msg.headersize);

    data1 = daemon_local.msg.headerbuffer + sizeof(DltStorageHeader);
    size1 = daemon_local.msg.headersize - sizeof(DltStorageHeader);

    daemon_local.msg.databuffer = (uint8_t *)malloc(sizeof(uint8_t));

    if (daemon_local.msg.databuffer == NULL)
        close(receiver.fd);

    EXPECT_NE((uint8_t *)NULL, daemon_local.msg.databuffer);

    memset(daemon_local.msg.databuffer, 0, sizeof(uint8_t));
    daemon_local.msg.datasize = sizeof(uint8_t);

    data2 = daemon_local.msg.databuffer;
    size2 = daemon_local.msg.datasize;

    ret = dlt_connection_send_multiple(&conn,
                                       data1,
                                       size1,
                                       data2,
                                       size2,
                                       1);

    EXPECT_EQ(DLT_RETURN_OK, ret);

    close(receiver.fd);
    free(daemon_local.msg.databuffer);
}

TEST(t_dlt_connection_send_multiple, normal_2)
{
    int ret = 0;
    void *data1 = nullptr;
    void *data2 = nullptr;
    int size1 = 0;
    int size2 = 0;
    DltConnection conn;
    DltReceiver receiver;
    DltDaemonLocal daemon_local;

    memset(&conn, 0, sizeof(DltConnection));
    memset(&receiver, 0, sizeof(DltReceiver));
    memset(&daemon_local, 0, sizeof(DltDaemonLocal));

    data1 = daemon_local.msg.headerbuffer + sizeof(DltStorageHeader);
    size1 = daemon_local.msg.headersize - sizeof(DltStorageHeader);
    data2 = daemon_local.msg.databuffer;
    size2 = daemon_local.msg.datasize;

    receiver.fd = connectServer();
    EXPECT_NE(-1, receiver.fd);

    conn.receiver = &receiver;
    conn.type = DLT_CONNECTION_CLIENT_MSG_TCP;

    daemon_local.msg.headersize = sizeof(DltStorageHeader) +
        sizeof(DltStandardHeader) +
        sizeof(DltStandardHeaderExtra) +
        sizeof(DltExtendedHeader);

    memset(daemon_local.msg.headerbuffer, 0, daemon_local.msg.headersize);

    data1 = daemon_local.msg.headerbuffer + sizeof(DltStorageHeader);
    size1 = daemon_local.msg.headersize - sizeof(DltStorageHeader);

    daemon_local.msg.databuffer = (uint8_t *)malloc(sizeof(uint8_t));

    if (daemon_local.msg.databuffer == NULL)
        close(receiver.fd);

    EXPECT_NE((uint8_t *)NULL, daemon_local.msg.databuffer);

    memset(daemon_local.msg.databuffer, 0, sizeof(uint8_t));
    daemon_local.msg.datasize = sizeof(uint8_t);

    data2 = daemon_local.msg.databuffer;
    size2 = daemon_local.msg.datasize;

    ret = dlt_connection_send_multiple(&conn,
                                       data1,
                                       size1,
                                       data2,
                                       size2,
                                       0);

    EXPECT_EQ(DLT_RETURN_OK, ret);

    close(receiver.fd);
    free(daemon_local.msg.databuffer);
}

TEST(t_dlt_connection_send_multiple, nullpointer)
{
    int ret = 0;
    void *data1 = nullptr;
    void *data2 = nullptr;
    int size1 = 0;
    int size2 = 0;
    DltDaemonLocal daemon_local;

    memset(&daemon_local, 0, sizeof(DltDaemonLocal));

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
    int sockfd = 0, portno = 0;
    struct sockaddr_in serv_addr;
    struct hostent *server = nullptr;

    portno = 8080;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server = gethostbyname("127.0.0.1");
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr,
           (char *)server->h_addr,
           server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Error: %s (%d) occured in connect socket\n",
               strerror(errno),
               errno);
        close(sockfd);
        return -1;
    }

    return sockfd;
}

#define GTEST_SOCKS_ACCEPTED 3

int main(int argc, char **argv)
{
    pid_t cpid = fork();

    if (cpid == -1) {
        printf("fork fail\n");
        return -1;
    }

    if (cpid) {
        int i = GTEST_SOCKS_ACCEPTED;
        int j = 0, optval = 1;
        char buffer[256] = {};
        int sockfd = 0, newsockfd[GTEST_SOCKS_ACCEPTED] = {}, portno = 0;
        socklen_t clilen = {};
        struct sockaddr_in serv_addr, cli_addr;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);

        if (sockfd == -1) {
            printf("Error in creating socket\n");
            return -1;
        }

        memset((char *) &serv_addr, 0, sizeof(serv_addr));
        memset((char *) &cli_addr, 0, sizeof(cli_addr));
        portno = 8080;

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);

        if (setsockopt(sockfd,
                       SOL_SOCKET,
                       SO_REUSEADDR,
                       &optval,
                       sizeof(optval)) == -1) {
            perror("setsockopt");
            close(sockfd);
            exit(1);
        }

        j = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

        if (j == -1) {
            perror("Bind Error\n");
            close(sockfd);
            return -1;
        }

        listen(sockfd, 5);

        while (i) {
            clilen = sizeof(cli_addr);
            newsockfd[i - 1] = accept(sockfd,
                                      (struct sockaddr *)&cli_addr,
                                      &clilen);

            if (newsockfd[i - 1] == -1) {
                printf("Error in accept");
                return -1;
            }

            memset(buffer, 0, 256);
            (void)(read(newsockfd[i - 1], buffer, 255) + 1); /* just ignore result */
            i--;
        }

        for (j = 0; j < GTEST_SOCKS_ACCEPTED; j++)
            close(newsockfd[i]);

        close(sockfd);
    }
    else {

        ::testing::InitGoogleTest(&argc, argv);
        ::testing::FLAGS_gtest_break_on_failure = false;
/*        ::testing::FLAGS_gtest_filter = "t_dlt_event_handler_register_connection*"; */
        return RUN_ALL_TESTS();
    }

    return 0;
}
