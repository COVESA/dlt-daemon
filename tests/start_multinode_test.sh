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
#file            : dlt_multinode_test.sh
#
#Description     : Smoke testing for multinode feature of DLT
#
#Author Name     : Onkar Palkar
#                  Jeevan Ramakant Nagvekar
#Email Id        : onkar.palkar@wipro.com
#                  jeevan.nagvekar1@wipro.com
#
#History         : 31/07/2018
################################################################################

ipaddr=127.0.0.1

#
# Function:    -getOSname()
#
# Description  -Retrieves OS name
#
getOSname()
{
    OS="`uname`"
}
#
# Function:    -cleanup()
#
# Description  -Delete the dlt_test folder if it already present
#              -Check weather required binaries are avaiable or not
#              -Restore dlt_gateway.conf file
#
# Return       -Zero on success
#              -Non zero on failure
#
cleanup()
{
    tmpPath=/tmp
    tmpFolder=dlt_test
    gatewayFolderName=gateway
    passiveFolderName=passive
    tmpPassiveDIR=tmpPassive
    tmpLogFile=log_multinode.txt
    tmpLogAsciFile=log_multinode_a.txt

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
    PIDOF "dlt-daemon"
    if [ $? -eq '0' ]
    then
        KILLALL "dlt-daemon"
        if [ $? -ne '0' ]
        then
            echo "Failed to kill daemons"
            return 1
        fi
    fi
    PIDOF "dlt-receive"
    if [ $? -eq '0' ]
    then
        KILLALL "dlt-receive"
        if [ $? -ne '0' ]
        then
            echo "Failed to kill dlt-receive"
            return 1
        fi
    fi
    PIDOF "dlt-convert"
    if [ $? -eq '0' ]
    then
        KILLALL "dlt-convert"
        if [ $? -ne '0' ]
        then
            echo "Failed to kill dlt-convert"
            return 1
        fi
    fi
    rm -rf $tmpPath/$tmpFolder >/dev/null
    return 0
}
#
# Function:     -setupTest()
#
# Description   -Create one gateway and passive folder
#        -Create and add dlt.conf and dlt_gateway.conf file in the  gateway folder
#        -Create and add dlt.conf file in the passive folder
#
# Return        -Zero on success
#               -Non zero on failure
#
setupTest()
{
    which dlt-daemon > /dev/null
    if [ $? -ne '0' ]
    then
        echo "dlt-daemon is not available"
        return 1
    fi
    which dlt-example-user > /dev/null
    if [ $? -ne '0' ]
    then
        echo "dlt-example-user is not available"
        return 1
    fi
    which dlt-receive > /dev/null
    if [ $? -ne '0' ]
    then
        echo "dlt-receive is not available"
        return 1
    fi
    mkdir -p $tmpPath/$tmpFolder
    if [ $? -ne '0' ]
        then
        echo "Error in creating dlt_test folder"
        return 1
    fi
    mkdir -p $tmpPath/$tmpFolder/$gatewayFolderName
    if [ $? -ne '0' ]
    then
        echo "Error in creating gateway folder"
        return 1
    fi
    touch $tmpPath/$tmpFolder/$gatewayFolderName/dlt.conf
    if [ $? -ne '0' ]
    then
        echo "Error in creating dlt.conf file"
        return 1
    fi
    echo "SendContextRegistration = 1" >>$tmpPath/$tmpFolder/$gatewayFolderName/dlt.conf
    echo "ECUId = ECU1" >>$tmpPath/$tmpFolder/$gatewayFolderName/dlt.conf
    echo "GatewayMode = 1" >>$tmpPath/$tmpFolder/$gatewayFolderName/dlt.conf
    echo "SharedMemorySize = 100000" >>$tmpPath/$tmpFolder/$gatewayFolderName/dlt.conf
    echo "LoggingMode = 0" >>$tmpPath/$tmpFolder/$gatewayFolderName/dlt.conf
    echo "LoggingLevel = 6" >>$tmpPath/$tmpFolder/$gatewayFolderName/dlt.conf
    echo "LoggingFilename = /tmp/dlt.log" >>$tmpPath/$tmpFolder/$gatewayFolderName/dlt.conf
    echo "TimeOutOnSend = 4" >>$tmpPath/$tmpFolder/$gatewayFolderName/dlt.conf
    echo "RingbufferMinSize = 500000" >>$tmpPath/$tmpFolder/$gatewayFolderName/dlt.conf
    echo "RingbufferMaxSize = 10000000" >>$tmpPath/$tmpFolder/$gatewayFolderName/dlt.conf
    echo "RingbufferStepSize = 500000" >>$tmpPath/$tmpFolder/$gatewayFolderName/dlt.conf
    echo "ControlSocketPath = /tmp/dlt-ctrl.sock" >>$tmpPath/$tmpFolder/$gatewayFolderName/dlt.conf
    echo "GatewayConfigFile = $tmpPath/$tmpFolder/$gatewayFolderName/dlt_gateway.conf" >> $tmpPath/$tmpFolder/$gatewayFolderName/dlt.conf
    touch $tmpPath/$tmpFolder/$gatewayFolderName/dlt_gateway.conf
    if [ $? -ne '0' ]
    then
        echo "Error in creating dlt_gateway file"
        return 1
    fi
    echo "[PassiveNode1]" >>$tmpPath/$tmpFolder/$gatewayFolderName/dlt_gateway.conf
    echo "IPaddress=$ipaddr">>$tmpPath/$tmpFolder/$gatewayFolderName/dlt_gateway.conf
    echo "Port=3495" >>$tmpPath/$tmpFolder/$gatewayFolderName/dlt_gateway.conf
    echo "EcuID=ECU2" >>$tmpPath/$tmpFolder/$gatewayFolderName/dlt_gateway.conf
    echo "Connect=OnStartup" >>$tmpPath/$tmpFolder/$gatewayFolderName/dlt_gateway.conf
    echo "Timeout=10" >>$tmpPath/$tmpFolder/$gatewayFolderName/dlt_gateway.conf
    echo "NOFiles=1" >>$tmpPath/$tmpFolder/$gatewayFolderName/dlt_gateway.conf
    mkdir -p $tmpPath/$tmpFolder/$passiveFolderName
        if [ $? -ne '0' ]
        then
        echo "Error in creating passive folder"
        return 1
    fi
    touch $tmpPath/$tmpFolder/$passiveFolderName/dlt.conf
    if [ $? -ne '0' ]
    then
        echo "Error in creating dlt.conf file"
        return 1
    fi
    echo "SendContextRegistration = 1" >>$tmpPath/$tmpFolder/$passiveFolderName/dlt.conf
    echo "ECUId = ECU2" >>$tmpPath/$tmpFolder/$passiveFolderName/dlt.conf
    echo "SharedMemorySize = 100000" >>$tmpPath/$tmpFolder/$passiveFolderName/dlt.conf
    echo "LoggingMode = 0" >>$tmpPath/$tmpFolder/$passiveFolderName/dlt.conf
    echo "LoggingLevel = 6" >>$tmpPath/$tmpFolder/$passiveFolderName/dlt.conf
    echo "LoggingFilename = /tmp/dlt.log" >>$tmpPath/$tmpFolder/$passiveFolderName/dlt.conf
    echo "TimeOutOnSend = 4" >>$tmpPath/$tmpFolder/$passiveFolderName/dlt.conf
    echo "RingbufferMinSize = 500000" >>$tmpPath/$tmpFolder/$passiveFolderName/dlt.conf
    echo "RingbufferMaxSize = 10000000" >>$tmpPath/$tmpFolder/$passiveFolderName/dlt.conf
    echo "RingbufferStepSize = 500000" >>$tmpPath/$tmpFolder/$passiveFolderName/dlt.conf
    echo "ControlSocketPath = /tmp/dlt-ctrl.sock" >>$tmpPath/$tmpFolder/$passiveFolderName/dlt.conf
    mkdir -p $tmpPath/$tmpFolder/$tmpPassiveDIR
    if [ $? -ne '0' ]
    then
        echo "Error while creating tempPassive folder"
        return 1
    fi
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
    dlt-daemon -c $tmpPath/$tmpFolder/$passiveFolderName/dlt.conf -p 3495 -t $tmpPath/$tmpFolder/$tmpPassiveDIR -d > /dev/null
    dlt-daemon -c $tmpPath/$tmpFolder/$gatewayFolderName/dlt.conf -p 3490 -d > /dev/null
    return 0
}
#
# Function:     -startExample()
#
# Description   -Start dlt-example-user on passive node
#
# Return        -Zero on success
#               -Non zero on failure
#
startExample()
{
    export DLT_PIPE_DIR=$tmpPath/$tmpFolder/$tmpPassiveDIR
    dlt-example-user MultiNodeTesting > /dev/null &
    return 0
}
#
# Function:     -starReceive()
#
# Description   -Start dlt-receive
#
# Return        -Zero on success
#               -Non zero on failure
#
startReceive()
{
    dlt-receive -o $tmpPath/$tmpFolder/$tmpLogFile localhost &
    return 0
}
#
# Function:     -verifyTest()
#
# Description   -Start dlt-convert
#               -check weather msg sent by passive node are available in logs or not
#
# Return        -Zero on success
#               -Non zero on failure
#
verifyTest()
{
    dlt-convert -a $tmpPath/$tmpFolder/$tmpLogFile > $tmpPath/$tmpFolder/$tmpLogAsciFile
    cat $tmpPath/$tmpFolder/$tmpLogAsciFile | grep -F "ECU2" > /dev/null

    if [ $? -eq '0' ]
    then
        return 0
    else
        return 1
    fi
}
#main function
########################################################################################
getOSname
cleanup
if [ $? -ne '0' ]
then
    echo "getIpAdd() failed"
    cleanup
    return 1
fi
setupTest
if [ $? -ne '0' ]
then
    echo "setupTest() failed"
    cleanup
    return 1
fi
startDaemons
if [ $? -ne '0' ]
then
    echo "startDaemons() failed"
    cleanup
    return 1
fi
startExample
if [ $? -ne '0' ]
then
    echo "startExample() failed"
    cleanup
    return 1
fi
#wait for 1 sec before starting dlt-receive to start dlt-example-user application properly
sleep 1
startReceive
if [ $? -ne '0' ]
then
    echo "startReceive() failed"
    cleanup
    return 1
fi
#Wait for 1 sec to collect messages sent by application at gateway
sleep 1
verifyTest
if [ $? -eq '0' ]
then
    echo "Test Passed"
else
    echo "Test Failed"
fi
cleanup
