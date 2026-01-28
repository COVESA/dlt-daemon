#!/bin/sh
################################################################################
# SPDX license identifier: MPL-2.0
#
# Copyright (C) 2023 Advanced Driver Information Technology.
# This code is developed by Advanced Driver Information Technology.
# Copyright of Advanced Driver Information Technology and Bosch.
#
# This file is part of COVESA Project DLT - Diagnostic Log and Trace.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License (MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see https://covesa.global.
################################################################################
################################################################################
#file            : gtest_dlt_user.sh
#
#Description     : user unit test preparation
#
#Author Name     : Luu Quang Minh
#Email Id        : <minh.luuquang@vn.bosch.com>
# Environment
BASEDIR="$(realpath "$(dirname "$0")")"
DLT_DAEMON_BUILD_DIR="${BASEDIR}/../src/daemon"
tmpPath="/tmp"
pidFile="$tmpPath/dlt.pid"
################################################################################
# Function:    -getDltDaemonPath()
#
# Description  -Retrieves path to dlt-daemon
#
getDltDaemonPath()
{
    if [ -z "${DLT_UT_DAEMON_PATH}" ]; then
        echo "WARNING: env variable DLT_UT_DAEMON_PATH is not set, take the default value."
        DLT_UT_DAEMON_PATH="${DLT_DAEMON_BUILD_DIR}/dlt-daemon"
        echo "INFO: env variable DLT_UT_DAEMON_PATH = " "${DLT_UT_DAEMON_PATH}"
    fi
    DLT_DAEMON="${DLT_UT_DAEMON_PATH}"
    if [ -n "$QEMU" ]; then
        DLT_DAEMON_RUN="$QEMU $DLT_DAEMON"
    else
        DLT_DAEMON_RUN="$DLT_DAEMON"
    fi
    echo "INFO: env variable DLT_DAEMON_RUN = " "${DLT_DAEMON_RUN}"
}
################################################################################
#
# Function:    -killPids()
#
# Description    -kill all remaining pids of dlt processes in stored file
#
killPids() {
    PID_FILE="$1"
    flag=true
    if [ ! -f "$PID_FILE" ]; then
        echo "Warning: PID file not found: $PID_FILE"
        flag=false
    fi
    if [ "$flag" = true ]; then
        while IFS= read -r pid; do
            [ -z "$pid" ] && continue
            if kill -0 "$pid" 2>/dev/null; then
                kill -9 "$pid"
                echo "Killed process with PID: $pid"
            else
                echo "PID $pid is not running or already terminated"
            fi
        done < "$PID_FILE"
        rm -f "$PID_FILE"
        echo "All PIDs killed, and the PID file has been removed."
    fi
    return 0
}
################################################################################
# Function:      -cleanup()
#
# Description
#
# Return         -Zero on success
#                -Non zero on failure
#
cleanup()
{
    cd "$tmpPath" || return 1
    rm -f $tmpPath/dlt_user.conf
    if  ! killPids "$pidFile"; then
        help
        return 1
    fi
    return 0
}
################################################################################
# Function:     -setupTest()
#
# Description   -Create gateway folder
#               -Create and add dlt_ecu*.conf and dlt_gateway.conf file in the  gateway folder
#
# Return        -Zero on success
#               -Non zero on failure
#
setupTest()
{
    if [ "$(touch "$pidFile")" ]; then
        echo "Error in creating dlt.pid file"
        return 1
    fi
    MESSAGE_FILTER_CONF="/etc/dlt_message_filter_ald.conf"
    if [ ! -f $MESSAGE_FILTER_CONF ]; then
        MESSAGE_FILTER_CONF=$tmpPath/dlt_message_filter.conf
    fi
    if [ "$(touch $tmpPath/dlt_message_filter.conf)" ]; then
        echo "Error in creating dlt_message_filter.conf file"
        return 1
    fi
    {
        echo "[General]";
        echo "Name = Genivi-LogMode";
        echo "DefaultLevel = 100";
        echo "[Filter1]";
        echo "Name            = Both";
        echo "Level           = 100";
        echo "Clients         = *";
        echo "ControlMessages = *";
        echo "Injections      = *";
    } >> $tmpPath/dlt_message_filter.conf
    if [ "$(touch $tmpPath/dlt_user.conf)" ]; then
        echo "Error in creating dlt_user.conf file"
        return 1
    fi
    conf_file="$tmpPath/dlt_user.conf"
    {
        echo "SendContextRegistration = 1";
        echo "ECUId = ECU1";
        echo "SharedMemorySize = 100000";
        echo "LoggingMode = 0";
        echo "LoggingLevel = 6";
        echo "LoggingFilename = /tmp/dlt.log";
        echo "TimeOutOnSend = 4";
        echo "RingbufferMinSize = 500000";
        echo "RingbufferMaxSize = 10000000";
        echo "RingbufferStepSize = 500000";
        echo "MessageFilterConfigFile = $MESSAGE_FILTER_CONF";
    } >> "$conf_file"
    return 0
}
################################################################################
# Function:     -startDaemon()
#
# Description   -Kill daemon if it is already running
#               -Start dlt-daemon
#
# Return        -Zero on success
#               -Non zero on failure
#
startDaemon()
{
    $DLT_DAEMON_RUN -c "$conf_file" >/dev/null 2>&1 &
    if [ $?  -eq '1' ]; then
        echo "ERROR: Cannot run Target Daemon"
        return 1
    else
        echo "INFO: Run Target Daemon successfully"
        echo $! >> "$pidFile"
    fi
    return 0
}
################################################################################
# Function:     -checkDaemonStart
#
# Description   -Check if dlt-daemon instances started successfully
#
checkDaemonStart()
{
    pid_count=$(wc -l < "$pidFile")
    echo "Setup total daemons: $pid_count"
    if [ "$pid_count" -ne '1' ]; then
        echo "Initialization of dlt-daemon instances failed"
        return 1
    fi
    return 0
}
help()
{
    echo "Usage: "
    echo "sh ./gtest_dlt_user.sh"
}
executeTests()
{
    echo "SETUP DONE"
    echo "Execute: gtest_dlt_user unit test"
    return 0
}
########################################################################################
#main function
########################################################################################
getDltDaemonPath
echo "Cleaning up dlt-daemon instances"
if ! cleanup; then
    help
    exit 1
fi
echo "Initializing test"
if ! setupTest; then
    help
    exit 1
fi
echo "Restarting dlt-daemon"
if ! startDaemon; then
    help
    exit 1
fi
echo "Checking dlt-daemon"
if ! checkDaemonStart; then
    help
    exit 1
fi
if ! executeTests; then
    help
    exit 1
fi
exit 0