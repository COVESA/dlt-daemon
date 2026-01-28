#!/bin/sh
################################################################################
# SPDX license identifier: MPL-2.0
#
# Copyright (C) 2016, Advanced Driver Information Technology
# This code is developed by Advanced Driver Information Technology.
# Copyright of Advanced Driver Information Technology, Bosch and DENSO.
#
# This file is part of COVESA Project DLT - Diagnostic Log and Trace.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License (MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.covesa.org/.
################################################################################
################################################################################
#file            : g_test_dlt_daemon_gateway.sh
#
#Description     : gateway unit test preparation
#
#Author Name     : Onkar Palkar
#                  Jeevan Ramakant Nagvekar
#Email Id        : onkar.palkar@wipro.com
#                  jeevan.nagvekar1@wipro.com
#
#History         : Last modified date : 31/07/2018
# Environment
BASEDIR="$(realpath "$(dirname "$0")")"
DLT_DAEMON_BUILD_DIR="${BASEDIR}/../src/daemon"
ipaddr="::1"
tmpPath="/tmp"
pidFile="$tmpPath/dlt.pid"
################################################################################
# Function:    -getOSname()
#
# Description  -Retrieves OS name
#
getOSname()
{
    OS="$(uname)"
}
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
    DLT_DAEMON="$DLT_UT_DAEMON_PATH"
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
#
# Function:    -cleanup()
#
# Description    -Delete the dlt_test folder if it already present
#        -Check weather required binaries are avaiable or not
#        -Restore dlt_gateway.conf file
#
# Return    -Zero on success
#        -Non zero on failure
#
cleanup()
{
    cd "$tmpPath" || return 1
    rm -f $tmpPath/dlt_ecu*.conf
    rm -f $tmpPath/dlt_gateway.conf
    rm -f /dev/shm/dlt-shm
    rm -f /dev/shm/sem.dlt-shm
    rm -f /dev/shm/dlt-shm-passive
    rm -f /dev/shm/sem.dlt-shm-passive
    if [ "$OS" = "QNX" ]
    then
        PIDOF()
        {
            if [ "$(slay -p "$1" > /dev/null)" ]; then
                return 0
            else
                return 1
            fi
        }
        KILLALL()
        {
            if [ "$(slay -9 -f "$1" > /dev/null)" ]; then
                return 0
            else
                return 1
            fi
        }
    fi
    if ! killPids "$pidFile"; then
        help
        return 1
    fi
    return 0
}
#
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
    if [ "$OS" = "QNX" ]; then
        MESSAGE_FILTER_CONF="/etc/dlt_message_filter.conf"
    else
        MESSAGE_FILTER_CONF="/etc/dlt_message_filter_ald.conf"
    fi
    if [ ! -f $MESSAGE_FILTER_CONF ]; then
        MESSAGE_FILTER_CONF=$tmpPath/dlt_message_filter.conf
    fi
    if [ "$(touch $tmpPath/dlt_ecu1.conf)" ]; then
        echo "Error in creating dlt_ecu1.conf file"
        return 1
    fi
    ecu1_conf_file="$tmpPath/dlt_ecu1.conf"
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
    } >> "$ecu1_conf_file"
    if [ "$(cp $tmpPath/dlt_ecu1.conf $tmpPath/dlt_ecu2.conf)" ]; then
        echo "Error in creating dlt_ecu2.conf file"
        return 1
    fi
    ecu2_conf_file="$tmpPath/dlt_ecu2.conf"
    sed -i -e 's/ECU1/ECU2/g' "$ecu2_conf_file"
    {
        echo "GatewayMode = 1";
        echo "ControlSocketPath = /tmp/dlt-ctrl.sock";
        echo "GatewayConfigFile = /tmp/dlt_gateway.conf";
    } >> "$ecu2_conf_file"
    if [ "$(touch $tmpPath/dlt_gateway.conf)" ]; then
        echo "Error in creating dlt_gateway.conf file"
        return 1
    fi
    {
        if [ $# -eq 1 ] && [ "$1" = "1" ]; then
                echo "[General]";
                echo "Interval=1";
        fi
        echo "[PassiveNode1]";
        if [ -n "$DLT_IPv6_LO" ]; then
            echo "IPaddress=$DLT_IPv6_LO";
        else
            echo "IPaddress=$ipaddr";
        fi
        echo "Port=3495";
        echo "EcuID=ECU1";
        echo "Connect=OnStartup";
        echo "Timeout=10";
        echo "SendControl=0x03,0x13";
        echo "SendSerialHeader=0";
        echo "NOFiles=1";
     } >> $tmpPath/dlt_gateway.conf
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
    if [ "$(touch $tmpPath/dlt_passive.conf)" ]; then
        echo "Error in creating dlt_passive.conf file"
        return 1
    fi
    {
        echo "MessageFilterConfigFile = $tmpPath/dlt_message_filter.conf";
        echo "ControlSocketPath = $tmpPath/dlt-ctrl-passive.sock";
    } >> $tmpPath/dlt_passive.conf
    return 0
}
#
# Function:     -startDaemons()
#
# Description   -Start dlt-daemon as passive node
#               -Start dlt-daemon as gateway node
#
# Return        -Zero on success
#               -Non zero on failure
#
startDaemons()
{
    DLT_PASSIVE_SHM_NAME=""
    $DLT_DAEMON_RUN -p 3495 -c "$ecu1_conf_file" >/dev/null 2>&1 &
    if [ $?  -eq '1' ]; then
        echo "ERROR: Cannot run Host Daemon"
        return 1
    else
        echo "INFO: Run Host Daemon successfully"
        echo $! >> "$pidFile"
    fi
    sleep 0.1
    # Check if the dlt shm file exist (DLT_SHM_ENABLE=ON)
    if [ -f /dev/shm/dlt-shm ]; then
        DLT_PASSIVE_SHM_NAME="-s dlt-shm-passive"
    fi
    $DLT_DAEMON_RUN -c "$ecu2_conf_file" "$DLT_PASSIVE_SHM_NAME" >/dev/null 2>&1 &
    if [ $?  -eq '1' ]; then
        echo "ERROR: Cannot run Target Daemon"
        return 1
    else
        echo "INFO: Run Target Daemon successfully"
        echo $! >> "$pidFile"
    fi
    return 0
}
#
# Function:     -checkDaemonStart
#
# Description   -Check if dlt-daemon instances started successfully
#
checkDaemonStart()
{
    BASE="$(basename "$DLT_DAEMON")"
    if [ "$OS" = "QNX" ]; then
        slay -p "$BASE" > /dev/null
        total=$?
        if [ "$total" -ne '2' ]; then
            echo "Initialization of dlt-daemon instances failed"
            return 1
        fi
    else
        pid_count=$(wc -l < "$pidFile")
        echo "Setup total daemons: $pid_count"
        if [ "$pid_count" -ne '2' ]; then
            echo "Initialization of dlt-daemon instances failed"
            return 1
        fi
    fi
    return 0
}
help()
{
    echo "Usage: "
    echo "sh ./gtest_dlt_daemon_gateway.sh"
}
executeTests()
{
    echo "SETUP DONE"
    echo "Execute: gtest_dlt_daemon_gateway unit test"
    return 0
}
#main function
########################################################################################
getOSname
getDltDaemonPath
echo "Cleaning up dlt-daemon instances"
  if ! cleanup; then
      help
      exit 1
  fi
echo "Initializing test"
if [ $# -eq 1 ] && [ "$1" = "-w" ]; then
    echo "Including General section in dlt_gateway.conf"
    if ! setupTest 1; then
        help
        exit 1
    fi
else
    if ! setupTest; then
        help
        exit 1
    fi
fi
echo "Restarting dlt-daemons"
if ! startDaemons; then
    help
    exit 1
fi
echo "Checking dlt-daemons"
if ! checkDaemonStart; then
    help
    exit 1
fi
if ! executeTests; then
    help
    exit 1
fi
exit 0
