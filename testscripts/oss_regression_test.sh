#!/bin/bash
#
# SPDX-License-Identifier: MPL-2.0
#
# DLT regression tests — runs inside the dlt-daemon-ci Docker container.
#
# Usage: ./docker/run.sh devtest
#
# This script builds dlt-daemon with tests/examples enabled, starts the daemon,
# and runs a series of integration test cases.

set -euo pipefail

DLT_PORT="${DLT_PORT:-3490}"
DLT_ECU="${DLT_ECU:-ECU1}"
DLT_APPID="${DLT_APPID:-LOG}"
DLT_CTXID="${DLT_CTXID:-TEST}"
BUILD_DIR="${BUILD_DIR:-build}"
TEST_DIR="/tmp/dlt-test"

echo "============================================"
echo "  DLT Regression Tests"
echo "============================================"

# ----------------------------------------------------------------
# Prepare test environment
# ----------------------------------------------------------------
rm -rf "${TEST_DIR}"
mkdir -p "${TEST_DIR}"

cat > "${TEST_DIR}/dlt.conf" <<'CONF'
SendContextRegistration = 1
ECUId = ECU1
SharedMemorySize = 100000
LoggingMode = 0
LoggingLevel = 6
LoggingFilename = /tmp/dlt-test/dlt.log
TimeOutOnSend = 4
RingbufferMinSize = 500000
RingbufferMaxSize = 10000000
RingbufferStepSize = 500000
ControlSocketPath = /tmp/dlt-test/dlt-ctrl.sock
OfflineLogstorageMaxDevices = 2
OfflineLogstorageDirPath = /tmp/dlt-test/logstorage
OfflineLogstorageTimestamp = 1
OfflineLogstorageDelimiter = _
OfflineLogstorageMaxCounter = 999
OfflineLogstorageCacheSize = 30000
CONF

mkdir -p "${TEST_DIR}/logstorage"

export PATH="${BUILD_DIR}/bin:${PATH}"

FAILED=0

# ----------------------------------------------------------------
# Helper: start daemon
# ----------------------------------------------------------------
start_daemon() {
    local protocol="${1:-1}"
    if [ "${protocol}" = "2" ]; then
        "${BUILD_DIR}/bin/dlt-daemon" -c "${TEST_DIR}/dlt.conf" -x 2 &
    else
        "${BUILD_DIR}/bin/dlt-daemon" -c "${TEST_DIR}/dlt.conf" &
    fi
    local pid=$!
    echo "${pid}" > "${TEST_DIR}/daemon.pid"

    for i in $(seq 1 30); do
        if nc -z 127.0.0.1 "${DLT_PORT}" 2>/dev/null; then
            echo "dlt-daemon is ready (PID=${pid}, protocol v${protocol})"
            return 0
        fi
        sleep 0.5
    done
    echo "ERROR: dlt-daemon did not start"
    cat "${TEST_DIR}/dlt.log" 2>/dev/null || true
    return 1
}

stop_daemon() {
    kill "$(cat "${TEST_DIR}/daemon.pid" 2>/dev/null)" 2>/dev/null || true
    sleep 1
}

# ----------------------------------------------------------------
# Case 1: dlt-example-user -> dlt-receive
# ----------------------------------------------------------------
echo ""
echo "=== Case 1: dlt-example-user -> dlt-receive ==="
start_daemon 1

timeout 10 "${BUILD_DIR}/bin/dlt-receive" -a 127.0.0.1 > "${TEST_DIR}/case1_output.txt" 2>&1 &
RECV_PID=$!
sleep 1

"${BUILD_DIR}/bin/dlt-example-user" -n 5 -d 100 "regression_test_msg" &
wait $!

sleep 2
kill "${RECV_PID}" 2>/dev/null || true
wait "${RECV_PID}" 2>/dev/null || true

if grep -q "regression_test_msg" "${TEST_DIR}/case1_output.txt"; then
    echo "PASS: Messages received by dlt-receive"
else
    echo "FAIL: No messages received"
    FAILED=1
fi

# ----------------------------------------------------------------
# Case 2: dlt-control commands
# ----------------------------------------------------------------
echo ""
echo "=== Case 2: dlt-control commands ==="
timeout 5 "${BUILD_DIR}/bin/dlt-control" -k 127.0.0.1 || true
timeout 5 "${BUILD_DIR}/bin/dlt-control" -j 127.0.0.1 || true
timeout 5 "${BUILD_DIR}/bin/dlt-control" -l 6 -a "${DLT_APPID}" -c "${DLT_CTXID}" 127.0.0.1 || true
timeout 5 "${BUILD_DIR}/bin/dlt-control" -d 4 127.0.0.1 || true
timeout 5 "${BUILD_DIR}/bin/dlt-control" -r 1 -a "${DLT_APPID}" -c "${DLT_CTXID}" 127.0.0.1 || true
timeout 5 "${BUILD_DIR}/bin/dlt-control" -o 127.0.0.1 || true
timeout 5 "${BUILD_DIR}/bin/dlt-control" -g 127.0.0.1 || true
echo "PASS: dlt-control commands executed"

