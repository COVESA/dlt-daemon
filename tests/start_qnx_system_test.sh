#!/bin/sh

################################################################################
# This software has been developed by Advanced Driver Information Technology.
# Copyright(c) 2021 Advanced Driver Information Technology GmbH,
# Advanced Driver Information Technology Corporation, Robert Bosch GmbH,
# Robert Bosch Car Multimedia GmbH and DENSO Corporation.
# All rights reserved.
################################################################################

################################################################################
# File:        start_qnx_system_test.sh
#
# Description: Test script to verify dlt-qnx-system using dlt-test-qnx-slogger
################################################################################

usage()
{
    cat <<EOM
Usage: ./start_qnx_system_test.sh [OPTION]...
  -h           Display help
  -v           Verbose mode
  -n count     Number of messages to be generated (Default: 10)
  -d delay     Milliseconds to wait between sending messages (Default: 500).
  -l length    Message payload length (Default: 100 bytes)
EOM

    exit 2
}

pr_verbose()
{
    VERBOSE_FUNC=$1
    VERBOSE_MSG=$2

    if [ "${VERBOSE}" -eq 1 ]; then
        echo ""
        echo "${VERBOSE_FUNC}: ${VERBOSE_MSG}"
    fi
}

################################################################################
# Function:    dlt_qnx_configure_setup
#
# Description: Setup configuration and json files for testing
################################################################################
#dlt_qnx_configure_setup()
#{
    # Replace parameter dlt-qnx-system.conf if necessary
    # Replace json value if necessary

    # Nothing to do as of now
#}

################################################################################
# Function:    dlt_qnx_clean_app
#
# Description: Clean up applications
################################################################################
dlt_qnx_clean_app()
{
    pr_verbose "$0" "Stop applications"

    slay -f dlt-test-qnx-slogger
    sleep "${SLEEP_TIME}"
    slay -f dlt-qnx-system
    sleep "${SLEEP_TIME}"
    slay -f dlt-receive
    sleep "${SLEEP_TIME}"
    rm -f "${DLT_RECEIVE_FILTER}"
    sleep "${SLEEP_TIME}"
}

################################################################################
# Function:    dlt_qnx_clean_log
#
# Description: Clean up logs
################################################################################
dlt_qnx_clean_log()
{
    pr_verbose "$0" "Delete logs"

    rm -f "${DLT_TEST_RESULT}"
}

################################################################################
# Function:    dlt_qnx_run_app
#
# Description: Start applications for testing
################################################################################
dlt_qnx_run_app()
{
    pr_verbose "$0" "Start dlt-qnx-system testing"

    # Prepare a text file to filter log messages by specific AppID and Ctx ID
    echo "QSYM QSLA" > "${DLT_RECEIVE_FILTER}"

    # Start dlt-receive
    echo "Start dlt-receive"
    "${DEV_DLT_PATH}"/dlt-receive -a -f "${DLT_RECEIVE_FILTER}" 127.0.0.1 > "${DLT_TEST_RESULT}" &
    sleep "${SLEEP_TIME}"

    # Start dlt-qnx-system
    echo "Start dlt-qnx-system"
    "${DEV_DLT_PATH}"/dlt-qnx-system > /dev/null &
    sleep "${SLEEP_TIME}"

    # Start dlt-test-qnx-slogger and wait until it's done
    echo "Start dlt-test-qnx-slogger"
    "${DEV_DLT_PATH}"/dlt-test-qnx-slogger -n "${COUNT}" -d "${DELAY}" -l "${LENGTH}" > /dev/null
    sleep "${SLEEP_TIME}"
}

