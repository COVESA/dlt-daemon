
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
 * Minh Luu Quang <minh.luuquang@vn.bosch.com>
 *
 * \copyright Copyright © 2024 Mercedes Benz Tech Innovation GmbH. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file gtest_dlt_daemon.cpp
 */

#include <gtest/gtest.h>
#include <limits.h>
#include <stdio.h>
#include <syslog.h>

extern "C"
{
    #include "dlt_daemon_common_cfg.h"
    #include "dlt-daemon_cfg.h"
    #include "dlt-daemon.h"
    #include "dlt_user_cfg.h"
    #include "dlt_client.h"
    #include "dlt_version.h"
    #include "dlt_protocol.h"
}

#include "gmock_dlt_daemon.h"

// Test Fixture for close_pipes
class ClosePipesTest : public ::testing::Test {
    protected:
        int fds[2];

        void SetUp() override {
            // Initialize file descriptors to invalid state
            fds[0] = DLT_FD_INIT;
            fds[1] = DLT_FD_INIT;
        }

        void TearDown() override {
            // Ensure no file descriptors are left open
            if (fds[0] != DLT_FD_INIT) close(fds[0]);
            if (fds[1] != DLT_FD_INIT) close(fds[1]);
        }
};

// Test closing valid pipes
TEST_F(ClosePipesTest, ClosesValidPipes) {
    // Create a pipe
    ASSERT_EQ(pipe(fds), 0);

    // Ensure file descriptors are valid
    ASSERT_GT(fds[0], 0);
    ASSERT_GT(fds[1], 0);

    // Close the pipes
    close_pipes(fds);

    // Verify file descriptors are reset to DLT_FD_INIT
    EXPECT_EQ(fds[0], DLT_FD_INIT);
    EXPECT_EQ(fds[1], DLT_FD_INIT);
}

// Test closing one invalid and one valid pipe
TEST_F(ClosePipesTest, ClosesMixedPipes) {
    // Create a pipe
    ASSERT_EQ(pipe(fds), 0);

    // Close one pipe manually
    close(fds[0]);
    fds[0] = DLT_FD_INIT;

    // Close the pipes using the function
    close_pipes(fds);

    // Verify file descriptors are reset to DLT_FD_INIT
    EXPECT_EQ(fds[0], DLT_FD_INIT);
    EXPECT_EQ(fds[1], DLT_FD_INIT);
}

// Test closing already closed pipes
TEST_F(ClosePipesTest, HandlesAlreadyClosedPipes) {
    // Set file descriptors to invalid state
    fds[0] = DLT_FD_INIT;
    fds[1] = DLT_FD_INIT;

    // Close the pipes using the function
    close_pipes(fds);

    // Verify file descriptors remain DLT_FD_INIT
    EXPECT_EQ(fds[0], DLT_FD_INIT);
    EXPECT_EQ(fds[1], DLT_FD_INIT);
}

// Test closing pipes with negative file descriptors
TEST_F(ClosePipesTest, HandlesNegativeFileDescriptors) {
    // Set file descriptors to negative values
    fds[0] = -100;
    fds[1] = -200;

    // Close the pipes using the function
    close_pipes(fds);

    // Verify file descriptors are reset to DLT_FD_INIT
    EXPECT_EQ(fds[0], DLT_FD_INIT);
    EXPECT_EQ(fds[1], DLT_FD_INIT);
}

// Test closing pipes with one negative and one valid file descriptor
TEST_F(ClosePipesTest, HandlesMixedNegativeAndValidDescriptors) {
    // Create a pipe
    ASSERT_EQ(pipe(fds), 0);

    // Set one file descriptor to a negative value
    fds[0] = -100;

    // Close the pipes using the function
    close_pipes(fds);

    // Verify file descriptors are reset to DLT_FD_INIT
    EXPECT_EQ(fds[0], DLT_FD_INIT);
    EXPECT_EQ(fds[1], DLT_FD_INIT);
}

extern "C" void usage();

// Test Fixture for usage()
class UsageTest : public ::testing::Test {
    protected:
        std::stringstream output;
        DltVersionMockImpl mock;  // Use the concrete implementation
        std::streambuf* orig_cout_buffer;

