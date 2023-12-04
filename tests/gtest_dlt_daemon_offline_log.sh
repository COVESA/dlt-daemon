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
    rm -rf $tmpPath/Test*.dlt
    rm -rf $tmpPath/*_dlt_logstorage.conf
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

    touch $tmpPath/pair_dlt_logstorage.conf
    if [ $? -eq '1' ]
    then
        echo "Error in creating pair_dlt_logstorage.conf file"
        return 1
    fi
    echo "[FILTER1]" >>$tmpPath/pair_dlt_logstorage.conf
    echo "LogAppName=NEGF,POSF" >>$tmpPath/pair_dlt_logstorage.conf
    echo "ContextName=TS1" >>$tmpPath/pair_dlt_logstorage.conf
    echo "ExcludedLogAppName=NEGF" >>$tmpPath/pair_dlt_logstorage.conf
    echo "ExcludedContextName=TS1" >>$tmpPath/pair_dlt_logstorage.conf
    echo "LogLevel=DLT_LOG_ERROR" >>$tmpPath/pair_dlt_logstorage.conf
    echo "File=Test" >>$tmpPath/pair_dlt_logstorage.conf
    echo "FileSize=1000000" >>$tmpPath/pair_dlt_logstorage.conf
    echo "NOFiles=1" >>$tmpPath/pair_dlt_logstorage.conf

    touch $tmpPath/only_ctid_dlt_logstorage.conf
    if [ $? -eq '1' ]
    then
        echo "Error in creating only_ctid_dlt_logstorage.conf file"
        return 1
    fi
    echo "[FILTER1]" >>$tmpPath/only_ctid_dlt_logstorage.conf
    echo "LogAppName=NEGF" >>$tmpPath/only_ctid_dlt_logstorage.conf
    echo "ContextName=TS1" >>$tmpPath/only_ctid_dlt_logstorage.conf
    echo "ExcludedContextName=TS1,TS2" >>$tmpPath/only_ctid_dlt_logstorage.conf
    echo "LogLevel=DLT_LOG_ERROR" >>$tmpPath/only_ctid_dlt_logstorage.conf
    echo "File=Test" >>$tmpPath/only_ctid_dlt_logstorage.conf
    echo "FileSize=1000000" >>$tmpPath/only_ctid_dlt_logstorage.conf
    echo "NOFiles=1" >>$tmpPath/only_ctid_dlt_logstorage.conf

    touch $tmpPath/only_apid_dlt_logstorage.conf
    if [ $? -eq '1' ]
    then
        echo "Error in creating only_apid_dlt_logstorage.conf file"
        return 1
    fi
    echo "[FILTER1]" >>$tmpPath/only_apid_dlt_logstorage.conf
    echo "LogAppName=NEGF" >>$tmpPath/only_apid_dlt_logstorage.conf
    echo "ContextName=TS1" >>$tmpPath/only_apid_dlt_logstorage.conf
    echo "ExcludedLogAppName=NEGF,POSF" >>$tmpPath/only_apid_dlt_logstorage.conf
    echo "LogLevel=DLT_LOG_ERROR" >>$tmpPath/only_apid_dlt_logstorage.conf
    echo "File=Test" >>$tmpPath/only_apid_dlt_logstorage.conf
    echo "FileSize=1000000" >>$tmpPath/only_apid_dlt_logstorage.conf
    echo "NOFiles=1" >>$tmpPath/only_apid_dlt_logstorage.conf

    touch $tmpPath/abnormal_dlt_logstorage.conf
    if [ $? -eq '1' ]
    then
        echo "Error in creating abnormal_dlt_logstorage.conf file"
        return 1
    fi
    echo "[FILTER1]" >>$tmpPath/abnormal_dlt_logstorage.conf
    echo "LogAppName=NEGF" >>$tmpPath/abnormal_dlt_logstorage.conf
    echo "ContextName=TS1" >>$tmpPath/abnormal_dlt_logstorage.conf
    echo "ExcludedLogAppName=NEGF,POSF" >>$tmpPath/abnormal_dlt_logstorage.conf
    echo "ExcludedContextName=TS1,TS2" >>$tmpPath/abnormal_dlt_logstorage.conf
    echo "LogLevel=DLT_LOG_ERROR" >>$tmpPath/abnormal_dlt_logstorage.conf
    echo "File=Test" >>$tmpPath/abnormal_dlt_logstorage.conf
    echo "FileSize=1000000" >>$tmpPath/abnormal_dlt_logstorage.conf
    echo "NOFiles=1" >>$tmpPath/abnormal_dlt_logstorage.conf
    return 0
}
#main function
########################################################################################
cleanup
setupTest
