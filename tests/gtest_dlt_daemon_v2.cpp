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
 * \file gtest_dlt_daemon_v2.cpp
 */

/*******************************************************************************
**                                                                            **
**  FILE      : gtest_dlt_daemon_v2.cpp                                       **
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

#include "dlt_daemon_common_cfg.h"
#include <gtest/gtest.h>
#include <limits.h>
#include <stdio.h>
#include <syslog.h>

extern "C"
{
#include "dlt-daemon.h"
#include "dlt-daemon_cfg.h"
#include "dlt_user_cfg.h"
#include "dlt_version.h"
#include "dlt_client.h"
#include "dlt_protocol.h"
}
#ifdef DLT_TRACE_LOAD_CTRL_ENABLE

const int _trace_load_send_size = 100;

static void init_daemon(DltDaemon* daemon, char* ecu) {
    DltGateway gateway;
    strcpy(ecu, "ECU1");

    EXPECT_EQ(0,
              dlt_daemon_init(daemon, DLT_DAEMON_RINGBUFFER_MIN_SIZE, DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                              DLT_DAEMON_RINGBUFFER_STEP_SIZE, DLT_RUNTIME_DEFAULT_DIRECTORY, DLT_LOG_INFO,
                              DLT_TRACE_STATUS_OFF, 0, 0));

    dlt_set_id(daemon->ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(daemon, &gateway, 0, 0));
}

static void setup_trace_load_settings(DltDaemon& daemon)
{
    daemon.preconfigured_trace_load_settings_count = 6;
    daemon.preconfigured_trace_load_settings =
        (DltTraceLoadSettings *)malloc(daemon.preconfigured_trace_load_settings_count * sizeof(DltTraceLoadSettings));
    memset(daemon.preconfigured_trace_load_settings, 0, daemon.preconfigured_trace_load_settings_count * sizeof(DltTraceLoadSettings));

    // APP0 only has app id
    strcpy(daemon.preconfigured_trace_load_settings[0].apid, "APP0");
    daemon.preconfigured_trace_load_settings[0].soft_limit = 1000;
    daemon.preconfigured_trace_load_settings[0].hard_limit = 2987;

    // APP1 has only three contexts, no app id
    strcpy(daemon.preconfigured_trace_load_settings[1].apid, "APP1");
    strcpy(daemon.preconfigured_trace_load_settings[1].ctid, "CT01");
    daemon.preconfigured_trace_load_settings[1].soft_limit = 100;
    daemon.preconfigured_trace_load_settings[1].hard_limit = 200;

    strcpy(daemon.preconfigured_trace_load_settings[2].apid, "APP1");
    strcpy(daemon.preconfigured_trace_load_settings[2].ctid, "CT02");
    daemon.preconfigured_trace_load_settings[2].soft_limit = 300;
    daemon.preconfigured_trace_load_settings[2].hard_limit = 400;

    strcpy(daemon.preconfigured_trace_load_settings[3].apid, "APP1");
    strcpy(daemon.preconfigured_trace_load_settings[3].ctid, "CT03");
    daemon.preconfigured_trace_load_settings[3].soft_limit = 500;
    daemon.preconfigured_trace_load_settings[3].hard_limit = 600;

    // APP2 has app id and context
    strcpy(daemon.preconfigured_trace_load_settings[4].apid, "APP2");
    strcpy(daemon.preconfigured_trace_load_settings[4].ctid, "CT01");
    daemon.preconfigured_trace_load_settings[4].soft_limit = 700;
    daemon.preconfigured_trace_load_settings[4].hard_limit = 800;

    strcpy(daemon.preconfigured_trace_load_settings[5].apid, "APP2");
    daemon.preconfigured_trace_load_settings[5].soft_limit = 900;
    daemon.preconfigured_trace_load_settings[5].hard_limit = 1000;

    // APP3 is not configured at all, but will be added via application_add
    // to make sure we assign default value in that case

    qsort(daemon.preconfigured_trace_load_settings, daemon.preconfigured_trace_load_settings_count,
          sizeof(DltTraceLoadSettings), dlt_daemon_compare_trace_load_settings);
}


TEST(t_trace_load_keep_message_v2, normal) {
    DltDaemon daemon;
    DltDaemonLocal daemon_local = {};
    const int num_apps = 4;
    const char* app_ids[num_apps] = {"APP0", "APP1", "APP2", "APP3"};
    DltDaemonApplication *apps[num_apps] = {};

    char ecu[DLT_ID_SIZE] = {};

    pid_t pid = 0;
    int fd = 15;
    const char *desc = "HELLO_TEST";

    const auto set_extended_header = [&daemon_local]() {
        daemon_local.msg.extendedheader = (DltExtendedHeaderV2 *)(daemon_local.msg.headerbuffer + sizeof(DltStorageHeaderV2) +
                                                               sizeof(DltStandardHeaderV2));
        memset(daemon_local.msg.extendedheader, 0, sizeof(DltExtendedHeaderV2));
    };

    const auto set_extended_header_log_level = [&daemon_local](unsigned int log_level) {
        daemon_local.msg.extendedheader->msin = ((daemon_local.msg.extendedheader->msin) & ~DLT_MSIN_MTIN) | ((log_level) << DLT_MSIN_MTIN_SHIFT);
    };

    const auto log_until_hard_limit_reached = [&daemon, &daemon_local] (DltDaemonApplication *app) {
        while (trace_load_keep_message_v2(app, _trace_load_send_size, &daemon, &daemon_local, 0));
    };

    const auto check_debug_and_trace_can_log = [&](DltDaemonApplication* app) {
        // messages for debug and verbose logs are never dropped
        set_extended_header_log_level(DLT_LOG_VERBOSE);
        EXPECT_TRUE(trace_load_keep_message_v2(app,
                                            app->trace_load_settings->hard_limit * 10,
                                            &daemon, &daemon_local, 0));
        set_extended_header_log_level(DLT_LOG_DEBUG);
        EXPECT_TRUE(trace_load_keep_message_v2(app,
                                            app->trace_load_settings->hard_limit * 10,
                                            &daemon, &daemon_local, 0));

        set_extended_header_log_level(DLT_LOG_INFO);
    };

    init_daemon_v2(&daemon, ecu);
    setup_trace_load_settings_v2(daemon);

    for (int i = 0; i < num_apps; i++) {
        apps[i] = dlt_daemon_application_add_v2(&daemon, (char *)app_ids[i], pid, (char *)desc, fd, ecu, 0);
        EXPECT_FALSE(apps[i]->trace_load_settings == NULL);
    }

    // messages without extended header will be kept
    daemon_local.msg.extendedheader = NULL;
    EXPECT_TRUE(trace_load_keep_message_v2(
        apps[0], apps[0]->trace_load_settings->soft_limit, &daemon, &daemon_local, 0));

    set_extended_header_v2();
    check_debug_and_trace_can_log(apps[0]);

    // messages for apps that have not been registered should be dropped
    DltDaemonApplication app = {};
    EXPECT_FALSE(trace_load_keep_message_v2(&app, 42, &daemon, &daemon_local, 0));

    // Test if hard limit is reached for applications that only configure an application id
    // Meaning that the limit is shared between all contexts
    memcpy(daemon_local.msg.extendedheader->ctid, "CT01", DLT_ID_SIZE);
    log_until_hard_limit_reached(apps[0]);
    EXPECT_FALSE(trace_load_keep_message_v2(apps[0], _trace_load_send_size, &daemon, &daemon_local, 0));
    memcpy(daemon_local.msg.extendedheader->ctid, "CT02", DLT_ID_SIZE);
    EXPECT_FALSE(trace_load_keep_message_v2(apps[0], _trace_load_send_size, &daemon, &daemon_local, 0));
    // Even after exhausting the limits, make sure debug and trace still work
    check_debug_and_trace_can_log(apps[0]);

    // APP1 has only three contexts, no app id
    memcpy(daemon_local.msg.extendedheader->ctid, "CT01", DLT_ID_SIZE);
    log_until_hard_limit_reached(apps[1]);
    EXPECT_FALSE(trace_load_keep_message_v2(apps[1], _trace_load_send_size, &daemon, &daemon_local, 0));
    // CT01 has reached its limit, make sure CT02 still can log
    memcpy(daemon_local.msg.extendedheader->ctid, "CT02", DLT_ID_SIZE);
    EXPECT_TRUE(trace_load_keep_message_v2(apps[1], _trace_load_send_size, &daemon, &daemon_local, 0));
    // Set CT02 to hard limit to hard limit 0, which should drop all messages
    apps[1]->trace_load_settings[2].hard_limit = 0;
    EXPECT_FALSE(trace_load_keep_message_v2(apps[1], _trace_load_send_size, &daemon, &daemon_local, 0));

    // APP2 has context and app id configured
    // Exhaust app limit first
    memcpy(daemon_local.msg.extendedheader->ctid, "CTXX", DLT_ID_SIZE);
    log_until_hard_limit_reached(apps[2]);
    EXPECT_FALSE(trace_load_keep_message_v2(apps[2], _trace_load_send_size, &daemon, &daemon_local, 0));
    // Context logging should still be possible
    memcpy(daemon_local.msg.extendedheader->ctid, "CT01", DLT_ID_SIZE);
    EXPECT_TRUE(trace_load_keep_message_v2(apps[2], _trace_load_send_size, &daemon, &daemon_local, 0));

    // Test not configured context
    memcpy(daemon_local.msg.extendedheader->ctid, "CTXX", DLT_ID_SIZE);
    EXPECT_EQ(
            trace_load_keep_message_v2(apps[1], _trace_load_send_size, &daemon, &daemon_local, 0),
            DLT_TRACE_LOAD_DAEMON_HARD_LIMIT_DEFAULT != 0);

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
    return RUN_ALL_TESTS();
}
