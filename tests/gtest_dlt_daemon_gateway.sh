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

ipaddr=127.0.0.1

################################################################################
# Function:    -getOSname()
#
# Description  -Retrieves OS name
#
getOSname()
{
    OS="`uname`"
}

################################################################################
# Function:    -getDltDaemonPath()
#
# Description  -Retrieves path to dlt-daemon
#
getDltDaemonPath()
{
    if [ -z "${DLT_UT_DAEMON_PATH}" ]; then
        echo "WARNNG: env variable DLT_UT_DAEMON_PATH is not set"
        DLT_DAEMON=`which dlt-daemon`
    else
        DLT_DAEMON="${DLT_UT_DAEMON_PATH}"
    fi
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
    tmpPath=/tmp

    if [ "$OS" = "QNX" ]
    then
        PIDOF()
        {
            slay -p $1 > /dev/null
            if [ $? -eq '0' ]
            then
                return 1
            else
                return 0
            fi
        }
        KILLALL()
        {
            slay -9 -f $1 > /dev/null
            if [ $? -eq '0' ]
            then
                return 1
            else
                return 0
            fi
        }
    else
        PIDOF()
        {
            pidof $1 > /dev/null
            return $?
        }
        KILLALL()
        {
            killall $1 > /dev/null
            return $?
        }
    fi

    cd $tmpPath

    BASE=`basename $DLT_DAEMON`
    PIDOF $BASE
    if [ $? -eq '0' ]
    then
        KILLALL $BASE
        if [ $? -eq '1' ]
        then
            echo "Failed to kill daemons"
            return 1
        fi
    fi

    rm -f $tmpPath/dlt.conf
    rm -f $tmpPath/dlt_gateway.conf
    rm -f /dev/shm/dlt-shm
    rm -f /dev/shm/sem.dlt-shm
    rm -f /dev/shm/dlt-shm-passive
    rm -f /dev/shm/sem.dlt-shm-passive
    return 0
}
#
# Function:     -setupTest()
#
# Description   -Create gateway folder
#               -Create and add dlt.conf and dlt_gateway.conf file in the  gateway folder
#
# Return        -Zero on success
#               -Non zero on failure
#
setupTest()
{
    which dlt-daemon > /dev/null
    if [ $? -eq '1' ]
    then
        echo "dlt-daemon is not available"
        return 1
    fi
    touch $tmpPath/dlt.conf
    if [ $? -eq '1' ]
    then
        echo "Error in creating dlt.conf file"
        return 1
    fi
    echo "SendContextRegistration = 1" >>$tmpPath/dlt.conf
    echo "ECUId = ECU2" >>$tmpPath/dlt.conf
    echo "GatewayMode = 1" >>$tmpPath/dlt.conf
    echo "SharedMemorySize = 100000" >>$tmpPath/dlt.conf
    echo "LoggingMode = 0" >>$tmpPath/dlt.conf
    echo "LoggingLevel = 6" >>$tmpPath/dlt.conf
    echo "LoggingFilename = /tmp/dlt.log" >>$tmpPath/dlt.conf
    echo "TimeOutOnSend = 4" >>$tmpPath/dlt.conf
    echo "RingbufferMinSize = 500000" >>$tmpPath/dlt.conf
    echo "RingbufferMaxSize = 10000000" >>$tmpPath/dlt.conf
    echo "RingbufferStepSize = 500000" >>$tmpPath/dlt.conf
    echo "ControlSocketPath = /tmp/dlt-ctrl.sock" >>$tmpPath/dlt.conf
    echo "GatewayConfigFile = /tmp/dlt_gateway.conf" >>$tmpPath/dlt.conf
    touch $tmpPath/dlt_gateway.conf
    if [ $? -eq '1' ]
    then
        echo "Error in creating dlt_gateway file"
        return 1
    fi

    if [ $# -eq 1 ] && [ "$1" = "1" ]
    then
        echo "[General]" >>$tmpPath/dlt_gateway.conf
        echo "Interval=1">>$tmpPath/dlt_gateway.conf
    fi

    echo "[PassiveNode1]" >>$tmpPath/dlt_gateway.conf
    echo "IPaddress=$ipaddr">>$tmpPath/dlt_gateway.conf
    echo "Port=3490" >>$tmpPath/dlt_gateway.conf
    echo "EcuID=ECU1" >>$tmpPath/dlt_gateway.conf
    echo "Connect=OnStartup" >>$tmpPath/dlt_gateway.conf
    echo "Timeout=10" >>$tmpPath/dlt_gateway.conf
    echo "SendControl=0x03,0x13" >>$tmpPath/dlt_gateway.conf
    echo "SendSerialHeader=0" >>$tmpPath/dlt_gateway.conf
    echo "NOFiles=1" >>$tmpPath/dlt_gateway.conf

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
    tmpPath=/tmp
    dlt-daemon -d
    sleep 1

    # Check if the dlt shm file exist (DLT_SHM_ENABLE=ON)
    if [ -f /dev/shm/dlt-shm ]; then
        DLT_PASSIVE_SHM_NAME="-s dlt-shm-passive"
    fi

    dlt-daemon -d -p 3495 -c $tmpPath/dlt.conf $DLT_PASSIVE_SHM_NAME
    return 0
}

#
# Function:     -checkDaemonStart
#
# Description   -Check if dlt-daemon instances started successfully
#
checkDaemonStart()
{
    BASE=`basename $DLT_DAEMON`
    if [ "$OS" = "QNX" ]; then
        slay -p $BASE > /dev/null
        total=$?
    else
        total=`pgrep -c $BASE`
    fi

    if [ $total -ne '2' ]; then
        echo "Initialization of dlt-daemon instances failed"
        exit 1
    fi
}

help()
{
    echo "Usage: "
    echo "sh ./gtest_dlt_daemon_gateway.sh"
}

executeTests()
{
    echo "Execute: gtest_dlt_daemon_gateway unit test"
}

#main function
########################################################################################

getOSname

getDltDaemonPath

echo "Cleaning up dlt-daemon instances"
cleanup

if [ $? -ne '0' ]
then
  help
  exit
fi

echo "Initializing test"

if [ $# -eq 1 ] && [ "$1" = "-w" ]
then
    echo "Including General section in dlt_gateway.conf"
    setupTest 1
else
    setupTest
fi

if [ $? -ne '0' ]
then
  help
  exit
fi

echo "Restarting dlt-daemons"
startDaemons

checkDaemonStart

executeTests
