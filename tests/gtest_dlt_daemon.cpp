
/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2024, Mercedes Benz Tech Innovation GmbH
 *
 * This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see https://www.covesa.global/.
 */

/*!
 * \author
 * Alexander Mohr <alexander.m.mohr@mercedes-benz.com>
 *
 * \copyright Copyright © 2024 Mercedes Benz Tech Innovation GmbH. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file gtest_dlt_daemon.cpp
 */

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
    strncpy(ecu, "ECU1", DLT_ID_SIZE);

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
    strncpy(daemon.preconfigured_trace_load_settings[0].apid, "APP0", DLT_ID_SIZE);
    daemon.preconfigured_trace_load_settings[0].soft_limit = 1000;
    daemon.preconfigured_trace_load_settings[0].hard_limit = 2987;

    // APP1 has only three contexts, no app id
    strncpy(daemon.preconfigured_trace_load_settings[1].apid, "APP1", DLT_ID_SIZE);
    strncpy(daemon.preconfigured_trace_load_settings[1].ctid, "CT01", DLT_ID_SIZE);
    daemon.preconfigured_trace_load_settings[1].soft_limit = 100;
    daemon.preconfigured_trace_load_settings[1].hard_limit = 200;

    strncpy(daemon.preconfigured_trace_load_settings[2].apid, "APP1", DLT_ID_SIZE);
    strncpy(daemon.preconfigured_trace_load_settings[2].ctid, "CT02", DLT_ID_SIZE);
    daemon.preconfigured_trace_load_settings[2].soft_limit = 300;
    daemon.preconfigured_trace_load_settings[2].hard_limit = 400;

    strncpy(daemon.preconfigured_trace_load_settings[3].apid, "APP1", DLT_ID_SIZE);
    strncpy(daemon.preconfigured_trace_load_settings[3].ctid, "CT03", DLT_ID_SIZE);
    daemon.preconfigured_trace_load_settings[3].soft_limit = 500;
    daemon.preconfigured_trace_load_settings[3].hard_limit = 600;

    // APP2 has app id and context
    strncpy(daemon.preconfigured_trace_load_settings[4].apid, "APP2", DLT_ID_SIZE);
    strncpy(daemon.preconfigured_trace_load_settings[4].ctid, "CT01", DLT_ID_SIZE);
    daemon.preconfigured_trace_load_settings[4].soft_limit = 700;
    daemon.preconfigured_trace_load_settings[4].hard_limit = 800;

    strncpy(daemon.preconfigured_trace_load_settings[5].apid, "APP2", DLT_ID_SIZE);
    daemon.preconfigured_trace_load_settings[5].soft_limit = 900;
    daemon.preconfigured_trace_load_settings[5].hard_limit = 1000;

    // APP3 is not configured at all, but will be added via application_add
    // to make sure we assign default value in that case

    qsort(daemon.preconfigured_trace_load_settings, daemon.preconfigured_trace_load_settings_count,
          sizeof(DltTraceLoadSettings), dlt_daemon_compare_trace_load_settings);
}

