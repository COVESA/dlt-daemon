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
 * Stefan Held <stefan_held@mentor.com>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file gtest_dlt_common.cpp
 */

#include <stdio.h>
#include <gtest/gtest.h>

extern "C" {
#include "dlt_daemon_common.h"
#include "dlt_daemon_common_cfg.h"
#include "dlt_user_shared_cfg.h"
#include "errno.h"
#include <syslog.h>
#include "dlt_types.h"
#include "dlt-daemon.h"
#include "dlt-daemon_cfg.h"
#include "dlt_daemon_common_cfg.h"
#include "dlt_daemon_socket.h"
#include "dlt_daemon_serial.h"
#include "dlt_daemon_client.h"
#include "dlt_offline_trace.h"
#include "dlt_gateway_types.h"
}

#ifndef DLT_USER_DIR
#   define DLT_USER_DIR  "/tmp/dltpipes"
#endif

/* Name of named pipe to DLT daemon */
#ifndef DLT_USER_FIFO
#   define DLT_USER_FIFO "/tmp/dlt"
#endif

/* Begin Method:dlt_daemon_common::dlt_daemon_init_user_information */
TEST(t_dlt_daemon_init_user_information, normal_one_list)
{
    DltDaemon daemon;
    DltGateway gateway;
    char ecu[] = "ECU1";

    EXPECT_EQ(0, dlt_daemon_init(&daemon,
                                 DLT_DAEMON_RINGBUFFER_MIN_SIZE,
                                 DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                                 DLT_DAEMON_RINGBUFFER_STEP_SIZE,
                                 DLT_RUNTIME_DEFAULT_DIRECTORY,
                                 DLT_LOG_INFO, DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));

    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}

/* Begin Method:dlt_daemon_common::dlt_daemon_init_user_information */
TEST(t_dlt_daemon_init_user_information, normal_multiple_lists)
{
    DltDaemon daemon;
    DltGateway gateway;
    char ecu[] = "ECU1";
    char ecu2[] = "ECU2";
    char ecu3[] = "ECU3";

    gateway.connections =
        (DltGatewayConnection *)calloc(2, sizeof(DltGatewayConnection));
    gateway.connections[0].ecuid = &ecu2[0];
    gateway.connections[1].ecuid = &ecu3[0];
    gateway.num_connections = 2;

    /* Normal Use-Case */
    EXPECT_EQ(0, dlt_daemon_init(&daemon,
                                 DLT_DAEMON_RINGBUFFER_MIN_SIZE,
                                 DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                                 DLT_DAEMON_RINGBUFFER_STEP_SIZE,
                                 DLT_RUNTIME_DEFAULT_DIRECTORY,
                                 DLT_LOG_INFO, DLT_TRACE_STATUS_OFF, 0, 0));

    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 1, 0));
    EXPECT_EQ(3, daemon.num_user_lists);
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(gateway.connections[0].ecuid, daemon.user_list[1].ecu, DLT_ID_SIZE));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(gateway.connections[1].ecuid, daemon.user_list[2].ecu, DLT_ID_SIZE));

    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
    free(gateway.connections);
}

TEST(t_dlt_daemon_init_user_information, nullpointer)
{
    DltDaemon daemon;
    DltGateway gateway;

    EXPECT_EQ(-1, dlt_daemon_init_user_information(NULL, NULL, 0, 0));
    EXPECT_EQ(-1, dlt_daemon_init_user_information(NULL, &gateway, 0, 0));
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, NULL, 0, 0));
    EXPECT_EQ(-1, dlt_daemon_init_user_information(&daemon, NULL, 1, 0));
}

/* Begin Method:dlt_daemon_common::dlt_daemon_find_users_list */
TEST(t_dlt_daemon_find_users_list, normal_one_list)
{
    DltDaemon daemon;
    DltGateway gateway;
    DltDaemonRegisteredUsers *user_list;
    char ecu[] = "ECU1";

    EXPECT_EQ(0, dlt_daemon_init(&daemon,
                                 DLT_DAEMON_RINGBUFFER_MIN_SIZE,
                                 DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                                 DLT_DAEMON_RINGBUFFER_STEP_SIZE,
                                 DLT_RUNTIME_DEFAULT_DIRECTORY,
                                 DLT_LOG_INFO, DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));

    user_list = dlt_daemon_find_users_list(&daemon, &ecu[0], 0);
    EXPECT_NE(user_list, nullptr);
    EXPECT_EQ(DLT_RETURN_OK, strncmp(user_list->ecu, daemon.ecuid, DLT_ID_SIZE));

    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}

/* Begin Method:dlt_daemon_common::dlt_daemon_find_users_list */
TEST(t_dlt_daemon_find_users_list, abnormal)
{
    DltDaemon daemon;
    DltGateway gateway;
    DltDaemonRegisteredUsers *user_list;
    char ecu[] = "ECU1";
    char bla[] = "BLAH";

    EXPECT_EQ(0, dlt_daemon_init(&daemon,
                                 DLT_DAEMON_RINGBUFFER_MIN_SIZE,
                                 DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                                 DLT_DAEMON_RINGBUFFER_STEP_SIZE,
                                 DLT_RUNTIME_DEFAULT_DIRECTORY,
                                 DLT_LOG_INFO, DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));

    user_list = dlt_daemon_find_users_list(&daemon, bla, 0);
    EXPECT_EQ(user_list, nullptr);

    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}

/* Begin Method:dlt_daemon_common::dlt_daemon_init_user_information */
TEST(t_dlt_daemon_find_users_list, normal_multiple_lists)
{
    DltDaemon daemon;
    DltGateway gateway;
    char ecu[] = "ECU1";
    char ecu2[] = "ECU2";
    char ecu3[] = "ECU3";
    DltDaemonRegisteredUsers *user_list;

    gateway.connections =
        (DltGatewayConnection *)calloc(2, sizeof(DltGatewayConnection));
    gateway.connections[0].ecuid = &ecu2[0];
    gateway.connections[1].ecuid = &ecu3[0];
    gateway.num_connections = 2;

    /* Normal Use-Case */
    EXPECT_EQ(0, dlt_daemon_init(&daemon,
                                 DLT_DAEMON_RINGBUFFER_MIN_SIZE,
                                 DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                                 DLT_DAEMON_RINGBUFFER_STEP_SIZE,
                                 DLT_RUNTIME_DEFAULT_DIRECTORY,
                                 DLT_LOG_INFO, DLT_TRACE_STATUS_OFF, 0, 0));

    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 1, 0));
    EXPECT_EQ(3, daemon.num_user_lists);
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(gateway.connections[0].ecuid, daemon.user_list[1].ecu, DLT_ID_SIZE));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(gateway.connections[1].ecuid, daemon.user_list[2].ecu, DLT_ID_SIZE));

    user_list = dlt_daemon_find_users_list(&daemon, &ecu[0], 0);
    EXPECT_NE(user_list, nullptr);
    EXPECT_EQ(DLT_RETURN_OK, strncmp(user_list->ecu, daemon.ecuid, DLT_ID_SIZE));

    user_list = dlt_daemon_find_users_list(&daemon, &ecu2[0], 0);
    EXPECT_NE(user_list, nullptr);
    EXPECT_EQ(DLT_RETURN_OK, strncmp(user_list->ecu, gateway.connections[0].ecuid, DLT_ID_SIZE));

    user_list = dlt_daemon_find_users_list(&daemon, &ecu3[0], 0);
    EXPECT_NE(user_list, nullptr);
    EXPECT_EQ(DLT_RETURN_OK, strncmp(user_list->ecu, gateway.connections[1].ecuid, DLT_ID_SIZE));

    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
    free(gateway.connections);
}

TEST(t_dlt_daemon_find_users_list, nullpointer)
{
    DltDaemon daemon;
    char ecu[] = "ECU1";

    EXPECT_EQ(NULL, dlt_daemon_find_users_list(NULL, NULL, 0));
    EXPECT_EQ(NULL, dlt_daemon_find_users_list(&daemon, NULL, 0));
    EXPECT_EQ(NULL, dlt_daemon_find_users_list(NULL, &ecu[0], 0));
}

