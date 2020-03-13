#!/bin/bash
################################################################################
# SPDX license identifier: MPL-2.0
#
# Copyright (C) 2019, Advanced Driver Information Technology
# This code is developed by Advanced Driver Information Technology.
# Copyright of Advanced Driver Information Technology, Bosch and DENSO.
#
# This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License (MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.genivi.org/.
################################################################################

################################################################################
#file            : gtest_dlt_all.sh
#
#Description     : Run all unit tests in Travis CI
#
#Author Name     : Saya Sugiura
################################################################################

function gtest_run_test()
{
    LOG="../.travis/$1.log"

    # Execute unit test
    ./$1 > $LOG

    # Check for result
    grep "FAILED TEST" $LOG
    if [ $? -eq 0 ]
    then
        cat $LOG
        exit 1
    fi
    echo "$1 passed"
}

pushd tests > /dev/null

gtest_run_test gtest_dlt_common

gtest_run_test gtest_dlt_user

gtest_run_test gtest_dlt_daemon_common

gtest_run_test gtest_dlt_daemon_event_handler

# Without General section in dlt_gateway.conf
./gtest_dlt_daemon_gateway.sh > /dev/null
gtest_run_test gtest_dlt_daemon_gateway

# With General section in dlt_gateway.conf
./gtest_dlt_daemon_gateway.sh -w > /dev/null
gtest_run_test gtest_dlt_daemon_gateway

./gtest_dlt_daemon_logstorage.sh > /dev/null
gtest_run_test gtest_dlt_daemon_offline_log

gtest_run_test dlt_env_ll_unit_test

popd > /dev/null