        void SetUp() override {
            // Store the original buffer and redirect stdout to a stringstream
            orig_cout_buffer = std::cout.rdbuf(output.rdbuf());
        }

        void TearDown() override {
            // Restore the original stdout buffer
            std::cout.rdbuf(orig_cout_buffer);
        }
};


// Test Cases
TEST_F(UsageTest, PrintsVersion) {
    usage();  // Call the function being tested

    std::string outputStr = output.str();
    EXPECT_NE(outputStr.find("DLT Daemon Version 2.18.0"), std::string::npos);
}

TEST_F(UsageTest, PrintsUsageMessage) {
    usage();  // Call the function being tested

    std::string outputStr = output.str();
    EXPECT_NE(outputStr.find("Usage: dlt-daemon [options]"), std::string::npos);
}

// Test if usage() prints the correct options
TEST_F(UsageTest, PrintsOptions) {
    usage();
    std::string outputStr = output.str();

    // Check for common options
    EXPECT_NE(outputStr.find("  -d            Daemonize"), std::string::npos);
    EXPECT_NE(outputStr.find("  -h            Usage"), std::string::npos);
    EXPECT_NE(outputStr.find("  -c filename   DLT daemon configuration file"), std::string::npos);
    EXPECT_NE(outputStr.find("  -p port       port to monitor for incoming requests"), std::string::npos);

    // Check for conditional options
#ifdef DLT_DAEMON_USE_FIFO_IPC
    EXPECT_NE(outputStr.find("  -t directory  Directory for local fifo and user-pipes"), std::string::npos);
#endif

#ifdef DLT_SHM_ENABLE
    EXPECT_NE(outputStr.find("  -s filename   The file name to create the share memory"), std::string::npos);
#endif

#ifdef DLT_LOG_LEVEL_APP_CONFIG
    EXPECT_NE(outputStr.find("  -a filename   The filename for load default app id log levels"), std::string::npos);
#endif

#ifdef DLT_TRACE_LOAD_CTRL_ENABLE
    EXPECT_NE(outputStr.find("  -l filename   The filename for load limits"), std::string::npos);
#endif
}

extern "C" int option_handling(DltDaemonLocal *daemon_local, int argc, char *argv[]);

// Test Fixture for option_handling
class OptionHandlingTest : public ::testing::Test {
    protected:
        DltLogSetFifoBasedirMock dltLogSetFifoBasedirMock;
        UsageMock usageMock;
        FprintfMock fprintfMock;
        std::stringstream stderrBuffer;

        void SetUp() override {
            // Redirect stderr to capture error messages
            std::cerr.rdbuf(stderrBuffer.rdbuf());

            // Set default mock behaviors
            ON_CALL(fprintfMock, fprintf_wrapper(_, _))
                .WillByDefault([](FILE *stream, const char *format) {
                    (void)stream; // Mark parameter as used to suppress warning
                    (void)format;
                    return 0; // Simulate successful fprintf
                });
        }

        void TearDown() override {
            // Restore stderr
            std::cerr.rdbuf(std::cerr.rdbuf(nullptr));
        }
};

// Test 1: Invalid Parameter (Null daemon_local)
TEST_F(OptionHandlingTest, InvalidParameter) {
    EXPECT_CALL(fprintfMock, fprintf_wrapper(stderr, "Invalid parameter passed to option_handling()\n"))
        .Times(1);

    int result = option_handling(nullptr, 0, nullptr);
    EXPECT_EQ(result, -1);
}

// Test 2: Default Values
TEST_F(OptionHandlingTest, DefaultValues) {
    DltDaemonLocal daemon_local;
    char *argv[] = { (char *)"dlt-daemon" };
    int argc = 1;

    EXPECT_CALL(dltLogSetFifoBasedirMock, dlt_log_set_fifo_basedir(DLT_USER_IPC_PATH))
        .Times(1);

    int result = option_handling(&daemon_local, argc, argv);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(daemon_local.flags.port, DLT_DAEMON_TCP_PORT);
}