/* Begin Method:dlt_daemon_common::dlt_daemon_application_add */
TEST(t_dlt_daemon_application_add, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    const char *apid = "TEST";
    pid_t pid = 0;
    const char *desc = "HELLO_TEST";
    DltDaemonApplication *app = NULL;
    char ecu[] = "ECU1";
    int fd = 15;

    /* Normal Use-Case */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));

    app = dlt_daemon_application_add(&daemon, (char *)apid, pid, (char *)desc, fd, ecu, 0);
    /*printf("### APP: APID=%s  DESCR=%s NUMCONTEXT=%i PID=%i USERHANDLE=%i\n", app->apid,app->application_description, app->num_contexts, app->pid, app->user_handle); */
    EXPECT_STREQ(apid, app->apid);
    EXPECT_STREQ(desc, app->application_description);
    EXPECT_EQ(pid, app->pid);
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));

    /* Apid > 4, expected truncate to 4 char or error */
    apid = "TO_LONG";
    app = dlt_daemon_application_add(&daemon, (char *)apid, pid, (char *)desc, fd, ecu, 0);
    char tmp[5];
    strncpy(tmp, apid, 4);
    tmp[4] = '\0';
    EXPECT_STREQ(tmp, app->apid);
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
TEST(t_dlt_daemon_application_add, abnormal)
{
/*    DltDaemon daemon; */
/*    const char * apid = "TEST"; */
/*    pid_t pid = 0; */
/*    const char * desc = "HELLO_TEST"; */
/*    DltDaemonApplication *app = NULL; */

    /* Add the same application with same pid twice */
/*    EXPECT_EQ(0, dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE, DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY,DLT_LOG_INFO, DLT_TRACE_STATUS_OFF,0,0)); */
/*    app = dlt_daemon_application_add(&daemon,(char *) apid, pid, (char *) desc, 0); */
/*    EXPECT_LE((DltDaemonApplication *) 0, app); */
/*    app = dlt_daemon_application_add(&daemon,(char *) apid, pid, (char *) desc, 0); */
/*    EXPECT_EQ((DltDaemonApplication *) 0, app); */
/*    dlt_daemon_application_del(&daemon,app, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */

    /* Add the same applicaiotn with different pid */
/*    app = dlt_daemon_application_add(&daemon,(char *) apid, 0, (char *) desc, 0); */
/*    EXPECT_LE((DltDaemonApplication *) 0, app); */
/*    app = dlt_daemon_application_add(&daemon,(char *) apid, 123, (char *) desc, 0); */
/*    EXPECT_EQ((DltDaemonApplication *) 0, app); */
/*    dlt_daemon_application_del(&daemon,app, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */


    /* verbose value != 0 or 1 */
/*    app = dlt_daemon_application_add(&daemon,(char *) apid, pid, (char *) desc, 0); */
/*    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_add(&daemon,(char *) apid, pid, (char *) desc, 12345678)); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */

    /* Apid < 4, expected fill to 4 chars or error */
/*    apid = "SH"; */
/*    app = dlt_daemon_application_add(&daemon,(char *) apid, pid, (char *) desc, 0); */
/*    char tmp[5]; */
/*    strncpy(tmp, apid, 4); */
/*    tmp[4] = '\0'; */
/*    EXPECT_STREQ(tmp, app->apid); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */
/*    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0)); */

}
TEST(t_dlt_daemon_application_add, nullpointer)
{
    DltDaemon daemon;
    const char *apid = "TEST";
    const char *desc = "HELLO_TEST";
    int fd = 42;
    char ecu[] = "ECU1";

    /* NULL-Pointer test */
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_add(NULL, NULL, 0, NULL, 0, NULL, 0));
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_add(NULL, NULL, 0, (char *)desc, 0, NULL, 0));
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_add(NULL, (char *)apid, 0, NULL, 0, NULL, 0));
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_add(NULL, (char *)apid, 0, (char *)desc, 0, NULL, 0));
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_add(&daemon, NULL, 0, NULL, 0, NULL, 0));
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_add(&daemon, NULL, 0, (char *)desc, 0, NULL, 0));
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_add(NULL, NULL, 0, NULL, fd, NULL, 0));
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_add(NULL, NULL, 0, NULL, 0, ecu, 0));
}
/* End Method:dlt_daemon_common::dlt_daemon_application_add */




/* Begin Method: dlt_daemon_common::dlt_daemon_application_del */
TEST(t_dlt_daemon_application_del, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    const char *apid = "TEST";
    pid_t pid = 0;
    const char *desc = "HELLO_TEST";
    DltDaemonApplication *app = NULL;
    char ecu[] = "ECU1";
    int fd = 42;

    /* Normal Use-Case, retrun type cannot be tested, only apid and desc */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    app = dlt_daemon_application_add(&daemon, (char *)apid, pid, (char *)desc, fd, ecu, 0);
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}

TEST(t_dlt_daemon_application_del, abnormal)
{
/*    DltDaemon daemon; */
/*    const char * apid = "TEST"; */
/*    pid_t pid = 0; */
/*    const char * desc = "HELLO_TEST"; */
/*    DltDaemonApplication *app = NULL; */

    /* no application exists, expect < 0 */
/*    EXPECT_EQ(0, dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE, DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY,DLT_LOG_INFO, DLT_TRACE_STATUS_OFF,0,0)); */
/*    EXPECT_GE(-1, dlt_daemon_application_del(&daemon, app, 0)); */

    /* Call delete two times */
/*    app = dlt_daemon_application_add(&daemon,(char *) apid, pid, (char *) desc, 0); */
/*    EXPECT_LE(0, dlt_daemon_application_del(&daemon,app, 0)); */
/*    EXPECT_GE(-1, dlt_daemon_application_del(&daemon,app, 0)); */
/*    dlt_daemon_applications_clear(&daemon, 0); */


    /* Verbose parameter != 0 or 1 */
/*    app = dlt_daemon_application_add(&daemon,(char *) apid, pid, (char *) desc, 0); */
/*    EXPECT_GE(-1, dlt_daemon_application_del(&daemon,app, 123456789)); */
/*    dlt_daemon_applications_clear(&daemon, 0); */
/*    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0)); */

}
TEST(t_dlt_daemon_application_del, nullpointer)
{
    DltDaemon daemon;
    DltDaemonApplication app;
    char ecu[] = "ECU1";

    /* NULL-Pointer */
    EXPECT_GE(-1, dlt_daemon_application_del(NULL, NULL, NULL, 0));
    EXPECT_GE(-1, dlt_daemon_application_del(NULL, &app, NULL, 0));
    EXPECT_GE(-1, dlt_daemon_application_del(&daemon, NULL, ecu, 0));
}
/* End Method: dlt_daemon_common::dlt_daemon_application_del */




/* Begin Method: dlt_daemon_common::dlt_daemon_applikation_find */
TEST(t_dlt_daemon_application_find, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    const char *apid = "TEST";
    pid_t pid = 0;
    const char *desc = "HELLO_TEST";
    DltDaemonApplication *app = NULL;
    char ecu[] = "ECU1";
    int fd = 42;

    /* Normal Use-Case */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    app = dlt_daemon_application_add(&daemon, (char *)apid, pid, (char *)desc, fd, ecu, 0);
    EXPECT_STREQ(apid, app->apid);
    EXPECT_STREQ(desc, app->application_description);
    EXPECT_EQ(pid, app->pid);
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));

    /* Application doesn't exist, expect NULL */
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_find(&daemon, ecu, (char *)apid, 0));

    /* Use a different apid, expect NULL */
    app = dlt_daemon_application_add(&daemon, (char *)apid, pid, (char *)desc, fd, ecu, 0);
    EXPECT_LE((DltDaemonApplication *)0, dlt_daemon_application_find(&daemon, ecu, (char *)apid, 0));
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_find(&daemon, ecu, (char *)"NEXI", 0));
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
TEST(t_dlt_daemon_application_find, abnormal)
{
/*    DltDaemon daemon; */
/*    const char * apid = "TEST"; */
/*    pid_t pid = 0; */
/*    const char * desc = "HELLO_TEST"; */
/*    DltDaemonApplication *app = NULL; */

    /* Verbose != 0 or 1, expect error */
/*    EXPECT_EQ(0, dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE, DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY,DLT_LOG_INFO, DLT_TRACE_STATUS_OFF,0,0)); */
/*    app = dlt_daemon_application_add(&daemon,(char *) apid, pid, (char *) desc, 0); */
/*    dlt_daemon_application_find(&daemon, (char *) apid, 0); */
/*    EXPECT_EQ((DltDaemonApplication *) 0, dlt_daemon_application_find(&daemon, (char *) apid, 123456789)); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */
/*    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0)); */

}
TEST(t_dlt_daemon_application_find, nullpointer)
{
    DltDaemon daemon;
    const char *apid = "TEST";

    /* NULL-Pointer, expected NULL */
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_find(NULL, NULL, NULL, 0));
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_find(NULL, (char *)apid, NULL, 0));
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_find(&daemon, NULL, NULL, 0));
}
/* End Method: dlt_daemon_common::dlt_daemon_applikation_find */




