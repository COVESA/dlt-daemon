#!/bin/sh
########################################################################################
#file            : g_test_dlt_daemon_logstorage.sh
#
#Description     : logstorage unit test preparation
#
#Author Name     : Onkar Palkar
#Email Id        : onkar.palkar@wipro.com
#
#History         : Last modified date : 02/09/2016
########################################################################################
#
# Function:    -cleanup()
#
# Description    -Delete dlt_logstorage.conf file from tmp folder if present
#
# Return    -Zero on success
#        -Non zero on failure
#

cleanup()
{
    tmpPath=/tmp
    cd $tmpPath
    rm -rf $tmpPath/dlt_logstorage.conf
    return 0
}

#
# Function:     -setupTest()
#
# Description   -create logstorage.conf file
#
# Return        -Zero on success
#               -Non zero on failure
#
setupTest()
{
    touch $tmpPath/dlt_logstorage.conf
    if [ $? -eq '1' ]
    then
        echo "Error in creating dlt_logstorage.conf file"
        return 1
    fi
    echo "[FILTER1]" >>$tmpPath/dlt_logstorage.conf
    echo "LogAppName=DLST" >>$tmpPath/dlt_logstorage.conf
    echo "ContextName=.*" >>$tmpPath/dlt_logstorage.conf
    echo "LogLevel=DLT_LOG_ERROR" >>$tmpPath/dlt_logstorage.conf
    echo "File=Test" >>$tmpPath/dlt_logstorage.conf
    echo "FileSize=1000000" >>$tmpPath/dlt_logstorage.conf
    echo "NOFiles=1" >>$tmpPath/dlt_logstorage.conf
    return 0
}
#main function
########################################################################################
cleanup
setupTest
