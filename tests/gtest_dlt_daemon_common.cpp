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

    char apid_buf[DLT_ID_SIZE+1] = {0};
    char desc_buf[32] = {0};
    memcpy(apid_buf, apid, DLT_ID_SIZE);
    apid_buf[DLT_ID_SIZE] = '\0';
    strncpy(desc_buf, desc, sizeof(desc_buf)-1);
    desc_buf[sizeof(desc_buf)-1] = '\0';
    app = dlt_daemon_application_add(&daemon, apid_buf, pid, desc_buf, fd, ecu, 0);
    /*printf("### APP: APID=%s  DESCR=%s NUMCONTEXT=%i PID=%i USERHANDLE=%i\n", app->apid,app->application_description, app->num_contexts, app->pid, app->user_handle); */
    EXPECT_EQ(0, strncmp(apid, app->apid, DLT_ID_SIZE));
    EXPECT_STREQ(desc, app->application_description);
    EXPECT_EQ(pid, app->pid);
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));

    /* Apid > 4, expected truncate to 4 char or error */
    apid = "TO_LONG";
    memcpy(apid_buf, apid, DLT_ID_SIZE);
    apid_buf[DLT_ID_SIZE] = '\0';
    strncpy(desc_buf, desc, sizeof(desc_buf)-1);
    desc_buf[sizeof(desc_buf)-1] = '\0';
    app = dlt_daemon_application_add(&daemon, apid_buf, pid, desc_buf, fd, ecu, 0);
    char tmp[5];
    memcpy(tmp, apid, 4);
    tmp[4] = '\0';
    EXPECT_EQ(0, strncmp(tmp, app->apid, DLT_ID_SIZE));
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
    char apid_buf[DLT_ID_SIZE+1] = {0};
    char desc_buf[32] = {0};
    /* NULL-Pointer test */
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_add(NULL, NULL, 0, NULL, 0, NULL, 0));
    strncpy(desc_buf, desc, sizeof(desc_buf)-1);
    desc_buf[sizeof(desc_buf)-1] = '\0';
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_add(NULL, NULL, 0, desc_buf, 0, NULL, 0));
    memcpy(apid_buf, apid, DLT_ID_SIZE);
    apid_buf[DLT_ID_SIZE] = '\0';
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_add(NULL, apid_buf, 0, NULL, 0, NULL, 0));
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_add(NULL, apid_buf, 0, desc_buf, 0, NULL, 0));
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_add(&daemon, NULL, 0, NULL, 0, NULL, 0));
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_add(&daemon, NULL, 0, desc_buf, 0, NULL, 0));
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
    char apid_buf[DLT_ID_SIZE+1] = {0};
    char desc_buf[32] = {0};
    memcpy(apid_buf, apid, DLT_ID_SIZE);
    apid_buf[DLT_ID_SIZE] = '\0';
    strncpy(desc_buf, desc, sizeof(desc_buf)-1);
    desc_buf[sizeof(desc_buf)-1] = '\0';
    app = dlt_daemon_application_add(&daemon, apid_buf, pid, desc_buf, fd, ecu, 0);
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
    char apid_buf[DLT_ID_SIZE+1] = {0};
    char desc_buf[32] = {0};
    memcpy(apid_buf, apid, DLT_ID_SIZE);
    apid_buf[DLT_ID_SIZE] = '\0';
    strncpy(desc_buf, desc, sizeof(desc_buf)-1);
    desc_buf[sizeof(desc_buf)-1] = '\0';
    app = dlt_daemon_application_add(&daemon, apid_buf, pid, desc_buf, fd, ecu, 0);
    EXPECT_EQ(0, strncmp(apid, app->apid, DLT_ID_SIZE));
    EXPECT_STREQ(desc, app->application_description);
    EXPECT_EQ(pid, app->pid);
    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));

    /* Application doesn't exist, expect NULL */
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_find(&daemon, ecu, apid_buf, 0));

    /* Use a different apid, expect NULL */
    memcpy(apid_buf, apid, DLT_ID_SIZE);
    apid_buf[DLT_ID_SIZE] = '\0';
    strncpy(desc_buf, desc, sizeof(desc_buf)-1);
    desc_buf[sizeof(desc_buf)-1] = '\0';
    app = dlt_daemon_application_add(&daemon, apid_buf, pid, desc_buf, fd, ecu, 0);
    EXPECT_LE((DltDaemonApplication *)0, dlt_daemon_application_find(&daemon, ecu, apid_buf, 0));
    char nexi[DLT_ID_SIZE+1] = {0};
    memcpy(nexi, "NEXI", DLT_ID_SIZE);
    nexi[DLT_ID_SIZE] = '\0';
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_find(&daemon, ecu, nexi, 0));
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
    char apid_buf[DLT_ID_SIZE+1] = {0};
    memcpy(apid_buf, apid, DLT_ID_SIZE);
    apid_buf[DLT_ID_SIZE] = '\0';
    EXPECT_EQ((DltDaemonApplication *)0, dlt_daemon_application_find(NULL, apid_buf, NULL, 0));
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
    char apid1_buf[DLT_ID_SIZE+1] = {0};
    char desc1_buf[32] = {0};
    memcpy(apid1_buf, "TES1", DLT_ID_SIZE);
    apid1_buf[DLT_ID_SIZE] = '\0';
    strncpy(desc1_buf, "Test clear 1", sizeof(desc1_buf)-1);
    desc1_buf[sizeof(desc1_buf)-1] = '\0';
    EXPECT_LE((DltDaemonApplication *)0,
              dlt_daemon_application_add(&daemon, apid1_buf, pid, desc1_buf, fd, ecu, 0));
    memcpy(apid1_buf, "TES1", DLT_ID_SIZE);
    apid1_buf[DLT_ID_SIZE] = '\0';
    strncpy(desc1_buf, "Test clear 1", sizeof(desc1_buf)-1);
    desc1_buf[sizeof(desc1_buf)-1] = '\0';
    dlt_daemon_application_add(&daemon, apid1_buf, pid, desc1_buf, fd, ecu, 0);
    char apid2_buf[DLT_ID_SIZE+1] = {0};
    char desc2_buf[32] = {0};
    memcpy(apid2_buf, "TES2", DLT_ID_SIZE);
    apid2_buf[DLT_ID_SIZE] = '\0';
    strncpy(desc2_buf, "Test clear 2", sizeof(desc2_buf)-1);
    desc2_buf[sizeof(desc2_buf)-1] = '\0';
    dlt_daemon_application_add(&daemon, apid2_buf, pid, desc2_buf, fd, ecu, 0);
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
    char apid_buf[DLT_ID_SIZE+1] = {0};
    char desc_buf[32] = {0};
    memcpy(apid_buf, apid, DLT_ID_SIZE);
    apid_buf[DLT_ID_SIZE] = '\0';
    strncpy(desc_buf, desc, sizeof(desc_buf)-1);
    desc_buf[sizeof(desc_buf)-1] = '\0';
    app = dlt_daemon_application_add(&daemon, apid_buf, pid, desc_buf, fd, ecu, 0);
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
    char apid_buf[DLT_ID_SIZE+1] = {0};
    char desc_buf[32] = {0};
    memcpy(apid_buf, apid, DLT_ID_SIZE);
    apid_buf[DLT_ID_SIZE] = '\0';
    strncpy(desc_buf, desc, sizeof(desc_buf)-1);
    desc_buf[sizeof(desc_buf)-1] = '\0';
    app = dlt_daemon_application_add(&daemon, apid_buf, pid, desc_buf, fd, ecu, 0);
    char filename_buf[256] = {0};
    strncpy(filename_buf, filename, sizeof(filename_buf)-1);
    filename_buf[sizeof(filename_buf)-1] = '\0';
    EXPECT_LE(0, dlt_daemon_applications_save(&daemon, filename_buf, 0));
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
    char filename_buf[256] = {0};
    strncpy(filename_buf, filename, sizeof(filename_buf)-1);
    filename_buf[sizeof(filename_buf)-1] = '\0';
    EXPECT_GE(-1, dlt_daemon_applications_save(NULL, filename_buf, 0));
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
    char filename_buf[256] = {0};
    strncpy(filename_buf, filename, sizeof(filename_buf)-1);
    filename_buf[sizeof(filename_buf)-1] = '\0';
    EXPECT_LE(0, dlt_daemon_applications_load(&daemon, filename_buf, 0));
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
    char filename_buf[256] = {0};
    strncpy(filename_buf, filename, sizeof(filename_buf)-1);
    filename_buf[sizeof(filename_buf)-1] = '\0';
    EXPECT_GE(-1, dlt_daemon_applications_load(NULL, filename_buf, 0));
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
    /* Log level out of valid range (-1 to 6), should return NULL */
    int8_t invalid_log_level = 100;
    app = dlt_daemon_application_add(&daemon, apid, 0, desc, fd, ecu, 0);
    daecontext = dlt_daemon_context_add(&daemon,
                                        apid,
                                        ctid,
                                        invalid_log_level,
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

    /* Trace Status out of valid range (-1 to 1), should return NULL */
    int8_t invalid_trace_status = 100;
    app = dlt_daemon_application_add(&daemon, apid, 0, desc, fd, ecu, 0);
    daecontext = dlt_daemon_context_add(&daemon,
                                        apid,
                                        ctid,
                                        DLT_LOG_DEFAULT,
                                        invalid_trace_status,
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


#ifdef DLT_TRACE_LOAD_CTRL_ENABLE
TEST(t_dlt_daemon_find_preconfigured_trace_load_settings, nullpointer)
{
    int num_settings;
    DltTraceLoadSettings *settings = NULL;
    DltDaemon daemon;

    EXPECT_EQ(dlt_daemon_find_preconfigured_trace_load_settings(
                  NULL, "APP1", "CTID", &settings, &num_settings, 0),
              DLT_RETURN_WRONG_PARAMETER);
    EXPECT_EQ(dlt_daemon_find_preconfigured_trace_load_settings(
                  &daemon, NULL, "CTID", &settings, &num_settings, 0),
              DLT_RETURN_WRONG_PARAMETER);
}

TEST(t_dlt_daemon_find_preconfigured_trace_load_settings, empty_trace_settings)
{
    int num_settings;
    DltTraceLoadSettings *settings = NULL;
    DltDaemon daemon;
    daemon.preconfigured_trace_load_settings = NULL;

    EXPECT_EQ(dlt_daemon_find_preconfigured_trace_load_settings(
                  &daemon, "APP1", NULL, &settings, &num_settings, 0),
              DLT_RETURN_OK);

    // test wrong trace load settings count
    daemon.preconfigured_trace_load_settings = (DltTraceLoadSettings *)malloc(
        sizeof(DltTraceLoadSettings));
    daemon.preconfigured_trace_load_settings_count = 0;

    EXPECT_EQ(dlt_daemon_find_preconfigured_trace_load_settings(
                  &daemon, "APP1", "CTID", &settings, &num_settings, 0),
              DLT_RETURN_OK);

    free(daemon.preconfigured_trace_load_settings);
}

TEST(t_dlt_daemon_find_preconfigured_trace_load_settings, app_id_not_found)
{
    int num_settings = 42;
    DltTraceLoadSettings *settings = (DltTraceLoadSettings *)42;
    DltDaemon daemon;

    // test wrong trace load settings count
    daemon.preconfigured_trace_load_settings = (DltTraceLoadSettings *)malloc(
        sizeof(DltTraceLoadSettings));
    memset(daemon.preconfigured_trace_load_settings, 0,
           sizeof(DltTraceLoadSettings));

    strncpy(daemon.preconfigured_trace_load_settings[0].apid, "APP2", DLT_ID_SIZE);
    daemon.preconfigured_trace_load_settings_count = 1;

    EXPECT_EQ(dlt_daemon_find_preconfigured_trace_load_settings(
                  &daemon, "APP1", NULL, &settings, &num_settings, 0),
              DLT_RETURN_OK);
    EXPECT_EQ(num_settings, 0);
    EXPECT_TRUE(settings == NULL);

    free(daemon.preconfigured_trace_load_settings);
}

static void setup_trace_load_settings(DltDaemon &daemon)
{
    daemon.preconfigured_trace_load_settings_count = 7;
    daemon.preconfigured_trace_load_settings = (DltTraceLoadSettings *)malloc(
        daemon.preconfigured_trace_load_settings_count * sizeof(DltTraceLoadSettings));
    memset(daemon.preconfigured_trace_load_settings, 0,
           daemon.preconfigured_trace_load_settings_count * sizeof(DltTraceLoadSettings));

    strncpy(daemon.preconfigured_trace_load_settings[0].apid, "APP2", DLT_ID_SIZE);
    strncpy(daemon.preconfigured_trace_load_settings[0].ctid, "CTID", DLT_ID_SIZE);

    strncpy(daemon.preconfigured_trace_load_settings[1].apid, "APP1", DLT_ID_SIZE);
    strncpy(daemon.preconfigured_trace_load_settings[1].ctid, "CTID", DLT_ID_SIZE);
    daemon.preconfigured_trace_load_settings[1].soft_limit = 21;
    daemon.preconfigured_trace_load_settings[1].hard_limit = 42;

    strncpy(daemon.preconfigured_trace_load_settings[2].apid, "APP1", DLT_ID_SIZE);
    strncpy(daemon.preconfigured_trace_load_settings[2].ctid, "CT02", DLT_ID_SIZE);
    daemon.preconfigured_trace_load_settings[2].soft_limit = 11;
    daemon.preconfigured_trace_load_settings[2].hard_limit = 22;

    strncpy(daemon.preconfigured_trace_load_settings[3].apid, "APP1", DLT_ID_SIZE);
    daemon.preconfigured_trace_load_settings[3].soft_limit = 44;
    daemon.preconfigured_trace_load_settings[3].hard_limit = 55;

    strncpy(daemon.preconfigured_trace_load_settings[4].apid, "APP3", DLT_ID_SIZE);
    strncpy(daemon.preconfigured_trace_load_settings[4].ctid, "CT03", DLT_ID_SIZE);
    daemon.preconfigured_trace_load_settings[4].soft_limit = 111;
    daemon.preconfigured_trace_load_settings[4].hard_limit = 222;

    strncpy(daemon.preconfigured_trace_load_settings[5].apid, "APP3", DLT_ID_SIZE);
    strncpy(daemon.preconfigured_trace_load_settings[5].ctid, "CT01", DLT_ID_SIZE);
    daemon.preconfigured_trace_load_settings[5].soft_limit = 333;
    daemon.preconfigured_trace_load_settings[5].hard_limit = 444;

    strncpy(daemon.preconfigured_trace_load_settings[6].apid, "APP1", DLT_ID_SIZE);
    strncpy(daemon.preconfigured_trace_load_settings[6].ctid, "CT03", DLT_ID_SIZE);
    daemon.preconfigured_trace_load_settings[6].soft_limit = 555;
    daemon.preconfigured_trace_load_settings[6].hard_limit = 666;

    qsort(daemon.preconfigured_trace_load_settings, daemon.preconfigured_trace_load_settings_count,
          sizeof(DltTraceLoadSettings), dlt_daemon_compare_trace_load_settings);
}

TEST(t_dlt_daemon_find_preconfigured_trace_load_settings, search_with_context)
{
    int num_settings = 42;
    DltTraceLoadSettings *settings = NULL;
    DltDaemon daemon;

    setup_trace_load_settings(daemon);

    EXPECT_EQ(dlt_daemon_find_preconfigured_trace_load_settings(
                  &daemon, "APP1", "CTID", &settings, &num_settings, 0),
              DLT_RETURN_OK);
    EXPECT_EQ(num_settings, 1);
    EXPECT_EQ(memcmp(settings, &daemon.preconfigured_trace_load_settings[3],
                     sizeof(DltTraceLoadSettings)),
              0);

    free(settings);
    free(daemon.preconfigured_trace_load_settings);
}

TEST(t_dlt_daemon_find_preconfigured_trace_load_settings, search_with_app_id)
{
    int num_settings = 42;
    DltTraceLoadSettings *settings = NULL;
    DltDaemon daemon;

    setup_trace_load_settings(daemon);

    EXPECT_EQ(dlt_daemon_find_preconfigured_trace_load_settings(
                  &daemon, "APP1", NULL, &settings, &num_settings, 0),
              DLT_RETURN_OK);
    EXPECT_EQ(num_settings, 4);
    for (int i = 0; i < num_settings; i++) {
        EXPECT_EQ(memcmp(&settings[i],
                         &daemon.preconfigured_trace_load_settings[i],
                         sizeof(DltTraceLoadSettings)),0);
    }

    free(settings);
    free(daemon.preconfigured_trace_load_settings);
}

TEST(t_dlt_daemon_application_add, trace_load_settings_not_found)
{
    DltDaemon daemon;
    DltGateway gateway;
    const char *apid_const = "TEST";
    pid_t pid = 0;
    const char *desc_const = "HELLO_TEST";
    DltDaemonApplication *app = NULL;
    char ecu[] = "ECU1";
    int fd = 15;

    EXPECT_EQ(0, dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE,
                                 DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                                 DLT_DAEMON_RINGBUFFER_STEP_SIZE,
                                 DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                                 DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK,
              strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    char apid[DLT_ID_SIZE+1] = {0};
    char desc[32] = {0};
    strncpy(apid, apid_const, DLT_ID_SIZE);
    apid[DLT_ID_SIZE] = '\0';
    strncpy(desc, desc_const, sizeof(desc)-1);
    desc[sizeof(desc)-1] = '\0';
    app = dlt_daemon_application_add(&daemon, apid, pid, desc,
                                     fd, ecu, 0);
    EXPECT_FALSE(app->trace_load_settings == NULL);
    EXPECT_EQ(app->trace_load_settings_count, 1);
    EXPECT_EQ(app->trace_load_settings[0].soft_limit,
              DLT_TRACE_LOAD_DAEMON_SOFT_LIMIT_DEFAULT);
    EXPECT_EQ(app->trace_load_settings[0].hard_limit,
              DLT_TRACE_LOAD_DAEMON_HARD_LIMIT_DEFAULT);
    EXPECT_STREQ(app->trace_load_settings[0].apid, apid);
    EXPECT_TRUE(app->trace_load_settings[0].ctid == NULL ||
                strlen(app->trace_load_settings[0].ctid) == 0);

    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));

    dlt_daemon_application_del(&daemon, app, ecu, 0);
    EXPECT_TRUE(app->trace_load_settings == NULL);
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}

TEST(t_dlt_daemon_application_add, trace_load_settings_configured)
{
    DltDaemon daemon;
    DltGateway gateway;
    const char *apid_const = "APP1";
    pid_t pid = 0;
    const char *desc_const = "HELLO_TEST";
    DltDaemonApplication *app = NULL;
    char ecu[] = "ECU1";
    int fd = 15;

    EXPECT_EQ(0, dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE,
                                 DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                                 DLT_DAEMON_RINGBUFFER_STEP_SIZE,
                                 DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                                 DLT_TRACE_STATUS_OFF, 0, 0));

    // must be done after daemon init, else data is overwritten
    setup_trace_load_settings(daemon);

    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK,
              strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    char apid[DLT_ID_SIZE+1] = {0};
    char desc[32] = {0};
    strncpy(apid, apid_const, DLT_ID_SIZE);
    apid[DLT_ID_SIZE] = '\0';
    strncpy(desc, desc_const, sizeof(desc)-1);
    desc[sizeof(desc)-1] = '\0';
    app = dlt_daemon_application_add(&daemon, apid, pid, desc,
                                     fd, ecu, 0);
    EXPECT_FALSE(app->trace_load_settings == NULL);
    EXPECT_EQ(app->trace_load_settings_count, 4);

    for (uint32_t i = 0; i < app->trace_load_settings_count; i++) {
        EXPECT_EQ(memcmp(&app->trace_load_settings[i],
                         &daemon.preconfigured_trace_load_settings[i],
                         sizeof(DltTraceLoadSettings)), 0);
    }

    EXPECT_LE(0, dlt_daemon_application_del(&daemon, app, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear(&daemon, ecu, 0));

    dlt_daemon_application_del(&daemon, app, ecu, 0);
    EXPECT_TRUE(app->trace_load_settings == NULL);
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}

TEST(t_dlt_daemon_user_send_trace_load_config, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    const char *apid_const = "APP1";
    pid_t pid = 0;
    const char *desc_const = "HELLO_TEST";
    DltDaemonApplication *app = NULL;
    char ecu[] = "ECU1";
    int fd = 15;

    EXPECT_EQ(0, dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE,
                                 DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                                 DLT_DAEMON_RINGBUFFER_STEP_SIZE,
                                 DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                                 DLT_TRACE_STATUS_OFF, 0, 0));

    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK,
              strncmp(daemon.ecuid, daemon.user_list[0].ecu, DLT_ID_SIZE));
    char apid[DLT_ID_SIZE+1] = {0};
    char desc[32] = {0};
    strncpy(apid, apid_const, DLT_ID_SIZE);
    apid[DLT_ID_SIZE] = '\0';
    strncpy(desc, desc_const, sizeof(desc)-1);
    desc[sizeof(desc)-1] = '\0';
    app = dlt_daemon_application_add(&daemon, apid, pid, desc,
                                     fd, ecu, 0);
    dlt_daemon_user_send_trace_load_config(&daemon, app, 0);
    dlt_daemon_application_del(&daemon, app, ecu, 0);

    setup_trace_load_settings(daemon);

    strncpy(apid, apid_const, DLT_ID_SIZE);
    apid[DLT_ID_SIZE] = '\0';
    strncpy(desc, desc_const, sizeof(desc)-1);
    desc[sizeof(desc)-1] = '\0';
    app = dlt_daemon_application_add(&daemon, apid, pid, desc,
                                     fd, ecu, 0);
    dlt_daemon_user_send_trace_load_config(&daemon, app, 0);
    dlt_daemon_application_del(&daemon, app, ecu, 0);
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}

#endif

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
