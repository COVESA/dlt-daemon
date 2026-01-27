/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, V2 - Volvo Group
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
 * Shivam Goel <shivam.goel@volvo.com>
 *
 * \copyright Copyright © 2011-2015 V2 - Volvo Group. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file gtest_dlt_daemon_common_v2.cpp
 */

/*******************************************************************************
**                                                                            **
**  FILE      : gtest_dlt_daemon_common_v2.cpp                                       **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Shivam Goel shivam.goel@volvo.com                             **
**                                                                            **
**  PURPOSE   :                                                               **
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
**   sg         Shivam Goel                V2 - Volvo Group                   **
*******************************************************************************/

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


/* Begin Method:dlt_daemon_common::dlt_daemon_find_users_list_v2 */
TEST(t_dlt_daemon_find_users_list_v2, normal_one_list)
{
    DltDaemon daemon;
    DltGateway gateway;
    DltDaemonRegisteredUsers *user_list;
    char ecu[] = "ECU1";
    uint8_t eculen = (uint8_t)strlen(ecu);

    EXPECT_EQ(0, dlt_daemon_init(&daemon,
                                 DLT_DAEMON_RINGBUFFER_MIN_SIZE,
                                 DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                                 DLT_DAEMON_RINGBUFFER_STEP_SIZE,
                                 DLT_RUNTIME_DEFAULT_DIRECTORY,
                                 DLT_LOG_INFO, DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id_v2(daemon.ecuid2, ecu, eculen);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));

    user_list = dlt_daemon_find_users_list_v2(&daemon, eculen, ecu, 0);
    EXPECT_NE(user_list, nullptr);
    EXPECT_EQ(DLT_RETURN_OK, strncmp(user_list->ecuid2, daemon.ecuid2, daemon.ecuid2len));

    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}

TEST(t_dlt_daemon_find_users_list_v2, nullpointer)
{
    DltDaemon daemon;
    char ecu[] = "ECU1";

    EXPECT_EQ(NULL, dlt_daemon_find_users_list_v2(NULL, 0, NULL, 0));
    EXPECT_EQ(NULL, dlt_daemon_find_users_list_v2(&daemon, 0, NULL, 0));
    EXPECT_EQ(NULL, dlt_daemon_find_users_list_v2(NULL, 0, ecu, 0));
}

/* Begin Method:dlt_daemon_common::dlt_daemon_application_add_v2 */
TEST(t_dlt_daemon_application_add_v2, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    char apid[] = "TEST";
    uint8_t apidlen = (uint8_t)strlen(apid);
    pid_t pid = 0;
    char desc[] = "HELLO_TEST";
    DltDaemonApplication *app = NULL;
    char ecu[] = "ECU1";
    uint8_t eculen = (uint8_t)strlen(ecu);
    int fd = 15;

    /* Normal Use-Case */
    EXPECT_EQ(0,
              dlt_daemon_init(&daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));
    daemon.ecuid2len = eculen;
    memset(daemon.ecuid2, 0, sizeof(daemon.ecuid2));
    dlt_set_id_v2(daemon.ecuid2, ecu, eculen);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, strncmp(daemon.ecuid2, daemon.user_list[0].ecuid2, eculen));

    app = dlt_daemon_application_add_v2(&daemon, apidlen, apid, pid, desc, fd, eculen, ecu, 0);
    // printf("### APP: APID=%s  DESCR=%s NUMCONTEXT=%i PID=%i USERHANDLE=%i\n", app->apid2,app->application_description, app->num_contexts, app->pid, app->user_handle);
    EXPECT_STREQ(apid, app->apid2);
    EXPECT_STREQ(desc, app->application_description);
    EXPECT_EQ(pid, app->pid);
    EXPECT_LE(0, dlt_daemon_application_del_v2(&daemon, app, eculen, ecu, 0));
    EXPECT_LE(0, dlt_daemon_applications_clear_v2(&daemon, ecu, 0));
    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));
}
/* End Method:dlt_daemon_common::dlt_daemon_application_add_v2 */


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