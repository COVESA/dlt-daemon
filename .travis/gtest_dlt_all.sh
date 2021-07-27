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

    # Send all messsages and system errors to log file
    export LIBC_FATAL_STDERR_=1

    # Execute unit test
    { ./$1 ;} > $LOG 2>&1

    # Release
    export LIBC_FATAL_STDERR_=0

    # Check for result
    grep "FAILED TEST\|core dumped" $LOG
    if [ $? -eq 0 ]
    then
        cat $LOG
        echo "$1 failed"
        exit 1
    fi
    echo "$1 passed"
}

CTEST_OUTPUT_ON_FAILURE=1 make test

pushd tests > /dev/null

# Without General section in dlt_gateway.conf
./gtest_dlt_daemon_gateway.sh > /dev/null
gtest_run_test gtest_dlt_daemon_gateway

# With General section in dlt_gateway.conf
./gtest_dlt_daemon_gateway.sh -w > /dev/null
gtest_run_test gtest_dlt_daemon_gateway

./gtest_dlt_daemon_offline_log.sh > /dev/null
gtest_run_test gtest_dlt_daemon_offline_log

popd > /dev/null
