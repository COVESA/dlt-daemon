#!/bin/sh
################################################################################
# @licence make begin@
# SPDX license identifier: MPL-2.0
#
# Copyright (C) 2016, Advanced Driver Information Technology
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
# @licence end@
################################################################################

################################################################################
#file            : g_test_dlt_daemon_gateway.sh
#
#Description     : gateway unit test preparation
#
#Author Name     : Onkar Palkar
#Email Id        : onkar.palkar@wipro.com
#
#History         : Last modified date : 29/07/2016
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
ipaddr=127.0.0.1

cleanup()
{
    tmpPath=/tmp
    cd $tmpPath
    pidof dlt-daemon > /dev/null
    if [ $? -eq '0' ]
    then
        killall dlt-daemon
        if [ $? -eq '1' ]
        then
            echo "Failed to kill daemons"
            return 1
        fi
    fi
    rm $tmpPath/dlt.conf
    rm $tmpPath/idlt_gateway.conf
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
    echo "ECUId = ECU1" >>$tmpPath/dlt.conf
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
    echo "GatewayConfigFile = /tmp/$tmpFolder/dlt_gateway.conf" >>$tmpPath/dlt.conf
    touch $tmpPath/dlt_gateway.conf
    if [ $? -eq '1' ]
    then
        echo "Error in creating dlt_gateway file"
        return 1
    fi
    echo "[PassiveNode1]" >>$tmpPath/dlt_gateway.conf
    echo "IPaddress=$ipaddr">>$tmpPath/dlt_gateway.conf
    echo "Port=3495" >>$tmpPath/dlt_gateway.conf
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
#
# Return        -Zero on success
#               -Non zero on failure
#
startDaemons()
{
    dlt-daemon -p 3495
    return 0
}
#main function
########################################################################################
cleanup
if [ $? -ne '0' ]
then
return 1
fi
setupTest
if [ $? -ne '0' ]
then
return 1
fi
startDaemons