TEST(t_trace_load_config_file_parser, normal)
{
    DltDaemon daemon;
    daemon.preconfigured_trace_load_settings = NULL;
    daemon.preconfigured_trace_load_settings_count = 0;
    DltDaemonLocal daemon_local;

    // XXXXXX is required by mkstemp
    char filename_template[] = "t_trace_load_config_file_parser.XXXXXX";
    int fd = mkstemp(filename_template);

    ASSERT_NE(fd, -1) << "Unable to create temporary file: " << strerror(errno);

    // Convert file descriptor to FILE* with fdopen
    FILE *temp = fdopen(fd, "w");

    if (temp == NULL) {
        close(fd);
        GTEST_FAIL() << "Unable to open file stream";
    }

    fprintf(temp, "QSYM QSLA 83333 100000\n");
    fprintf(temp, "TEST 123 456\n");
    fprintf(temp, "TEST FOO 42 100\n");
    fprintf(temp, "BAR 42 42 \n");
    fprintf(temp, "FOO BAR 84 84\n");
    fprintf(temp, "AP12  CTX1  100  200\n");
    fflush(temp);
    fclose(temp);

    strcpy(daemon_local.flags.lvalue, filename_template);

    trace_load_config_file_parser(&daemon, &daemon_local);

    EXPECT_EQ(daemon.preconfigured_trace_load_settings_count, 6);

    EXPECT_EQ(strncmp(daemon.preconfigured_trace_load_settings[0].apid, "AP12", DLT_ID_SIZE), 0);
    EXPECT_EQ(strncmp(daemon.preconfigured_trace_load_settings[0].ctid, "CTX1", DLT_ID_SIZE), 0);
    EXPECT_EQ(daemon.preconfigured_trace_load_settings[0].soft_limit, 100);
    EXPECT_EQ(daemon.preconfigured_trace_load_settings[0].hard_limit, 200);

    EXPECT_EQ(strncmp(daemon.preconfigured_trace_load_settings[1].apid, "BAR", DLT_ID_SIZE), 0);
    EXPECT_EQ(strlen(daemon.preconfigured_trace_load_settings[1].ctid), 0);
    EXPECT_EQ(daemon.preconfigured_trace_load_settings[1].soft_limit, 42);
    EXPECT_EQ(daemon.preconfigured_trace_load_settings[1].hard_limit, 42);

    EXPECT_EQ(strncmp(daemon.preconfigured_trace_load_settings[2].apid, "FOO", DLT_ID_SIZE), 0);
    EXPECT_EQ(strncmp(daemon.preconfigured_trace_load_settings[2].ctid, "BAR", DLT_ID_SIZE), 0);
    EXPECT_EQ(daemon.preconfigured_trace_load_settings[2].soft_limit, 84);
    EXPECT_EQ(daemon.preconfigured_trace_load_settings[2].hard_limit, 84);

    EXPECT_EQ(strncmp(daemon.preconfigured_trace_load_settings[3].apid, "QSYM", DLT_ID_SIZE), 0);
    EXPECT_EQ(strncmp(daemon.preconfigured_trace_load_settings[3].ctid, "QSLA", DLT_ID_SIZE), 0);
    EXPECT_EQ(daemon.preconfigured_trace_load_settings[3].soft_limit, 83333);
    EXPECT_EQ(daemon.preconfigured_trace_load_settings[3].hard_limit, 100000);

    EXPECT_EQ(strncmp(daemon.preconfigured_trace_load_settings[4].apid, "TEST", DLT_ID_SIZE), 0);
    EXPECT_EQ(strlen(daemon.preconfigured_trace_load_settings[4].ctid), 0);
    EXPECT_EQ(daemon.preconfigured_trace_load_settings[4].soft_limit, 123);
    EXPECT_EQ(daemon.preconfigured_trace_load_settings[4].hard_limit, 456);

    EXPECT_EQ(strncmp(daemon.preconfigured_trace_load_settings[5].apid, "TEST", DLT_ID_SIZE), 0);
    EXPECT_EQ(strncmp(daemon.preconfigured_trace_load_settings[5].ctid, "FOO", DLT_ID_SIZE), 0);
    EXPECT_EQ(daemon.preconfigured_trace_load_settings[5].soft_limit, 42);
    EXPECT_EQ(daemon.preconfigured_trace_load_settings[5].hard_limit, 100);

    free(daemon.preconfigured_trace_load_settings);
    unlink(filename_template);
}

