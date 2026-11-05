#!/bin/sh
set -e
# cleanup on exit to avoid hanging ctest
IPC_PATH="${3:-/tmp}"
ENDPOINT="$IPC_PATH/dlt"
PID_FILE="/tmp/dlt.pid.gtest_dlt_logd_converter"
cleanup() {
    if [ -f "$PID_FILE" ]; then
        while IFS= read -r pid; do
            kill -9 "$pid" 2>/dev/null || true
        done < "$PID_FILE"
        rm -f "$PID_FILE"
    fi
    if [ -p "$ENDPOINT" ] || [ -S "$ENDPOINT" ]; then
        rm -f "$ENDPOINT" || true
    fi
}
trap cleanup EXIT
# Args: <dlt-daemon> <test-bin> <ipc-path>
DAEMON_BIN="$1"
TEST_BIN="$2"

DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR"

rm -f "$PID_FILE"
rm -f "$DIR"/MLTI*.dlt
mkdir -p "$DIR"

# The daemon creates its IPC endpoint <ipc-path>/dlt itself (FIFO or UNIX
# socket depending on the build). No background reader is needed: the daemon
# opens the FIFO O_RDWR, an extra reader would steal messages non-deterministically.

"$DAEMON_BIN" -c "$DIR/dlt.conf" &
DAEMON_PID=$!
echo "$DAEMON_PID" >> "$PID_FILE"

# Wait until the daemon has created its IPC endpoint (up to 5s)
i=0
while [ ! -p "$ENDPOINT" ] && [ ! -S "$ENDPOINT" ]; do
    if ! kill -0 "$DAEMON_PID" 2>/dev/null; then
        echo "dlt-daemon exited before creating $ENDPOINT" >&2
        exit 1
    fi
    i=$((i + 1))
    if [ "$i" -ge 50 ]; then
        echo "Timeout waiting for daemon IPC endpoint $ENDPOINT" >&2
        exit 1
    fi
    sleep 0.1
done
# Small margin so the remaining sockets (TCP/control) are up as well
sleep 0.2

"$TEST_BIN" -c "$DIR/dlt-logd-converter.conf" &
sleep 0.1

if [ -f "$PID_FILE" ]; then
    while IFS= read -r pid; do
        kill -9 "$pid" 2>/dev/null || true
    done < "$PID_FILE"
fi
sleep 0.5

exit 0