/* Begin Method: dlt_daemon_common::dlt_daemon_applications_clear */
TEST(t_dlt_daemon_applications_clear, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    pid_t pid = 0;
    char ecu[] = "ECU1";
    int fd = 42;

    /* Normal Use Case, expect >= 0 */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    EXPECT_LE((DltDaemonApplication *)0,
              dlt_daemon_application_add(&daemon, (char *)"TES1", pid, (char *)"Test clear 1", fd, ecu, 0));
    dlt_daemon_application_add(&daemon, (char *)"TES2", pid, (char *)"Test clear 2", fd, ecu, 0);
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
TEST(t_dlt_daemon_applications_clear, abnormal)
{
/*    DltDaemon daemon; */
/*    pid_t pid = 0; */

    /* No applications added, expect < -1 */
/*    EXPECT_EQ(0, dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE, DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY,DLT_LOG_INFO, DLT_TRACE_STATUS_OFF,0,0)); */
/*    EXPECT_GE(-1, dlt_daemon_applications_clear(&daemon, 0)); */

    /* Verbose != 0 or 1, expect error */
/*    dlt_daemon_application_add(&daemon, (char *) "TEST", pid, (char *) "Test clear", 0); */
/*    EXPECT_GE(-1, dlt_daemon_applications_clear(&daemon, 123456789)); */
/*    dlt_daemon_applications_clear(&daemon, 0); */
/*    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0)); */
}
TEST(t_dlt_daemon_applications_clear, nullpointer)
{
    /* NULL-Pointer, expect < 0 */
    EXPECT_GE(-1, dlt_daemon_applications_clear(NULL, NULL, 0));
}
/* End Method: dlt_daemon_common::dlt_daemon_applications_clear */



/* Begin Method: dlt_daemon_common::dlt_daemon_applications_invalidate_fd */
TEST(t_dlt_daemon_applications_invalidate_fd, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    const char *apid = "TEST";
    pid_t pid = 0;
    const char *desc = "HELLO_TEST";
    DltDaemonApplication *app = NULL;
    char ecu[] = "ECU1";
    int fd = 42;

    /* Normal Use-Case */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    app = dlt_daemon_application_add(&daemon, (char *)apid, pid, (char *)desc, fd, ecu, 0);
    EXPECT_LE(0, dlt_daemon_applications_invalidate_fd(&daemon, ecu, app->user_handle, 0));
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
TEST(t_dlt_daemon_applications_invalidate_fd, abnormal)
{
/*    DltDaemon daemon; */
/*    const char * apid = "TEST"; */
/*    pid_t pid = 0; */
/*    const char * desc = "HELLO_TEST"; */
/*    DltDaemonApplication *app = NULL; */

    /* Daemon isn't initialized, expected error */
/*    EXPECT_EQ(0, dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE, DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY,DLT_LOG_INFO, DLT_TRACE_STATUS_OFF,0,0)); */
/*    EXPECT_GE(-1, dlt_daemon_applications_invalidate_fd(&daemon, 0, 0)); */

    /* Verbose != 0 or 1, expect error */
/*    app = dlt_daemon_application_add(&daemon,(char *) apid, pid, (char *) desc, 0); */
/*    EXPECT_GE(-1, dlt_daemon_applications_invalidate_fd(&daemon, app->user_handle, 123456789)); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */
/*    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0)); */
}
TEST(t_dlt_daemon_applications_invalidate_fd, nullpointer)
{
    /* NULL-Pointer */
    EXPECT_GE(-1, dlt_daemon_applications_invalidate_fd(NULL, NULL, 0, 0));
}
/* End Method: dlt_daemon_common::dlt_daemon_applications_invalidate_fd */




/* Begin Method: dlt_daemon_common::dlt_daemon_applications_save */
TEST(t_dlt_daemon_applications_save, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    const char *apid = "TEST";
    pid_t pid = 0;
    const char *desc = "HELLO_TEST";
    DltDaemonApplication *app = NULL;
    const char *filename = "/tmp/dlt-runtime.cfg";
    char ecu[] = "ECU1";
    int fd = 42;

    /* Normal Use-Case */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    app = dlt_daemon_application_add(&daemon, (char *)apid, pid, (char *)desc, fd, ecu, 0);
    EXPECT_LE(0, dlt_daemon_applications_save(&daemon, (char *)filename, 0));
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
TEST(t_dlt_daemon_applications_save, abnormal)
{
/*    DltDaemon daemon; */
/*    const char * apid = "TEST"; */
/*    pid_t pid = 0; */
/*    const char * desc = "HELLO_TEST"; */
/*    DltDaemonApplication *app = NULL; */
/*    const char * filename = "/tmp/dlt-runtime.cfg"; */

    /* Uninitialized */
/*    EXPECT_EQ(0, dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE, DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY,DLT_LOG_INFO, DLT_TRACE_STATUS_OFF,0,0)); */
/*    EXPECT_GE(-1, dlt_daemon_applications_save(&daemon, (char *) filename, 0)); */

    /* Verbose != 1 or 0, expect error */
/*    app = dlt_daemon_application_add(&daemon,(char *) apid, pid, (char *) desc, 0); */
/*    EXPECT_GE(-1, dlt_daemon_applications_save(&daemon, (char *) filename, 123456789)); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */

    /* Wrong path filename */
/*    app = dlt_daemon_application_add(&daemon,(char *) apid, pid, (char *) desc, 0); */
/*    EXPECT_GE(-1, dlt_daemon_applications_save(&daemon, (char *) "PATH_DONT_EXIST", 0)); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */
/*    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0)); */
}
TEST(t_dlt_daemon_applications_save, nullpointer)
{
    DltDaemon daemon;
    const char *filename = "/tmp/dlt-runtime.cfg";

    /* NULL-Pointer */
    EXPECT_GE(-1, dlt_daemon_applications_save(NULL, NULL, 0));
    EXPECT_GE(-1, dlt_daemon_applications_save(NULL, (char *)filename, 0));
    EXPECT_GE(-1, dlt_daemon_applications_save(&daemon, NULL, 0));
}
/* End Method: dlt_daemon_common::dlt_daemon_applications_save */




/* Begin Method: dlt_daemon_common::dlt_daemon_applications_load */
TEST(t_dlt_daemon_applications_load, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    char ecu[] = "ECU1";
    const char *filename = "/tmp/dlt-runtime.cfg";

    /* Normal Use-Case, first execute t_dlt_daemon_applications_save !! */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    EXPECT_LE(0, dlt_daemon_applications_load(&daemon, (char *)filename, 0));
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
TEST(t_dlt_daemon_applications_load, abnormal)
{
/*    DltDaemon daemon; */
/*    const char * apid = "TEST"; */
/*    pid_t pid = 0; */
/*    const char * desc = "HELLO_TEST"; */
/*    DltDaemonApplication *app = NULL; */
/*    const char * filename = "/tmp/dlt-runtime.cfg"; */

    /* Uninitialized */
/*    EXPECT_EQ(0, dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE, DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY,DLT_LOG_INFO, DLT_TRACE_STATUS_OFF,0,0)); */
/*    EXPECT_GE(-1, dlt_daemon_applications_load(&daemon, (char *) filename, 0)); */

    /* Verbose != 1 or 0, expect error */
/*    app = dlt_daemon_application_add(&daemon,(char *) apid, pid, (char *) desc, 0); */
/*    EXPECT_GE(-1, dlt_daemon_applications_load(&daemon, (char *) filename, 123456789)); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */

    /* Wrong path filename */
/*    app = dlt_daemon_application_add(&daemon,(char *) apid, pid, (char *) desc, 0); */
/*    EXPECT_GE(-1, dlt_daemon_applications_load(&daemon, (char *) "PATH_DONT_EXIST", 0)); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */
/*    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0)); */
}
TEST(t_dlt_daemon_applications_load, nullpointer)
{
    DltDaemon daemon;
    const char *filename = "/tmp/dlt-runtime.cfg";

    /* NULL-Pointer */
    EXPECT_GE(-1, dlt_daemon_applications_load(NULL, NULL, 0));
    EXPECT_GE(-1, dlt_daemon_applications_load(NULL, (char *)filename, 0));
    EXPECT_GE(-1, dlt_daemon_applications_load(&daemon, NULL, 0));
}
/* End Method: dlt_daemon_common::dlt_daemon_applications_load */




/*##############################################################################################################################*/
/*##############################################################################################################################*/
/*##############################################################################################################################*/




/* Begin Method: dlt_daemon_common::dlt_daemon_context_add */
TEST(t_dlt_daemon_context_add, normal)
{
/*  Log Level */
/*  DLT_LOG_DEFAULT =             -1,   / **< Default log level * / */
/*  DLT_LOG_OFF     =           0x00,   / **< Log level off * / */
/*  DLT_LOG_FATAL   =           0x01,   / **< fatal system error * / */
/*  DLT_LOG_ERROR   =           0x02,   / **< error with impact to correct functionality * / */
/*  DLT_LOG_WARN    =           0x03,   / **< warning, correct behaviour could not be ensured * / */
/*  DLT_LOG_INFO    =           0x04,   / **< informational * / */
/*  DLT_LOG_DEBUG   =           0x05,   / **< debug  * / */
/*  DLT_LOG_VERBOSE =           0x06    / **< highest grade of information * / */

/*  Trace Status */
/*  DLT_TRACE_STATUS_DEFAULT =   -1,    / **< Default trace status * / */
/*  DLT_TRACE_STATUS_OFF     = 0x00,    / **< Trace status: Off * / */
/*  DLT_TRACE_STATUS_ON      = 0x01     / **< Trace status: On * / */

    DltDaemon daemon;
    DltGateway gateway;
    ID4 apid = "TES";
    ID4 ctid = "CON";
    char desc[255] = "TEST dlt_daemon_context_add";
    DltDaemonContext *daecontext = NULL;
    DltDaemonApplication *app = NULL;
    char ecu[] = "ECU1";
    int fd = 42;

    /* Normal Use-Case */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    app = dlt_daemon_application_add(&daemon, apid, 0, desc, fd, ecu, 0);
    daecontext = dlt_daemon_context_add(&daemon, apid, ctid, DLT_LOG_DEFAULT,
                                        DLT_TRACE_STATUS_DEFAULT, 0, 0, desc, ecu, 0);
    /*printf("### CONTEXT: APID=%s\tCTID=%s\n", daecontext->apid,daecontext->ctid); */
    EXPECT_STREQ(apid, daecontext->apid);
    EXPECT_STREQ(ctid, daecontext->ctid);
    EXPECT_STREQ(desc, daecontext->context_description);
    EXPECT_EQ(DLT_LOG_DEFAULT, daecontext->log_level);
    EXPECT_EQ(DLT_TRACE_STATUS_DEFAULT, daecontext->trace_status);
    EXPECT_LE(0, dlt_daemon_context_del(&daemon, daecontext, ecu, 0));
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_contexts_clear(&daemon, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
TEST(t_dlt_daemon_context_add, abnormal)
{
    DltDaemon daemon;
    DltGateway gateway;
    ID4 apid = "TES";
    ID4 ctid = "CON";
    char desc[255] = "TEST dlt_daemon_context_add";
    DltDaemonContext *daecontext = NULL;
    DltDaemonApplication *app = NULL;
    char ecu[] = "ECU1";
    int fd = 42;

    /* Log Level dont exists */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    DltLogLevelType DLT_LOG_NOT_EXIST = (DltLogLevelType) - 100;
    app = dlt_daemon_application_add(&daemon, apid, 0, desc, fd, ecu, 0);
    daecontext = dlt_daemon_context_add(&daemon,
                                        apid,
                                        ctid,
                                        DLT_LOG_NOT_EXIST,
                                        DLT_TRACE_STATUS_DEFAULT,
                                        0,
                                        0,
                                        desc,
                                        ecu,
                                        0);
    /*printf("### CONTEXT: APID=%s\tCTID=%s\n", daecontext->apid,daecontext->ctid); */
    EXPECT_EQ((DltDaemonContext *)0, daecontext);
    EXPECT_GE(-1, dlt_daemon_context_del(&daemon, daecontext, ecu, 0));
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_contexts_clear(&daemon, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));

    /* Trace Status dont exists */
    DltTraceStatusType DLT_TRACE_TYPE_NOT_EXIST = (DltTraceStatusType) - 100;
    app = dlt_daemon_application_add(&daemon, apid, 0, desc, fd, ecu, 0);
    daecontext = dlt_daemon_context_add(&daemon,
                                        apid,
                                        ctid,
                                        DLT_LOG_DEFAULT,
                                        DLT_TRACE_TYPE_NOT_EXIST,
                                        0,
                                        0,
                                        desc,
                                        ecu,
                                        0);
    /*printf("### CONTEXT: APID=%s\tCTID=%s\n", daecontext->apid,daecontext->ctid); */
    EXPECT_EQ((DltDaemonContext *)0, daecontext);
    EXPECT_GE(-1, dlt_daemon_context_del(&daemon, daecontext, ecu, 0));
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_contexts_clear(&daemon, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));

    /* Apid to long */
/*    char apid_tl[8] = "TO_LONG"; */
/*    app = dlt_daemon_application_add(&daemon, apid_tl, 0, desc, 0); */
/*    daecontext = dlt_daemon_context_add(&daemon,apid_tl,ctid,DLT_LOG_DEFAULT,DLT_TRACE_STATUS_DEFAULT,0,0,desc,0); */
/*    printf("### CONTEXT: APID=%s\tCTID=%s\n", daecontext->apid,daecontext->ctid); */
/*    EXPECT_STREQ(apid_tl, daecontext->apid); */
/*    EXPECT_STREQ(ctid, daecontext->ctid); */
/*    EXPECT_STREQ(desc, daecontext->context_description); */
/*    EXPECT_EQ(DLT_LOG_DEFAULT, daecontext->log_level); */
/*    EXPECT_EQ(DLT_TRACE_STATUS_DEFAULT, daecontext->trace_status); */
/*    dlt_daemon_context_del(&daemon, daecontext, 0); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_contexts_clear(&daemon, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */

    /* Apid to short */
/*    char apid_ts[3] = "TS"; */
/*    app = dlt_daemon_application_add(&daemon, apid_ts, 0, desc, 0); */
/*    daecontext = dlt_daemon_context_add(&daemon,apid_ts,ctid,DLT_LOG_DEFAULT,DLT_TRACE_STATUS_DEFAULT,0,0,desc,0); */
/*    //printf("### CONTEXT: APID=%s\tCTID=%s\n", daecontext->apid,daecontext->ctid); */
/*    EXPECT_STREQ(apid_ts, daecontext->apid); */
/*    EXPECT_STREQ(ctid, daecontext->ctid); */
/*    EXPECT_STREQ(desc, daecontext->context_description); */
/*    EXPECT_EQ(DLT_LOG_DEFAULT, daecontext->log_level); */
/*    EXPECT_EQ(DLT_TRACE_STATUS_DEFAULT, daecontext->trace_status); */
/*    //EXPECT_EQ(4, strlen(apid_ts)); */
/*    dlt_daemon_context_del(&daemon, daecontext, 0); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_contexts_clear(&daemon, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */

    /* Ctid to long */
/*    char ctid_tl[8] = "TO_LONG"; */
/*    app = dlt_daemon_application_add(&daemon, apid, 0, desc, 0); */
/*    daecontext = dlt_daemon_context_add(&daemon,apid,ctid_tl,DLT_LOG_DEFAULT,DLT_TRACE_STATUS_DEFAULT,0,0,desc,0); */
/*    //printf("### CONTEXT: APID=%s\tCTID=%s\n", daecontext->apid,daecontext->ctid); */
/*    EXPECT_STREQ(apid, daecontext->apid); */
/*    EXPECT_STREQ(ctid_tl, daecontext->ctid); */
/*    EXPECT_STREQ(desc, daecontext->context_description); */
/*    EXPECT_EQ(DLT_LOG_DEFAULT, daecontext->log_level); */
/*    EXPECT_EQ(DLT_TRACE_STATUS_DEFAULT, daecontext->trace_status); */
/*    dlt_daemon_context_del(&daemon, daecontext, 0); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_contexts_clear(&daemon, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */

    /* Ctid to short */
/*    char ctid_ts[4] = "TS"; */
/*    app = dlt_daemon_application_add(&daemon, apid, 0, desc, 0); */
/*    daecontext = dlt_daemon_context_add(&daemon,apid,ctid_ts,DLT_LOG_DEFAULT,DLT_TRACE_STATUS_DEFAULT,0,0,desc,0); */
/*    //printf("### CONTEXT: APID=%s\tCTID=%s\n", daecontext->apid,daecontext->ctid); */
/*    EXPECT_STREQ(apid, daecontext->apid); */
/*    EXPECT_STREQ(ctid_ts, daecontext->ctid); */
/*    EXPECT_STREQ(desc, daecontext->context_description); */
/*    EXPECT_EQ(DLT_LOG_DEFAULT, daecontext->log_level); */
/*    EXPECT_EQ(DLT_TRACE_STATUS_DEFAULT, daecontext->trace_status); */
/*    dlt_daemon_context_del(&daemon, daecontext, 0); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_contexts_clear(&daemon, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */

    /* Verbose != 0 or 1 */
/*    app = dlt_daemon_application_add(&daemon, apid, 0, desc, 0); */
/*    daecontext = dlt_daemon_context_add(&daemon,apid,ctid,DLT_LOG_DEFAULT,DLT_TRACE_STATUS_DEFAULT,0,0,desc,123456789); */
/*    //printf("### CONTEXT: APID=%s\tCTID=%s\n", daecontext->apid,daecontext->ctid); */
/*    EXPECT_EQ((DltDaemonContext *) 0, daecontext); */
/*    dlt_daemon_context_del(&daemon, daecontext, 0); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_contexts_clear(&daemon, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
TEST(t_dlt_daemon_context_add, nullpointer)
{
    DltDaemon daemon;
    DltGateway gateway;
    ID4 apid = "TES";
    ID4 ctid = "CON";
    char ecu[] = "ECU1";
    char desc[255] = "TEST dlt_daemon_context_add";

    /* NULL-Pointer */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_add(NULL, NULL, NULL, 0, 0, 0, 0, NULL, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_add(NULL, NULL, NULL, 0, 0, 0, 0, desc, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_add(NULL, NULL, ctid, 0, 0, 0, 0, NULL, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_add(NULL, NULL, ctid, 0, 0, 0, 0, desc, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_add(NULL, apid, NULL, 0, 0, 0, 0, NULL, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_add(NULL, apid, NULL, 0, 0, 0, 0, desc, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_add(NULL, apid, ctid, 0, 0, 0, 0, NULL, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_add(NULL, apid, ctid, 0, 0, 0, 0, desc, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_add(&daemon, NULL, NULL, 0, 0, 0, 0, NULL, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_add(&daemon, NULL, NULL, 0, 0, 0, 0, desc, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_add(&daemon, NULL, ctid, 0, 0, 0, 0, NULL, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_add(&daemon, NULL, ctid, 0, 0, 0, 0, desc, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_add(&daemon, apid, NULL, 0, 0, 0, 0, NULL, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_add(&daemon, apid, NULL, 0, 0, 0, 0, desc, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_add(&daemon, apid, ctid, 0, 0, 0, 0, NULL, NULL, 0));
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
/* End Method: dlt_daemon_common::dlt_daemon_context_add */



/* Begin Method: dlt_daemon_common::dlt_daemon_context_del */
TEST(t_dlt_daemon_context_del, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    ID4 apid = "TES";
    ID4 ctid = "CON";
    char desc[255] = "TEST dlt_daemon_context_add";
    DltDaemonContext *daecontext = NULL;
    DltDaemonApplication *app = NULL;
    char ecu[] = "ECU1";
    int fd = 42;

    /* Normal Use-Case */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    app = dlt_daemon_application_add(&daemon, apid, 0, desc, fd, ecu, 0);
    daecontext = dlt_daemon_context_add(&daemon,
                                        apid,
                                        ctid,
                                        DLT_LOG_DEFAULT,
                                        DLT_TRACE_STATUS_DEFAULT,
                                        0,
                                        0,
                                        desc,
                                        ecu,
                                        0);
    EXPECT_LE(0, dlt_daemon_context_del(&daemon, daecontext, ecu, 0));
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_contexts_clear(&daemon, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
TEST(t_dlt_daemon_context_del, abnormal)
{
/*    DltDaemon daemon; */
/*    ID4 apid = "TES"; */
/*    ID4 ctid = "CON"; */
/*    char desc[255] = "TEST dlt_daemon_context_add"; */
/*    DltDaemonContext *daecontext; */
/*    DltDaemonApplication *app; */

    /* Context uninitialized */
/*    EXPECT_EQ(0, dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE, DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY,DLT_LOG_INFO, DLT_TRACE_STATUS_OFF,0,0)); */
/*    EXPECT_GE(-1, dlt_daemon_context_del(&daemon, daecontext, 0)); */

    /* No application used */
/*    daecontext = dlt_daemon_context_add(&daemon,apid,ctid,DLT_LOG_DEFAULT,DLT_TRACE_STATUS_DEFAULT,0,0,desc,0); */
/*    EXPECT_GE(-1, dlt_daemon_context_del(&daemon, daecontext, 0)); */
/*    EXPECT_GE(-1, dlt_daemon_application_del(&daemon, app, 0)); */
/*    EXPECT_LE(0, dlt_daemon_contexts_clear(&daemon, 0)); */
/*    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, 0)); */

    /* No contex added */
/*    app = dlt_daemon_application_add(&daemon, apid, 0, desc, 0); */
/*    EXPECT_GE(-1, dlt_daemon_context_del(&daemon, daecontext, 0)); */
/*    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, 0)); */
/*    EXPECT_LE(0, dlt_daemon_contexts_clear(&daemon, 0)); */
/*    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, 0)); */

    /* Verbose != 0 or 1 */
/*    app = dlt_daemon_application_add(&daemon, apid, 0, desc, 0); */
/*    daecontext = dlt_daemon_context_add(&daemon,apid,ctid,DLT_LOG_DEFAULT,DLT_TRACE_STATUS_DEFAULT,0,0,desc,0); */
/*    EXPECT_GE(-1, dlt_daemon_context_del(&daemon, daecontext, 123456789)); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_contexts_clear(&daemon, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */
/*    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0)); */
}
TEST(t_dlt_daemon_context_del, nullpointer)
{
    DltDaemon daemon;
    DltDaemonContext daecontext;
    char ecu[] = "ECU1";

    /*NULL-Pointer */
    EXPECT_GE(-1, dlt_daemon_context_del(NULL, NULL, ecu, 0));
    EXPECT_GE(-1, dlt_daemon_context_del(NULL, &daecontext, NULL, 0));
    EXPECT_GE(-1, dlt_daemon_context_del(&daemon, NULL, NULL, 0));
}
/* End Method: dlt_daemon_common::dlt_daemon_context_del */




/* Begin Method: dlt_daemon_common::dlt_daemon_context_find */
TEST(t_dlt_daemon_context_find, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    ID4 apid = "TES";
    ID4 ctid = "CON";
    char desc[255] = "TEST dlt_daemon_context_add";
    DltDaemonContext *daecontext = NULL;
    DltDaemonApplication *app = NULL;
    char ecu[] = "ECU1";
    int fd = 42;

    /* Normal Use-Case */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    app = dlt_daemon_application_add(&daemon, apid, 0, desc, fd, ecu, 0);
    daecontext = dlt_daemon_context_add(&daemon,
                                        apid,
                                        ctid,
                                        DLT_LOG_DEFAULT,
                                        DLT_TRACE_STATUS_DEFAULT,
                                        0,
                                        0,
                                        desc,
                                        ecu,
                                        0);
    EXPECT_STREQ(apid, daecontext->apid);
    EXPECT_STREQ(ctid, daecontext->ctid);
    EXPECT_STREQ(desc, daecontext->context_description);
    EXPECT_EQ(DLT_LOG_DEFAULT, daecontext->log_level);
    EXPECT_EQ(DLT_TRACE_STATUS_DEFAULT, daecontext->trace_status);
    EXPECT_LE(0, dlt_daemon_context_del(&daemon, daecontext, ecu, 0));
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_contexts_clear(&daemon, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
TEST(t_dlt_daemon_context_find, abnormal)
{
    DltDaemon daemon;
    DltGateway gateway;
    ID4 apid = "TES";
    ID4 ctid = "CON";
    char desc[255] = "TEST dlt_daemon_context_add";
    DltDaemonContext *daecontext = NULL;
    DltDaemonApplication *app = NULL;
    char ecu[] = "ECU1";
    int fd = 42;

    /* Uninitialized */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_find(&daemon, apid, ctid, ecu, 0));

    /* No apid */
    char no_apid[1] = "";
    app = dlt_daemon_application_add(&daemon, no_apid, 0, desc, fd, ecu, 0);
    daecontext = dlt_daemon_context_add(&daemon,
                                        no_apid,
                                        ctid,
                                        DLT_LOG_DEFAULT,
                                        DLT_TRACE_STATUS_DEFAULT,
                                        0,
                                        0,
                                        desc,
                                        ecu,
                                        0);
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_find(&daemon, no_apid, ctid, ecu, 0));
    EXPECT_GE(-1, dlt_daemon_context_del(&daemon, daecontext, ecu, 0));
    EXPECT_GE(-1, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_contexts_clear(&daemon, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));

    /* No ctid */
    char no_ctid[1] = "";
    app = dlt_daemon_application_add(&daemon, apid, 0, desc, fd, ecu, 0);
    daecontext = dlt_daemon_context_add(&daemon,
                                        apid,
                                        no_ctid,
                                        DLT_LOG_DEFAULT,
                                        DLT_TRACE_STATUS_DEFAULT,
                                        0,
                                        0,
                                        desc,
                                        ecu,
                                        0);
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_find(&daemon, apid, no_ctid, ecu, 0));
    EXPECT_GE(-1, dlt_daemon_context_del(&daemon, daecontext, ecu, 0));
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_contexts_clear(&daemon, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));

    /* No application added */
    daecontext = dlt_daemon_context_add(&daemon,
                                        no_apid,
                                        ctid,
                                        DLT_LOG_DEFAULT,
                                        DLT_TRACE_STATUS_DEFAULT,
                                        0,
                                        0,
                                        desc,
                                        ecu,
                                        0);
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_find(&daemon, no_apid, ctid, ecu, 0));
    EXPECT_GE(-1, dlt_daemon_context_del(&daemon, daecontext, ecu, 0));
    EXPECT_LE(0, dlt_daemon_contexts_clear(&daemon, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));

    /* Verbose != 0 or 1 */
/*    app = dlt_daemon_application_add(&daemon, apid, 0, desc, 0); */
/*    daecontext = dlt_daemon_context_add(&daemon,apid,ctid,DLT_LOG_DEFAULT,DLT_TRACE_STATUS_DEFAULT,0,0,desc,0); */
/*    EXPECT_EQ((DltDaemonContext *) 0 ,dlt_daemon_context_find(&daemon, apid, ctid, 123456789)); */
/*    dlt_daemon_context_del(&daemon, daecontext, 0); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_contexts_clear(&daemon, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
TEST(t_dlt_daemon_context_find, nullpointer)
{
    DltDaemon daemon;
    ID4 apid = "TES";
    ID4 ctid = "CON";
    ID4 ecu = "ECU";

    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_find(NULL, NULL, NULL, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_find(NULL, NULL, ctid, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_find(NULL, apid, NULL, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_find(NULL, apid, ctid, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_find(&daemon, NULL, NULL, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_find(&daemon, NULL, ctid, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_find(&daemon, apid, NULL, NULL, 0));
    EXPECT_EQ((DltDaemonContext *)0, dlt_daemon_context_find(&daemon, NULL, NULL, ecu, 0));
}
/* End Method: dlt_daemon_common::dlt_daemon_context_find */




/* Begin Method: dlt_daemon_common::dlt_daemon_contexts_clear */
TEST(t_dlt_daemon_contexts_clear, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    ID4 apid = "TES";
    ID4 ctid = "CON";
    char desc[255] = "TEST dlt_daemon_context_add";
    DltDaemonContext *daecontext = NULL;
    DltDaemonApplication *app = NULL;
    char ecu[] = "ECU1";
    int fd = 42;

    /* Normal Use-Case */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    app = dlt_daemon_application_add(&daemon, apid, 0, desc, fd, ecu, 0);
    daecontext = dlt_daemon_context_add(&daemon,
                                        apid,
                                        ctid,
                                        DLT_LOG_DEFAULT,
                                        DLT_TRACE_STATUS_DEFAULT,
                                        0,
                                        0,
                                        desc,
                                        ecu,
                                        0);
    EXPECT_LE(0, dlt_daemon_context_del(&daemon, daecontext, ecu, 0));
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_contexts_clear(&daemon, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
TEST(t_dlt_daemon_contexts_clear, abnormal)
{
/*    DltDaemon daemon; */
/*    ID4 apid = "TES"; */
/*    ID4 ctid = "CON"; */
/*    char desc[255] = "TEST dlt_daemon_context_add"; */
/*    DltDaemonContext *daecontext = NULL; */
/*    DltDaemonApplication *app = NULL; */

    /* No context added */
/*    EXPECT_EQ(0, dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE, DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY,DLT_LOG_INFO, DLT_TRACE_STATUS_OFF,0,0)); */
/*    EXPECT_GE(-1, dlt_daemon_contexts_clear(&daemon, 0)); */

    /* Verbose != 0 or 1 */
/*    app = dlt_daemon_application_add(&daemon, apid, 0, desc, 0); */
/*    daecontext = dlt_daemon_context_add(&daemon,apid,ctid,DLT_LOG_DEFAULT,DLT_TRACE_STATUS_DEFAULT,0,0,desc,0); */
/*    dlt_daemon_context_del(&daemon, daecontext, 0); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    EXPECT_LE(0, dlt_daemon_contexts_clear(&daemon, 123456789)); */
/*    dlt_daemon_applications_clear(&daemon, 0); */
/*    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0)); */
}
TEST(t_dlt_daemon_contexts_clear, nullpointer)
{
    /* NULL-Pointer */
    EXPECT_GE(-1, dlt_daemon_contexts_clear(NULL, NULL, 0));
}
/* End Method: dlt_daemon_common::dlt_daemon_contexts_clear */




/* Begin Method: dlt_daemon_common::dlt_daemon_contexts_invalidate_fd */
TEST(t_dlt_daemon_contexts_invalidate_fd, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    ID4 apid = "TES";
    ID4 ctid = "CON";
    char desc[255] = "TEST dlt_daemon_context_add";
    DltDaemonContext *daecontext = NULL;
    DltDaemonApplication *app = NULL;
    char ecu[] = "ECU1";
    int fd = 42;

    /* Normal Use-Case */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    app = dlt_daemon_application_add(&daemon, apid, 0, desc, fd, ecu, 0);
    daecontext = dlt_daemon_context_add(&daemon,
                                        apid,
                                        ctid,
                                        DLT_LOG_DEFAULT,
                                        DLT_TRACE_STATUS_DEFAULT,
                                        0,
                                        0,
                                        desc,
                                        ecu,
                                        0);
    EXPECT_LE(0, dlt_daemon_contexts_invalidate_fd(&daemon, ecu, app->user_handle, 0));
    EXPECT_LE(0, dlt_daemon_context_del(&daemon, daecontext, ecu, 0));
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_contexts_clear(&daemon, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
TEST(t_dlt_daemon_contexts_invalidate_fd, abnormal)
{
/*    DltDaemon daemon; */
/*    ID4 apid = "TES"; */
/*    ID4 ctid = "CON"; */
/*    char desc[255] = "TEST dlt_daemon_context_add"; */
/*    DltDaemonContext *daecontext = NULL; */
/*    DltDaemonApplication *app = NULL; */

    /* Uninitialized */
/*    EXPECT_EQ(0, dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE, DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY,DLT_LOG_INFO, DLT_TRACE_STATUS_OFF,0,0)); */
/*    EXPECT_GE(-1, dlt_daemon_contexts_invalidate_fd(&daemon, app->user_handle, 0)); */

    /* Verbose != 0 or 1 */
/*    app = dlt_daemon_application_add(&daemon, apid, 0, desc, 0); */
/*    daecontext = dlt_daemon_context_add(&daemon,apid,ctid,DLT_LOG_DEFAULT,DLT_TRACE_STATUS_DEFAULT,0,0,desc,0); */
/*    EXPECT_GE(-1, dlt_daemon_contexts_invalidate_fd(&daemon, app->user_handle, 123456789)); */
/*    dlt_daemon_context_del(&daemon, daecontext, 0); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_contexts_clear(&daemon, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */
/*    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0)); */
}
TEST(t_dlt_daemon_contexts_invalidate_fd, nullpointer)
{
    /* NULL-Pointer */
    EXPECT_GE(-1, dlt_daemon_contexts_invalidate_fd(NULL, NULL, 0, 0));
}
/* End Method: dlt_daemon_common::dlt_daemon_contexts_invalidate_fd */




/* Begin Method: dlt_daemon_common::dlt_daemon_contexts_save */
TEST(t_dlt_daemon_contexts_save, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    ID4 apid = "TES";
    ID4 ctid = "CON";
    char desc[255] = "TEST dlt_daemon_context_add";
    DltDaemonContext *daecontext = NULL;
    DltDaemonApplication *app = NULL;
    const char *filename = "/tmp/dlt-runtime-context.cfg";
    char ecu[] = "ECU1";
    int fd = 42;

    /* Normal Use-Case */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    app = dlt_daemon_application_add(&daemon, apid, 0, desc, fd, ecu, 0);
    daecontext = dlt_daemon_context_add(&daemon,
                                        apid,
                                        ctid,
                                        DLT_LOG_DEFAULT,
                                        DLT_TRACE_STATUS_DEFAULT,
                                        0,
                                        0,
                                        desc,
                                        ecu,
                                        0);
    EXPECT_LE(0, dlt_daemon_contexts_save(&daemon, filename, 0));
    EXPECT_LE(0, dlt_daemon_context_del(&daemon, daecontext, ecu, 0));
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_contexts_clear(&daemon, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
TEST(t_dlt_daemon_contexts_save, abnormal)
{
/*    DltDaemon daemon; */
/*    ID4 apid = "TES"; */
/*    ID4 ctid = "CON"; */
/*    char desc[255] = "TEST dlt_daemon_context_add"; */
/*    DltDaemonContext *daecontext = NULL; */
/*    DltDaemonApplication *app = NULL; */
/*    const char * filename = "/tmp/dlt-runtime-context.cfg"; */

    /* Uninitialized */
/*    EXPECT_EQ(0, dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE, DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY,DLT_LOG_INFO, DLT_TRACE_STATUS_OFF,0,0)); */
/*    EXPECT_GE(-1, dlt_daemon_contexts_save(&daemon, filename, 0)); */

    /* Verbose != 1 or 0, expect error */
/*    app = dlt_daemon_application_add(&daemon, apid, 0, desc, 0); */
/*    daecontext = dlt_daemon_context_add(&daemon,apid,ctid,DLT_LOG_DEFAULT,DLT_TRACE_STATUS_DEFAULT,0,0,desc,0); */
/*    EXPECT_GE(-1, dlt_daemon_contexts_save(&daemon, filename, 123456789)); */
/*    dlt_daemon_context_del(&daemon, daecontext, 0); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_contexts_clear(&daemon, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */

    /* Wrong path filename */
/*    app = dlt_daemon_application_add(&daemon, apid, 0, desc, 0); */
/*    daecontext = dlt_daemon_context_add(&daemon,apid,ctid,DLT_LOG_DEFAULT,DLT_TRACE_STATUS_DEFAULT,0,0,desc,0); */
/*    EXPECT_GE(-1, dlt_daemon_contexts_save(&daemon, (char *) "PATCH_NOT_EXISTS", 0)); */
/*    dlt_daemon_context_del(&daemon, daecontext, 0); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_contexts_clear(&daemon, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */
/*    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0)); */
}
TEST(t_dlt_daemon_contexts_save, nullpointer)
{
    DltDaemon daemon;
    const char *filename = "/tmp/dlt-runtime-context.cfg";

    /* NULL-Pointer */
    EXPECT_GE(-1, dlt_daemon_contexts_save(NULL, NULL, 0));
    EXPECT_GE(-1, dlt_daemon_contexts_save(NULL, filename, 0));
    EXPECT_GE(-1, dlt_daemon_contexts_save(&daemon, NULL, 0));
}
/* End Method: dlt_daemon_common::dlt_daemon_contexts_save */




/* Begin Method: dlt_daemon_common::dlt_daemon_contexts_load */
TEST(t_dlt_daemon_contexts_load, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    ID4 apid = "TES";
    ID4 ctid = "CON";
    char desc[255] = "TEST dlt_daemon_context_add";
    DltDaemonContext *daecontext = NULL;
    DltDaemonApplication *app = NULL;
    const char *filename = "/tmp/dlt-runtime-context.cfg";
    char ecu[] = "ECU1";
    int fd = 42;

    /* Normal Use-Case */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    app = dlt_daemon_application_add(&daemon, apid, 0, desc, fd, ecu, 0);
    daecontext = dlt_daemon_context_add(&daemon,
                                        apid,
                                        ctid,
                                        DLT_LOG_DEFAULT,
                                        DLT_TRACE_STATUS_DEFAULT,
                                        0,
                                        0,
                                        desc,
                                        ecu,
                                        0);
    EXPECT_LE(0, dlt_daemon_contexts_load(&daemon, filename, 0));
    EXPECT_LE(0, dlt_daemon_context_del(&daemon, daecontext, ecu, 0));
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_contexts_clear(&daemon, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
TEST(t_dlt_daemon_contexts_load, abnormal)
{
/*    DltDaemon daemon; */
/*    ID4 apid = "TES"; */
/*    ID4 ctid = "CON"; */
/*    char desc[255] = "TEST dlt_daemon_context_add"; */
/*    DltDaemonContext *daecontext = NULL; */
/*    DltDaemonApplication *app = NULL; */
/*    const char * filename = "/tmp/dlt-runtime-context.cfg"; */

    /* Uninitialized */
/*    EXPECT_EQ(0, dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE, DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY,DLT_LOG_INFO, DLT_TRACE_STATUS_OFF,0,0)); */
/*    EXPECT_GE(-1, dlt_daemon_contexts_load(&daemon, filename, 0)); */

    /* Verbose != 1 or 0, expect error */
/*    app = dlt_daemon_application_add(&daemon, apid, 0, desc, 0); */
/*    daecontext = dlt_daemon_context_add(&daemon,apid,ctid,DLT_LOG_DEFAULT,DLT_TRACE_STATUS_DEFAULT,0,0,desc,0); */
/*    EXPECT_GE(-1, dlt_daemon_contexts_load(&daemon, filename, 123456789)); */
/*    dlt_daemon_context_del(&daemon, daecontext, 0); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_contexts_clear(&daemon, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */

    /* Wrong path filename */
/*    app = dlt_daemon_application_add(&daemon, apid, 0, desc, 0); */
/*    daecontext = dlt_daemon_context_add(&daemon,apid,ctid,DLT_LOG_DEFAULT,DLT_TRACE_STATUS_DEFAULT,0,0,desc,0); */
/*    EXPECT_GE(-1, dlt_daemon_contexts_load(&daemon, (char *) "PATCH_NOT_EXISTS", 0)); */
/*    dlt_daemon_context_del(&daemon, daecontext, 0); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_contexts_clear(&daemon, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */
/*    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0)); */
}
TEST(t_dlt_daemon_contexts_load, nullpointer)
{
    DltDaemon daemon;
    const char *filename = "/tmp/dlt-runtime-context.cfg";

    /* NULL-Pointer */
    EXPECT_GE(-1, dlt_daemon_contexts_load(NULL, NULL, 0));
    EXPECT_GE(-1, dlt_daemon_contexts_load(NULL, filename, 0));
    EXPECT_GE(-1, dlt_daemon_contexts_load(&daemon, NULL, 0));
}
/* End Method: dlt_daemon_common::dlt_daemon_contexts_load */





/*##############################################################################################################################*/
/*##############################################################################################################################*/
/*##############################################################################################################################*/




/* Begin Method: dlt_daemon_common::dlt_daemon_user_send_all_log_state */
/* Can't test this Method, maybe a return value would be a better solution */
TEST(t_dlt_daemon_user_send_all_log_state, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    char ecu[] = "ECU1";

    /* Normal Use-Case */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    EXPECT_NO_FATAL_FAILURE(dlt_daemon_user_send_all_log_state(&daemon, 0));
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
TEST(t_dlt_daemon_user_send_all_log_state, abnormal)
{}
TEST(t_dlt_daemon_user_send_all_log_state, nullpointer)
{
    EXPECT_NO_FATAL_FAILURE(dlt_daemon_user_send_all_log_state(NULL, 0));
}
/* End Method: dlt_daemon_common::dlt_daemon_user_send_all_log_state */




/* Begin Method: dlt_daemon_common::dlt_daemon_user_send_default_update */
/* Can't test this Method, maybe a return value would be a better solution */
TEST(t_dlt_daemon_user_send_default_update, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    char ecu[] = "ECU1";

    /* Normal Use-Case */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    EXPECT_NO_FATAL_FAILURE(dlt_daemon_user_send_default_update(&daemon, 0));
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
TEST(t_dlt_daemon_user_send_default_update, abnormal)
{}
TEST(t_dlt_daemon_user_send_default_update, nullpointer)
{
    EXPECT_NO_FATAL_FAILURE(dlt_daemon_user_send_default_update(NULL, 0));
}
/* End Method: dlt_daemon_common::dlt_daemon_user_send_default_update */




/* Begin Method: dlt_daemon_common::dlt_daemon_user_send_log_level */
TEST(t_dlt_daemon_user_send_log_level, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    ID4 apid = "TES";
    ID4 ctid = "CON";
    char desc[255] = "TEST dlt_daemon_context_add";
    DltDaemonContext *daecontext = NULL;
    DltDaemonApplication *app = NULL;
    char ecu[] = "ECU1";
    int fd = 42;

    /* Normal Use-Case */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    app = dlt_daemon_application_add(&daemon, apid, 0, desc, fd, ecu, 0);
    daecontext = dlt_daemon_context_add(&daemon,
                                        apid,
                                        ctid,
                                        DLT_LOG_DEFAULT,
                                        DLT_TRACE_STATUS_DEFAULT,
                                        0,
                                        1,
                                        desc,
                                        ecu,
                                        0);
    EXPECT_LE(0, dlt_daemon_user_send_log_level(&daemon, daecontext, 0));
    EXPECT_LE(0, dlt_daemon_context_del(&daemon, daecontext, ecu, 0));
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_contexts_clear(&daemon, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
TEST(t_dlt_daemon_user_send_log_level, abnormal)
{
/*    DltDaemon daemon; */
/*    ID4 apid = "TES"; */
/*    ID4 ctid = "CON"; */
/*    char desc[255] = "TEST dlt_daemon_context_add"; */
/*    DltDaemonContext *daecontext = NULL; */
/*    DltDaemonApplication *app = NULL; */

    /* Uninitialized */
/*    EXPECT_EQ(0, dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE, DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY,DLT_LOG_INFO, DLT_TRACE_STATUS_OFF,0,0)); */
/*    EXPECT_GE(-1, dlt_daemon_user_send_log_level(&daemon, daecontext, 0)); */

    /* File Handler <= 0 */
/*    app = dlt_daemon_application_add(&daemon, apid, 0, desc, 0); */
/*    daecontext = dlt_daemon_context_add(&daemon,apid,ctid,DLT_LOG_DEFAULT,DLT_TRACE_STATUS_DEFAULT,0,-1,desc,0); */
/*    EXPECT_GE(-1, dlt_daemon_user_send_log_level(&daemon, daecontext, 0)); */
/*    EXPECT_LE(0, dlt_daemon_context_del(&daemon, daecontext, 0)); */
/*    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, 0)); */
/*    EXPECT_LE(0, dlt_daemon_contexts_clear(&daemon, 0)); */
/*    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, 0)); */

    /* Verbose != 0 or 1 */
/*    app = dlt_daemon_application_add(&daemon, apid, 0, desc, 0); */
/*    daecontext = dlt_daemon_context_add(&daemon,apid,ctid,DLT_LOG_DEFAULT,DLT_TRACE_STATUS_DEFAULT,0,1,desc,0); */
/*    EXPECT_GE(-1, dlt_daemon_user_send_log_level(&daemon, daecontext, 123456789)); */
/*    dlt_daemon_context_del(&daemon, daecontext, 0); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_contexts_clear(&daemon, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */
/*    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0)); */
}
TEST(t_dlt_daemon_user_send_log_level, nullpointer)
{
    DltDaemon daemon;
    DltDaemonContext daecontext;

    /* NULL-Pointer */
    EXPECT_GE(-1, dlt_daemon_user_send_log_level(NULL, NULL, 0));
    EXPECT_GE(-1, dlt_daemon_user_send_log_level(NULL, &daecontext, 0));
    EXPECT_GE(-1, dlt_daemon_user_send_log_level(&daemon, NULL, 0));
}
/* End Method: dlt_daemon_common::dlt_daemon_user_send_log_level */




/* Begin Method: dlt_daemon_common::dlt_daemon_user_send_log_state */
TEST(t_dlt_daemon_user_send_log_state, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
/*    ID4 apid = "TES"; */
/*    ID4 ctid = "CON"; */
/*    char desc[255] = "TEST dlt_daemon_context_add"; */
/*    DltDaemonContext *daecontext; */
/*    DltDaemonApplication *app; */
    pid_t pid = 18166;
    char ecu[] = "ECU1";
    char filename[DLT_DAEMON_COMMON_TEXTBUFSIZE + 1];
    snprintf(filename, DLT_DAEMON_COMMON_TEXTBUFSIZE, "%s/dlt%d", DLT_USER_DIR, pid);

    /* Normal Use-Case */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
/*    open(filename, O_RDWR |O_NONBLOCK); */
/*    dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE, DLT_DAEMON_RINGBUFFER_STEP_SIZE, "",0); */
/*    app = dlt_daemon_application_add(&daemon, apid, pid, desc, 0); */
/*    //printf("### USERHANDLE=%i\n", app->user_handle); */
/*    daecontext = dlt_daemon_context_add(&daemon,apid,ctid,DLT_LOG_DEFAULT,DLT_TRACE_STATUS_DEFAULT,0,0,desc,0); */
/*    EXPECT_GE(0, dlt_daemon_user_send_log_state(&daemon, app, 0)); */
/*    EXPECT_LE(0, dlt_daemon_context_del(&daemon, daecontext, 0)); */
/*    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, 0)); */
/*    EXPECT_LE(0, dlt_daemon_contexts_clear(&daemon, 0)); */
/*    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, 0)); */
/*    EXPECT_LE(0, close(app->user_handle)); */
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
TEST(t_dlt_daemon_user_send_log_state, abnormal)
{
/*    DltDaemon daemon; */
/*    ID4 apid = "TES"; */
/*    ID4 ctid = "CON"; */
/*    char desc[255] = "TEST dlt_daemon_context_add"; */
/*    DltDaemonContext *daecontext = NULL; */
/*    DltDaemonApplication *app = NULL; */
/*    pid_t pid = 18166; */
/*    char filename[DLT_DAEMON_COMMON_TEXTBUFSIZE+1]; */
/*    snprintf(filename,DLT_DAEMON_COMMON_TEXTBUFSIZE,"%s/dlt%d",DLT_USER_DIR,pid); */

    /*Uninitialized */
/*    EXPECT_EQ(0, dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE, DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY,DLT_LOG_INFO, DLT_TRACE_STATUS_OFF,0,0)); */
/*    EXPECT_GE(-1, dlt_daemon_user_send_log_state(&daemon, app, 0)); */

    /* No Pipe open */
    /*open(filename, O_RDWR |O_NONBLOCK); */
/*    dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE, DLT_DAEMON_RINGBUFFER_STEP_SIZE, "",0); */
/*    app = dlt_daemon_application_add(&daemon, apid, pid, desc, 0); */
    /*printf("### USERHANDLE=%i\n", app->user_handle); */
/*    daecontext = dlt_daemon_context_add(&daemon,apid,ctid,DLT_LOG_DEFAULT,DLT_TRACE_STATUS_DEFAULT,0,0,desc,0); */
/*    EXPECT_GE(-1, dlt_daemon_user_send_log_state(&daemon, app, 0)); */
/*    EXPECT_LE(0, dlt_daemon_context_del(&daemon, daecontext, 0)); */
/*    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, 0)); */
/*    EXPECT_LE(0, dlt_daemon_contexts_clear(&daemon, 0)); */
/*    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, 0)); */
/*    EXPECT_LE(0, close(app->user_handle)); */

    /* Verbose != 1 or 0 */
/*    open(filename, O_RDWR |O_NONBLOCK); */
/*    dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE, DLT_DAEMON_RINGBUFFER_STEP_SIZE, "",0); */
/*    app = dlt_daemon_application_add(&daemon, apid, pid, desc, 0); */
/*    //printf("### USERHANDLE=%i\n", app->user_handle); */
/*    daecontext = dlt_daemon_context_add(&daemon,apid,ctid,DLT_LOG_DEFAULT,DLT_TRACE_STATUS_DEFAULT,0,0,desc,0); */
/*    EXPECT_GE(-1, dlt_daemon_user_send_log_state(&daemon, app, 123456789)); */
/*    dlt_daemon_context_del(&daemon, daecontext, 0); */
/*    dlt_daemon_application_del(&daemon, app, 0); */
/*    dlt_daemon_contexts_clear(&daemon, 0); */
/*    dlt_daemon_applications_clear(&daemon, 0); */
/*    close(app->user_handle); */
/*    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0)); */
}
TEST(t_dlt_daemon_user_send_log_state, nullpointer)
{
    DltDaemon daemon;
    DltDaemonApplication app;

    EXPECT_GE(0, dlt_daemon_user_send_log_state(NULL, NULL, 0));
    EXPECT_GE(0, dlt_daemon_user_send_log_state(NULL, &app, 0));
    EXPECT_GE(0, dlt_daemon_user_send_log_state(&daemon, NULL, 0));
}
/* End Method: dlt_daemon_common::dlt_daemon_user_send_log_state */

/*##############################################################################################################################*/
/*##############################################################################################################################*/
/*##############################################################################################################################*/




int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::FLAGS_gtest_break_on_failure = true;
    /*::testing::FLAGS_gtest_filter = "*.normal"; */
    /*::testing::FLAGS_gtest_repeat = 10000; */
    return RUN_ALL_TESTS();
}