TEST(t_trace_load_config_file_parser, errors)
{
    DltDaemon daemon;
    daemon.preconfigured_trace_load_settings = NULL;
    daemon.preconfigured_trace_load_settings_count = 0;
    DltDaemonLocal daemon_local;

    // XXXXXX is required by mkstemp
    char filename_template[] = "t_trace_load_config_file_parser.XXXXXX";
    int fd = mkstemp(filename_template);

    ASSERT_NE(fd, -1) << "Unable to create temporary file: " << strerror(errno);

    // Convert file descriptor to FILE* with fdopen
    FILE *temp = fdopen(fd, "w");

    if (temp == NULL) {
        close(fd);
        GTEST_FAIL() << "Unable to open file stream";
    }

    // Test case 1: ID Length Exceeded
    fprintf(temp, "APPID1234 100 200\n");
    fprintf(temp, "APP1 CTXID123 100 200\n");

    // Test case 2: Missing Fields
    fprintf(temp, "APP2 100\n");

    // Test case 3: Extra Fields
    fprintf(temp, "APP3 CTX1 100 200 EXTRA\n");

    // Test case 4: Invalid ID Characters
    fprintf(temp, "\U0001F480 100 200\n");
    fprintf(temp, "APP4 \U0001F480 100 200\n");
    fprintf(temp, "\U0001F480 \U0001F480 100 200\n");

    // Test case 5: Non-numeric Limits
    fprintf(temp, "APP5 CTX1 42 HARD\n");
    fprintf(temp, "APP6 CTX1 SOFT 42\n");
    fprintf(temp, "APP6 CTX1 SOFT HARD\n");

    // Test case 6: Negative Limits
    fprintf(temp, "APP7 CTX1 -100 200\n");
    fprintf(temp, "APP8 CTX1 100 -200\n");
    fprintf(temp, "APP9 CTX1 -100 -200\n");

    // Test case 7: Limits Out of Order
    fprintf(temp, "AP10 CTX1 300 200\n");

    // Test case 8: Whitespace Issues
    fprintf(temp, "APP1100 200\n");

    // Test case 9: Optional CTXID Omitted Incorrectly
    fprintf(temp, "APP1CTX1 100 200\n");

    // Test case 10: Comments Misplacement or Format
    fprintf(temp, "APP1 CTX1 100 # This is a comment 200\n");
    fprintf(temp, "APP1 CTX1 100 200 // This is a comment\n");
    
    // Test case 11: empty lines
    fprintf(temp, " \n");
    fprintf(temp, "\n");

    // Test case 12: missing new line
    fprintf(temp, "APP1 100 200 APP2 300 400\n");

    // Test case 13: data longer than value length
    char long_data[4096];
    memset(long_data, 'A', sizeof(long_data));
    fprintf(temp, "%s 100 200 APP2 300 400\n", long_data);

    // Test case 14: tabs
    fprintf(temp, "APP1\t100\t200\n");
    fprintf(temp, "APP1\t100\t200\t\n");
    fprintf(temp, "APP1 100 200\t \n");
    fprintf(temp, "APP1\t100 200 \n");

    fflush(temp);
    fclose(temp);

    strcpy(daemon_local.flags.lvalue, filename_template);

    trace_load_config_file_parser(&daemon, &daemon_local);
    EXPECT_EQ(daemon.preconfigured_trace_load_settings_count, 0);
    free(daemon.preconfigured_trace_load_settings);
    unlink(filename_template);
}

static void matches_default_element(DltDaemonApplication *app, DltTraceLoadSettings *settings) {
    EXPECT_STREQ(settings->apid, app->apid);
    EXPECT_STREQ(settings->ctid, "");
    EXPECT_EQ(settings->soft_limit, DLT_TRACE_LOAD_DAEMON_SOFT_LIMIT_DEFAULT);
    EXPECT_EQ(settings->hard_limit, DLT_TRACE_LOAD_DAEMON_HARD_LIMIT_DEFAULT);
}

