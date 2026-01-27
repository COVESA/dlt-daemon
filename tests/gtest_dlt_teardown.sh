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
#file            : gtest_dlt_cleanup.sh
#
#Description     : user unit test cleanup
#
#Author Name     : Nguyen Thi Bich Dao
#Email Id        : <Dao.NguyenThiBich2@vn.bosch.com>
# Environment
tmpPath="/tmp"
pidFile="$tmpPath/dlt.pid"
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
    if  ! killPids "$pidFile"; then
        echo "Usage: "
        echo "sh ./gtest_dlt_teardown.sh"
        return 1
    fi
    return 0
}
########################################################################################
#main function
########################################################################################
cleanup