// Test 3: Valid Options
TEST_F(OptionHandlingTest, ValidOptions) {
    DltDaemonLocal daemon_local;
    char *argv[] = { (char *)"dlt-daemon", (char *)"-d", (char *)"-c", (char *)"config.conf", (char *)"-p", (char *)"3491" };
    int argc = 6;

    EXPECT_CALL(dltLogSetFifoBasedirMock, dlt_log_set_fifo_basedir(DLT_USER_IPC_PATH))
        .Times(1);

    int result = option_handling(&daemon_local, argc, argv);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(daemon_local.flags.dflag, 1);
    EXPECT_STREQ(daemon_local.flags.cvalue, "config.conf");
    EXPECT_EQ(daemon_local.flags.port, 3491);
}

// Test 4: Invalid Port
TEST_F(OptionHandlingTest, InvalidPort) {
    DltDaemonLocal daemon_local;
    char *argv[] = { (char *)"dlt-daemon", (char *)"-p", (char *)"invalid" };
    int argc = 3;

    EXPECT_CALL(fprintfMock, fprintf_wrapper(stderr, "Invalid port `invalid' specified.\n"))
        .Times(1);

    int result = option_handling(&daemon_local, argc, argv);
    EXPECT_EQ(result, -1);
}

// Test 5: Missing Argument
TEST_F(OptionHandlingTest, MissingArgument) {
    DltDaemonLocal daemon_local;
    char *argv[] = { (char *)"dlt-daemon", (char *)"-c" };
    int argc = 2;

    EXPECT_CALL(fprintfMock, fprintf_wrapper(stderr, "Option -c requires an argument.\n"))
        .Times(1);
    EXPECT_CALL(usageMock, usage())
        .Times(1);

    int result = option_handling(&daemon_local, argc, argv);
    EXPECT_EQ(result, -1);
}

// Test 6: Unknown Option
TEST_F(OptionHandlingTest, UnknownOption) {
    DltDaemonLocal daemon_local;
    char *argv[] = { (char *)"dlt-daemon", (char *)"-x" };
    int argc = 2;

    EXPECT_CALL(fprintfMock, fprintf_wrapper(stderr, "Unknown option `-x'.\n"))
        .Times(1);
    EXPECT_CALL(usageMock, usage())
        .Times(1);

    int result = option_handling(&daemon_local, argc, argv);
    EXPECT_EQ(result, -1);
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
    check_debug_and_trace_can_log(apps[0]);

    // messages for apps that have not been registered should be dropped
    DltDaemonApplication app = {};
    EXPECT_FALSE(trace_load_keep_message(&app, 42, &daemon, &daemon_local, 0));

    // Test if hard limit is reached for applications that only configure an application id
    // Meaning that the limit is shared between all contexts
    memcpy(daemon_local.msg.extendedheader->ctid, "CT01", DLT_ID_SIZE);
    log_until_hard_limit_reached(apps[0]);
    EXPECT_FALSE(trace_load_keep_message(apps[0], _trace_load_send_size, &daemon, &daemon_local, 0));
    memcpy(daemon_local.msg.extendedheader->ctid, "CT02", DLT_ID_SIZE);
    EXPECT_FALSE(trace_load_keep_message(apps[0], _trace_load_send_size, &daemon, &daemon_local, 0));
    // Even after exhausting the limits, make sure debug and trace still work
    check_debug_and_trace_can_log(apps[0]);

    // APP1 has only three contexts, no app id
    memcpy(daemon_local.msg.extendedheader->ctid, "CT01", DLT_ID_SIZE);
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
    log_until_hard_limit_reached(apps[2]);
    EXPECT_FALSE(trace_load_keep_message(apps[2], _trace_load_send_size, &daemon, &daemon_local, 0));
    // Context logging should still be possible
    memcpy(daemon_local.msg.extendedheader->ctid, "CT01", DLT_ID_SIZE);
    EXPECT_TRUE(trace_load_keep_message(apps[2], _trace_load_send_size, &daemon, &daemon_local, 0));

    // Test not configured context
    memcpy(daemon_local.msg.extendedheader->ctid, "CTXX", DLT_ID_SIZE);
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