TEST(t_find_runtime_trace_load_settings, normal) {
    DltDaemon daemon;
    const int num_apps = 4;
    const char* app_ids[num_apps] = {"APP0", "APP1", "APP2", "APP3"};
    DltDaemonApplication *apps[num_apps] = {};

    DltTraceLoadSettings* settings;
    char ecu[DLT_ID_SIZE] = {};

    pid_t pid = 0;
    int fd = 15;
    const char *desc = "HELLO_TEST";

    init_daemon(&daemon, ecu);
    setup_trace_load_settings(daemon);

    for (int i = 0; i < num_apps; i++) {
        apps[i] = dlt_daemon_application_add(&daemon, (char *)app_ids[i], pid, (char *)desc, fd, ecu, 0);
        EXPECT_FALSE(apps[i]->trace_load_settings == NULL);
    }

    // Find settings for APP0 with context that has not been configured
    settings = dlt_find_runtime_trace_load_settings(
        apps[0]->trace_load_settings, apps[0]->trace_load_settings_count,
        apps[0]->apid, "FOO");
    EXPECT_EQ(memcmp(settings, &daemon.preconfigured_trace_load_settings[0], sizeof(DltTraceLoadSettings)), 0);

    // Find settings for APP1 with context that has been configured
    settings = dlt_find_runtime_trace_load_settings(
        apps[1]->trace_load_settings, apps[1]->trace_load_settings_count,
        apps[1]->apid, "CT01");
    EXPECT_EQ(memcmp(settings, &daemon.preconfigured_trace_load_settings[1], sizeof(DltTraceLoadSettings)), 0);

    settings = dlt_find_runtime_trace_load_settings(
        apps[1]->trace_load_settings, apps[1]->trace_load_settings_count,
        apps[1]->apid, "CT02");
    EXPECT_EQ(memcmp(settings, &daemon.preconfigured_trace_load_settings[2], sizeof(DltTraceLoadSettings)), 0);

    settings = dlt_find_runtime_trace_load_settings(
        apps[1]->trace_load_settings, apps[1]->trace_load_settings_count,
        apps[1]->apid, "CT03");
    EXPECT_EQ(memcmp(settings, &daemon.preconfigured_trace_load_settings[3], sizeof(DltTraceLoadSettings)), 0);

    // This context does not have a configuration, so the default element should be returned
    settings = dlt_find_runtime_trace_load_settings(
        apps[1]->trace_load_settings, apps[1]->trace_load_settings_count,
        apps[1]->apid, "CTXX");
    matches_default_element(apps[1], settings);

    // APP2 has app id and context configured
    settings = dlt_find_runtime_trace_load_settings(
        apps[2]->trace_load_settings, apps[2]->trace_load_settings_count,
        apps[2]->apid, "CTXX");
    EXPECT_EQ(memcmp(settings, &daemon.preconfigured_trace_load_settings[4], sizeof(DltTraceLoadSettings)), 0);

    settings = dlt_find_runtime_trace_load_settings(
        apps[2]->trace_load_settings, apps[2]->trace_load_settings_count,
        apps[2]->apid, "CT01");
    EXPECT_EQ(memcmp(settings, &daemon.preconfigured_trace_load_settings[5], sizeof(DltTraceLoadSettings)), 0);

    // Find settings for APP3 that has not been configured at all
    settings = dlt_find_runtime_trace_load_settings(
        apps[3]->trace_load_settings, apps[3]->trace_load_settings_count,
        apps[3]->apid, "TEST");
    matches_default_element(apps[3], settings);

    // Nullptr for everything
    settings = dlt_find_runtime_trace_load_settings(NULL, 0, NULL, NULL);
    EXPECT_TRUE(settings == NULL);

    // Nullptr for app
    settings = dlt_find_runtime_trace_load_settings(
        apps[0]->trace_load_settings, apps[0]->trace_load_settings_count, NULL,
        NULL);
    EXPECT_TRUE(settings == NULL);

    // Empty appid
    settings = dlt_find_runtime_trace_load_settings(
        apps[0]->trace_load_settings, apps[0]->trace_load_settings_count, "",
        NULL);
    EXPECT_TRUE(settings == NULL);

    // Context id invalid
    settings = dlt_find_runtime_trace_load_settings(
        apps[0]->trace_load_settings, apps[0]->trace_load_settings_count,
        apps[0]->apid, "FOOBAR");
    EXPECT_EQ(memcmp(settings, &daemon.preconfigured_trace_load_settings[0], sizeof(DltTraceLoadSettings)), 0);

    // remove application, add it again and check if the settings are still there
    dlt_daemon_application_del(&daemon, apps[0], ecu, 0);
    apps[0] = dlt_daemon_application_add(&daemon, (char *)app_ids[0], pid, (char *)desc, fd, ecu, 0);
    settings = dlt_find_runtime_trace_load_settings(
        apps[0]->trace_load_settings, apps[0]->trace_load_settings_count,
        apps[0]->apid, "FOO");
    EXPECT_EQ(memcmp(settings, &daemon.preconfigured_trace_load_settings[0], sizeof(DltTraceLoadSettings)), 0);

    EXPECT_EQ(0, dlt_daemon_free(&daemon, 0));

    // Test after freeing daemon
    settings = dlt_find_runtime_trace_load_settings(
        apps[0]->trace_load_settings, apps[0]->trace_load_settings_count,
        apps[0]->apid, NULL);
    EXPECT_TRUE(settings == NULL);
    EXPECT_TRUE(apps[0]->trace_load_settings == NULL);
}