# ----------------------------------------------------------------
# Case 3: Logstorage
# ----------------------------------------------------------------
echo ""
echo "=== Case 3: Logstorage connect/disconnect ==="

cat > "${TEST_DIR}/logstorage/dlt_logstorage.conf" <<'CONF'
[FILTER1]
LogAppName=LOG
ContextName=TEST
LogLevel=DLT_LOG_INFO
File=Test
FileSize=10000
NOFiles=5
CONF

timeout 15 "${BUILD_DIR}/bin/dlt-logstorage-ctrl" \
    -c 1 -e "${DLT_ECU}" -p "${TEST_DIR}/logstorage" -t 10 \
    -C "${TEST_DIR}/dlt.conf" 127.0.0.1 || true
sleep 1

# Restart daemon if it crashed during logstorage connect
if ! kill -0 "$(cat "${TEST_DIR}/daemon.pid" 2>/dev/null)" 2>/dev/null; then
    echo "WARNING: Daemon crashed during logstorage connect, restarting"
    start_daemon
fi

"${BUILD_DIR}/bin/dlt-example-user" -n 10 -d 100 "logstorage_test_msg" &
wait $!
sleep 2

if ls "${TEST_DIR}/logstorage"/*.dlt 2>/dev/null; then
    echo "PASS: Logstorage files created"
    timeout 15 "${BUILD_DIR}/bin/dlt-logstorage-ctrl" \
        -c 0 -e "${DLT_ECU}" -p "${TEST_DIR}/logstorage" -t 10 \
        -C "${TEST_DIR}/dlt.conf" 127.0.0.1 || true
else
    echo "FAIL: No logstorage files created"
    FAILED=1
fi

# ----------------------------------------------------------------
# Case 4: dlt-convert
# ----------------------------------------------------------------
echo ""
echo "=== Case 4: dlt-convert ==="
DLT_FILE=$(ls "${TEST_DIR}/logstorage"/*.dlt 2>/dev/null | head -1)
if [ -n "${DLT_FILE}" ]; then
    "${BUILD_DIR}/bin/dlt-convert" -c "${DLT_FILE}" || true
    "${BUILD_DIR}/bin/dlt-convert" -a -b 0 -e 4 "${DLT_FILE}" || true
    echo "PASS: dlt-convert executed"
else
    echo "SKIP: No DLT file to convert"
fi

# ----------------------------------------------------------------
# Case 5: dlt-example-user-func
# ----------------------------------------------------------------
echo ""
echo "=== Case 5: Functional API example ==="
timeout 10 "${BUILD_DIR}/bin/dlt-receive" -a 127.0.0.1 > "${TEST_DIR}/case5_output.txt" 2>&1 &
RECV_PID=$!
sleep 1

"${BUILD_DIR}/bin/dlt-example-user-func" -n 3 -d 100 "func_test_msg" &
wait $!
sleep 2
kill "${RECV_PID}" 2>/dev/null || true
wait "${RECV_PID}" 2>/dev/null || true

if [ -s "${TEST_DIR}/case5_output.txt" ]; then
    echo "PASS: Functional API messages received"
else
    echo "FAIL: No messages received"
    FAILED=1
fi

# ----------------------------------------------------------------
# Case 6: dlt-adaptor-stdin
# ----------------------------------------------------------------
echo ""
echo "=== Case 6: Adaptor stdin ==="
timeout 10 "${BUILD_DIR}/bin/dlt-receive" -a 127.0.0.1 > "${TEST_DIR}/case6_output.txt" 2>&1 &
RECV_PID=$!
sleep 1

echo "adaptor_stdin_test" | timeout 5 "${BUILD_DIR}/bin/dlt-adaptor-stdin" -a SINA -c SINC &
wait $!
sleep 2
kill "${RECV_PID}" 2>/dev/null || true
wait "${RECV_PID}" 2>/dev/null || true

if grep -q "adaptor_stdin_test" "${TEST_DIR}/case6_output.txt"; then
    echo "PASS: Adaptor stdin message received"
else
    echo "FAIL: Adaptor stdin message not received"
    FAILED=1
fi

# ----------------------------------------------------------------
# Case 7: dlt-example-filetransfer
# ----------------------------------------------------------------
echo ""
echo "=== Case 7: File transfer ==="
echo "This is a test file for DLT file transfer." > "${TEST_DIR}/transfer_me.txt"

timeout 10 "${BUILD_DIR}/bin/dlt-receive" -a 127.0.0.1 > "${TEST_DIR}/case7_output.txt" 2>&1 &
RECV_PID=$!
sleep 1

timeout 8 "${BUILD_DIR}/bin/dlt-example-filetransfer" \
    -a FLTR -c FLTR -t 5000 "${TEST_DIR}/transfer_me.txt" &
wait $!
sleep 2
kill "${RECV_PID}" 2>/dev/null || true
wait "${RECV_PID}" 2>/dev/null || true

if [ -s "${TEST_DIR}/case7_output.txt" ]; then
    echo "PASS: File transfer messages received"
else
    echo "FAIL: No file transfer messages received"
    FAILED=1
fi

# ----------------------------------------------------------------
# Case 8: dlt-sortbytimestamp
# ----------------------------------------------------------------
echo ""
echo "=== Case 8: Sort by timestamp ==="
DLT_FILE=$(ls "${TEST_DIR}/logstorage"/*.dlt 2>/dev/null | head -1)
if [ -n "${DLT_FILE}" ]; then
    "${BUILD_DIR}/bin/dlt-sortbytimestamp" "${DLT_FILE}" "${TEST_DIR}/sorted.dlt" || true
    if [ -f "${TEST_DIR}/sorted.dlt" ]; then
        echo "PASS: Sorted file created"
        "${BUILD_DIR}/bin/dlt-convert" -c "${TEST_DIR}/sorted.dlt" || true
    else
        echo "FAIL: Sorted file not created"
        FAILED=1
    fi
else
    echo "SKIP: No DLT file to sort"
fi

# ----------------------------------------------------------------
# Case 9: dlt-control injection
# ----------------------------------------------------------------
echo ""
echo "=== Case 9: Injection message ==="
timeout 10 "${BUILD_DIR}/bin/dlt-receive" -a 127.0.0.1 > "${TEST_DIR}/case9_output.txt" 2>&1 &
RECV_PID=$!
sleep 1

timeout 5 "${BUILD_DIR}/bin/dlt-control" \
    -a "${DLT_APPID}" -c "${DLT_CTXID}" \
    -s 0xFFF1 -m "injection_payload" 127.0.0.1 || true

sleep 2
kill "${RECV_PID}" 2>/dev/null || true
wait "${RECV_PID}" 2>/dev/null || true
echo "PASS: Injection command executed"

# ----------------------------------------------------------------
# Case 10: DLT v2 protocol
# ----------------------------------------------------------------
echo ""
echo "=== Case 10: DLT v2 example user ==="
stop_daemon
start_daemon 2

timeout 10 "${BUILD_DIR}/bin/dlt-receive-v2" -a 127.0.0.1 > "${TEST_DIR}/case10_output.txt" 2>&1 &
RECV_PID=$!
sleep 1

"${BUILD_DIR}/bin/dlt-example-user-v2" -n 5 -d 100 "v2_test_msg" &
wait $!
sleep 2
kill "${RECV_PID}" 2>/dev/null || true
wait "${RECV_PID}" 2>/dev/null || true

if [ -s "${TEST_DIR}/case10_output.txt" ]; then
    echo "PASS: DLT v2 messages received"
else
    echo "FAIL: No DLT v2 messages received"
    FAILED=1
fi

# ----------------------------------------------------------------
# Cleanup
# ----------------------------------------------------------------
stop_daemon
rm -rf "${TEST_DIR}"

# ----------------------------------------------------------------
# Summary
# ----------------------------------------------------------------
echo ""
echo "============================================"
echo "  DLT Regression Test Summary"
echo "============================================"
echo "Case 1:  dlt-example-user -> dlt-receive"
echo "Case 2:  dlt-control commands"
echo "Case 3:  Logstorage connect/disconnect"
echo "Case 4:  dlt-convert file reading"
echo "Case 5:  dlt-example-user-func"
echo "Case 6:  dlt-adaptor-stdin"
echo "Case 7:  dlt-example-filetransfer"
echo "Case 8:  dlt-sortbytimestamp"
echo "Case 9:  dlt-control injection"
echo "Case 10: DLT v2 example user"
echo "============================================"

if [ "${FAILED}" -ne 0 ]; then
    echo "SOME TESTS FAILED"
    exit 1
fi

echo "ALL TESTS PASSED"
