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
#file        : dlt_logstorage_test.sh
#
#Descriptiom : Smoke testing for logstorage feature of DLT
#
#Author      : Onkar Palkar
#Email       : onkar.palkar@wipro.com
#
#History : 8/6/2016
################################################################################

#
# Function:      -cleanup()
#
# Description    -Delete the tmpDltLog folder if it already present
#
# Return         -Zero on success
#                -Non zero on failure
#
cleanup()
{
    pathTmp=/tmp
    folderName=tmpDltLog
    ls $pathTmp | grep $folderName > /dev/null
    if [ $? -eq '0' ]
    then
        rm -rf $pathTmp/$folderName
    fi
    pidof dlt-daemon > /dev/null
    if [ $? -eq '0' ]
    then
        killall dlt-daemon
        if [ $? -ne '0' ]
        then
            echo "Failed to kill dlt-daemon"
            return 1
        fi
    fi
    return 0
}
#
# Function:     -setup()
#
# Description   -Create tmpDltLog folder
#               -Add dlt.conf file in the tmpDltLog folder
#               -Add dlt_logstorage.conf file in the tmpDltLog folder
#
# Return        -Zero on success
#               -Non zero on failure
#
setup()
{
    which dlt-daemon > /dev/null
    if [ $? -ne '0' ]
    then
        echo "dlt-daemon not available"
        return 1
    fi
    which dlt-example-user > /dev/null
    if [ $? -ne '0' ]
    then
        echo "dlt-example-user not available"
        return 1
    fi
    which dlt-convert > /dev/null
    if [ $? -ne '0' ]
    then
        echo "dlt-convert not available"
        return 1
    fi
    mkdir $pathTmp/$folderName
    if [ $? -ne '0' ]
    then
        echo "Error while creating folder tmpDltLog"
        return 1
    fi
    touch $pathTmp/$folderName/dlt.conf
    if [ $? -ne '0' ]
    then
        echo "Error while creating dlt.conf file"
        return 1
    fi
    echo "SendContextRegistration = 1" >>$pathTmp/$folderName/dlt.conf
    echo "ECUId = ECU1" >>$pathTmp/$folderName/dlt.conf
    echo "SharedMemorySize = 100000" >>$pathTmp/$folderName/dlt.conf
    echo "LoggingMode = 0" >>$pathTmp/$folderName/dlt.conf
    echo "LoggingLevel = 6" >>$pathTmp/$folderName/dlt.conf
    echo "LoggingFilename = /tmp/dlt.log" >>$pathTmp/$folderName/dlt.conf
    echo "TimeOutOnSend = 4" >>$pathTmp/$folderName/dlt.conf
    echo "RingbufferMinSize = 500000" >>$pathTmp/$folderName/dlt.conf
    echo "RingbufferMaxSize = 10000000" >>$pathTmp/$folderName/dlt.conf
    echo "RingbufferStepSize = 500000" >>$pathTmp/$folderName/dlt.conf
    echo "ControlSocketPath = /tmp/dlt-ctrl.sock" >>$pathTmp/$folderName/dlt.conf
    echo "OfflineLogstorageMaxDevices = 2" >>$pathTmp/$folderName/dlt.conf
    echo "OfflineLogstorageDirPath = $pathTmp/$folderName" >>$pathTmp/$folderName/dlt.conf
    echo "OfflineLogstorageTimestamp = 1" >>$pathTmp/$folderName/dlt.conf
    echo "OfflineLogstorageDelimiter = _" >>$pathTmp/$folderName/dlt.conf
    echo "OfflineLogstorageMaxCounter = 999" >>$pathTmp/$folderName/dlt.conf
    echo "OfflineLogstorageCacheSize = 30000" >>$pathTmp/$folderName/dlt.conf
    touch $pathTmp/$folderName/dlt_logstorage.conf
    if [ $? -ne '0' ]
    then
        echo "Error while creating dlt_logstorage.conf file"
        return 1
    fi
    echo "[FILTER1]" >>$pathTmp/$folderName/dlt_logstorage.conf
    echo "LogAppName=LOG" >>$pathTmp/$folderName/dlt_logstorage.conf
    echo "ContextName=TEST" >>$pathTmp/$folderName/dlt_logstorage.conf
    echo "LogLevel=DLT_LOG_INFO" >>$pathTmp/$folderName/dlt_logstorage.conf
    echo "File=Test" >>$pathTmp/$folderName/dlt_logstorage.conf
    echo "FileSize=10000" >>$pathTmp/$folderName/dlt_logstorage.conf
    echo "NOFiles=1" >>$pathTmp/$folderName/dlt_logstorage.conf
    return 0
}
#
# Function:     -startDaemonAndApp()
#
# Description   -Kill daemon if it is already running
#               -Start dlt-daemon
#               -Start dlt-example-user
#
# Return        -Zero on success
#               -Non zero on failure
#
startDaemonAndApp()
{
    dlt-daemon -c $pathTmp/$folderName/dlt.conf -d > /dev/null
    dlt-example-user test_msg > /dev/null
    return 0
}
#
# Function:     -verifyTest()
#
# Description   -Verify count of messages
#
# Return        -Zero on success
#               -Non zero on failure
#
verifyTest()
{
    ls $pathTmp/$folderName | grep .dlt > /dev/null
    if [ $? -ne '0' ]
    then
        echo "Log file is not present"
        return 1
    fi
    msgCount=`dlt-convert -c $pathTmp/$folderName/*.dlt`
    if [ $? -ne '0' ]
    then
        echo "Error while reading count in log file"
        return 1
    fi
    echo $msgCount | grep 10 > /dev/null
    if [ $? -ne '0' ]
    then
        echo "Message count is incorrect"
        return 1
    fi
    return 0
}
########################################################################################
#main function
########################################################################################
cleanup
setup
if [ $? -ne '0' ]
then
    echo "Error in function setup()"
    cleanup
    return 1
fi
startDaemonAndApp
if [ $? -ne '0' ]
then
    echo "Error in function startDaemonAndApp()"
    cleanup
    return 1
fi
verifyTest
if [ $? -eq '0' ]
then
    echo "Test Passed"
else
    echo "Test Failed"
fi
cleanup