TEST(t_trace_load_keep_message, normal) {
    DltDaemon daemon;
    DltDaemonLocal daemon_local = {};
    const int num_apps = 4;
    const char* app_ids[num_apps] = {"APP0", "APP1", "APP2", "APP3"};
    DltDaemonApplication *apps[num_apps] = {};

    char ecu[DLT_ID_SIZE] = {};

    pid_t pid = 0;
    int fd = 15;
    const char *desc = "HELLO_TEST";

    for (auto& app_id : app_ids) {
        dlt_register_app(app_id, app_id);
    }

    const auto set_extended_header = [&daemon_local]() {
        daemon_local.msg.extendedheader = (DltExtendedHeader *)(daemon_local.msg.headerbuffer + sizeof(DltStorageHeader) +
                                                               sizeof(DltStandardHeader));
        memset(daemon_local.msg.extendedheader, 0, sizeof(DltExtendedHeader));
    };

    const auto set_extended_header_log_level = [&daemon_local](unsigned int log_level) {
        daemon_local.msg.extendedheader->msin = ((daemon_local.msg.extendedheader->msin) & ~DLT_MSIN_MTIN) | ((log_level) << DLT_MSIN_MTIN_SHIFT);
    };

    const auto log_until_hard_limit_reached = [&daemon, &daemon_local] (DltDaemonApplication *app) {
        while (trace_load_keep_message(app, _trace_load_send_size, &daemon, &daemon_local, 0));
    };

    const auto check_debug_and_trace_can_log = [&](DltDaemonApplication* app) {
        // messages for debug and verbose logs are never dropped
        set_extended_header_log_level(DLT_LOG_VERBOSE);

        EXPECT_TRUE(trace_load_keep_message(app,
                                            app->trace_load_settings->hard_limit * 10,
                                            &daemon, &daemon_local, 0));
        set_extended_header_log_level(DLT_LOG_DEBUG);
        EXPECT_TRUE(trace_load_keep_message(app,
                                            app->trace_load_settings->hard_limit * 10,
                                            &daemon, &daemon_local, 0));

        set_extended_header_log_level(DLT_LOG_INFO);
    };

    init_daemon(&daemon, ecu);
    setup_trace_load_settings(daemon);

    for (int i = 0; i < num_apps; i++) {
        apps[i] = dlt_daemon_application_add(&daemon, (char *)app_ids[i], pid, (char *)desc, fd, ecu, 0);
        EXPECT_FALSE(apps[i]->trace_load_settings == NULL);
    }

    // messages without extended header will be kept
    daemon_local.msg.extendedheader = NULL;
    EXPECT_TRUE(trace_load_keep_message(
        apps[0], apps[0]->trace_load_settings->soft_limit, &daemon, &daemon_local, 0));

    set_extended_header();
    memcpy(daemon_local.msg.extendedheader->apid, apps[0], DLT_ID_SIZE);

    // messages for apps that have not been registered should be dropped
    DltDaemonApplication app = {};
    EXPECT_FALSE(trace_load_keep_message(&app, 42, &daemon, &daemon_local, 0));

    // Test if hard limit is reached for applications that only configure an application id
    // Meaning that the limit is shared between all contexts
    dlt_daemon_context_add(&daemon, apps[0]->apid, "CT01", DLT_LOG_VERBOSE, 0, 0, 0, "", "ECU1", 0);
    dlt_daemon_context_add(&daemon, apps[0]->apid, "CT02", DLT_LOG_VERBOSE, 0, 0, 0, "", "ECU1", 0);
    memcpy(daemon_local.msg.extendedheader->ctid, "CT01", DLT_ID_SIZE);
    log_until_hard_limit_reached(apps[0]);
    EXPECT_FALSE(trace_load_keep_message(apps[0], _trace_load_send_size, &daemon, &daemon_local, 0));
    memcpy(daemon_local.msg.extendedheader->ctid, "CT02", DLT_ID_SIZE);
    EXPECT_FALSE(trace_load_keep_message(apps[0], _trace_load_send_size, &daemon, &daemon_local, 0));
    // Even after exhausting the limits, make sure debug and trace still work
    check_debug_and_trace_can_log(apps[0]);

    // APP1 has only three contexts, no app id
    memcpy(daemon_local.msg.extendedheader->ctid, "CT01", DLT_ID_SIZE);
    memcpy(daemon_local.msg.extendedheader->apid, apps[1], DLT_ID_SIZE);

    dlt_daemon_context_add(&daemon, apps[1]->apid, "CT02", DLT_LOG_VERBOSE, 0, 0, 0, "", "ECU1", 0);
    dlt_daemon_context_add(&daemon, apps[1]->apid, "CT02", DLT_LOG_VERBOSE, 0, 0, 0, "", "ECU1", 0);

    log_until_hard_limit_reached(apps[1]);
    EXPECT_FALSE(trace_load_keep_message(apps[1], _trace_load_send_size, &daemon, &daemon_local, 0));
    // CT01 has reached its limit, make sure CT02 still can log
    memcpy(daemon_local.msg.extendedheader->ctid, "CT02", DLT_ID_SIZE);
    EXPECT_TRUE(trace_load_keep_message(apps[1], _trace_load_send_size, &daemon, &daemon_local, 0));
    // Set CT02 to hard limit to hard limit 0, which should drop all messages
    apps[1]->trace_load_settings[2].hard_limit = 0;
    EXPECT_FALSE(trace_load_keep_message(apps[1], _trace_load_send_size, &daemon, &daemon_local, 0));

    // APP2 has context and app id configured
    // Exhaust app limit first
    memcpy(daemon_local.msg.extendedheader->ctid, "CTXX", DLT_ID_SIZE);
    memcpy(daemon_local.msg.extendedheader->apid, apps[2], DLT_ID_SIZE);
    dlt_daemon_context_add(&daemon, apps[2]->apid, "CTXX", DLT_LOG_VERBOSE, 0, 0, 0, "", "ECU1", 0);
    dlt_daemon_context_add(&daemon, apps[2]->apid, "CT01", DLT_LOG_VERBOSE, 0, 0, 0, "", "ECU1", 0);

    log_until_hard_limit_reached(apps[2]);
    EXPECT_FALSE(trace_load_keep_message(apps[2], _trace_load_send_size, &daemon, &daemon_local, 0));
    // Context logging should still be possible
    memcpy(daemon_local.msg.extendedheader->ctid, "CT01", DLT_ID_SIZE);
    EXPECT_TRUE(trace_load_keep_message(apps[2], _trace_load_send_size, &daemon, &daemon_local, 0));

    // Test not configured context
    memcpy(daemon_local.msg.extendedheader->ctid, "CTXX", DLT_ID_SIZE);
    memcpy(daemon_local.msg.extendedheader->apid, apps[1], DLT_ID_SIZE);
    dlt_daemon_context_add(&daemon, apps[1]->apid, "CTXX", DLT_LOG_VERBOSE, 0, 0, 0, "", "ECU1", 0);

    EXPECT_EQ(
            trace_load_keep_message(apps[1], _trace_load_send_size, &daemon, &daemon_local, 0),
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
