
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

extern "C" void usage();

// Test Fixture for usage()
class UsageTest : public ::testing::Test {
    protected:
        std::stringstream output;
        UsageMock mock;
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
    mock.usage();  // Call the function being tested

    std::string outputStr = output.str();
    EXPECT_NE(outputStr.find("DLT Daemon Version 2.18.0"), std::string::npos);
}

TEST_F(UsageTest, PrintsUsageMessage) {
    mock.usage();  // Call the function being tested

    std::string outputStr = output.str();
    EXPECT_NE(outputStr.find("Usage: dlt-daemon [options]"), std::string::npos);
}

// Test if usage() prints the correct options
TEST_F(UsageTest, PrintsOptions) {
    mock.usage();
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