################################################################################
# Function:    dlt_qnx_test_result
#
# Description: Verify result from dlt_run_app() function
################################################################################
dlt_qnx_test_result()
{
    echo ""
    echo "Verify test result"

    TEST_RESULT_1="PASS"
    TEST_RESULT_2="PASS"
    TEST_RESULT_3="PASS"
    BUFFER_NAME="dlt_test_qnx_slogger"

    #########################################
    # 1. Verify if all messages are received
    #########################################
    RESULT_COUNT=$(grep "${BUFFER_NAME}.*slog" -c "${DLT_TEST_RESULT}")
    if [ "${RESULT_COUNT}" -ne "${COUNT}" ]; then
        echo "Number of log messages are not matching (Expected: ${COUNT}, Actual: ${RESULT_COUNT})"
        TEST_RESULT_1="FAIL"
    fi

    ####
    RESULT=$(grep "${BUFFER_NAME}.*slog" "${DLT_TEST_RESULT}")
    RESULT_TIME_PREV=0
    echo "$RESULT"| while read LINE
    do
        pr_verbose "$0" "TEST : LINE=${LINE}"

        RESULT_TIME=$(echo "${LINE}" | awk -F " " '{print $3}')
        RESULT_PAYLOAD=$(echo "${LINE}" | awk -F " " '{print $17}')

        #############################################################
        # 2. Verify if each log messages have expected time interval
        #    Allow diff between ${DELAY} and ${DELAY}+10 msec
        #############################################################
        pr_verbose "$0" "TEST2: RESULT_TIME=${RESULT_TIME}, RESULT_TIME_PREV=${RESULT_TIME_PREV}"

        # Compare diff of timestamps from second log message
        if [ "${RESULT_TIME_PREV}" -ne 0 ]; then
            RESULT_TIME_DIFF=$((RESULT_TIME - RESULT_TIME_PREV))

            RANGE_MIN=$((DELAY * 10))
            RANGE_MAX=$(((DELAY + 10) * 10))
            if [[ "${RANGE_MIN}" -gt "${RESULT_TIME_DIFF}" ]] | [[ "${RANGE_MAX}" -le "${RESULT_TIME_DIFF}" ]]; then
                echo "Diff of timestamp is too big (Expected diff: ${RANGE_MIN}-${RANGE_MAX}, Actual diff: ${RESULT_TIME_DIFF}"
                TEST_RESULT_2="FAIL"
            fi
        fi
        RESULT_TIME_PREV=${RESULT_TIME}

        #################################################################
        # 3. Verify if each log messages receives expected string length
        #################################################################
        pr_verbose "$0" "TEST2: RESULT_PAYLOAD=${RESULT_PAYLOAD}"

        RESULT_LENGTH=$(echo ${#RESULT_PAYLOAD})
        if [ "${RESULT_LENGTH}" -ne "${LENGTH}" ]; then
            echo "Length is not matching (Excepted:${LENGTH}, Actual:${RESULT_LENGTH})"
            TEST_RESULT_3="FAIL"
        fi
    done

    # Print test result
    echo "  1. Verify if all messages are received                        : ${TEST_RESULT_1}"
    echo "  2. Verify if each log messages have expected time interval    : ${TEST_RESULT_2}"
    echo "  3. Verify if each log messages receives expected string length: ${TEST_RESULT_3}"
}

################################################################################
# Function:    dlt_qnx_verify_app
#
# Description: Verify if necessary binaries are existing on the target and have
#              execute permission
################################################################################
dlt_qnx_verify_app()
{
    APP=$1

    pr_verbose "$0" "Verify ${APP} is existing on the target and have execute permission"

    if [ ! -e "${APP}" ]; then
        echo "${APP} is missing!"
        exit 1
    elif [ ! -x "${APP}" ]; then
        echo "${APP} does not have execute permission!"
        exit 1
    fi
}

################################################################################
# Main
################################################################################

VERBOSE=0

SLEEP_TIME=1

COUNT=10
DELAY=500
LENGTH=100

DLT_TEST_RESULT="dlt_qnx_system_test.txt"
DLT_RECEIVE_FILTER="dlt_receive_filter.txt"

DEV_DLT_PATH="/usr/bin"

# OS
if [ "$(uname)" != "QNX" ]; then
    echo "This script can be only run under QNX system!"
    exit 1
fi

# Options
while getopts "vn:d:l:h" optKey; do
    case "${optKey}" in
        v)
            VERBOSE=1
            ;;
        n)
            COUNT=${OPTARG}
            ;;
        d)
            DELAY=${OPTARG}
            ;;
        l)
            LENGTH=${OPTARG}
            ;;
        '-h'|'--help'|*)
            usage
            ;;
    esac
done

echo "*******************************"
echo " Run dlt-test-qnx-system with:"
echo "   Number of logs: ${COUNT}"
echo "   Delay         : ${DELAY} msec"
echo "   Payload length: ${LENGTH} bytes"
echo "*******************************"
echo ""

# Start dlt-daemon
if [ ! "$(pidin u | grep dlt-daemon)" ]; then
    echo "Start dlt-daemon in daemonized mode"
    dlt-daemon -d
fi

# Verify necessary binaries are available under the target
dlt_qnx_verify_app ${DEV_DLT_PATH}/dlt-receive
dlt_qnx_verify_app ${DEV_DLT_PATH}/dlt-qnx-system
dlt_qnx_verify_app ${DEV_DLT_PATH}/dlt-test-qnx-slogger

#dlt_qnx_configure_setup

dlt_qnx_clean_app
dlt_qnx_clean_log

dlt_qnx_run_app
dlt_qnx_clean_app

dlt_qnx_test_result
dlt_qnx_clean_log

slay dlt-daemon
