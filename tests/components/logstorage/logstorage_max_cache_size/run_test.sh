#!/bin/sh
set -e
# cleanup on exit to avoid hanging ctest
PID_FILE="/tmp/dlt.pid.logstorage_max_cache_size"
cleanup() {
    if [ -f "$PID_FILE" ]; then
        while IFS= read -r pid; do
            kill -9 "$pid" 2>/dev/null || true
        done < "$PID_FILE"
        rm -f "$PID_FILE"
    fi
    if [ -p /tmp/dlt ] || [ -S /tmp/dlt ]; then
        rm -f /tmp/dlt || true
    fi
}
trap cleanup EXIT
# Args: <dlt-daemon> <test-bin> <dlt-convert>
DAEMON_BIN="$1"
TEST_BIN="$2"

DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR"

rm -f "$PID_FILE"

# The daemon creates its IPC endpoint /tmp/dlt itself (FIFO or UNIX socket
# depending on the build). No background reader is needed: the daemon opens
# the FIFO O_RDWR, an extra reader would steal messages non-deterministically.

"$DAEMON_BIN" -c "$DIR/dlt.conf" &
DAEMON_PID=$!
echo "$DAEMON_PID" >> "$PID_FILE"

# Wait until the daemon has created its IPC endpoint /tmp/dlt (up to 5s)
i=0
while [ ! -p /tmp/dlt ] && [ ! -S /tmp/dlt ]; do
    if ! kill -0 "$DAEMON_PID" 2>/dev/null; then
        echo "dlt-daemon exited before creating /tmp/dlt" >&2
        exit 1
    fi
    i=$((i + 1))
    if [ "$i" -ge 50 ]; then
        echo "Timeout waiting for daemon IPC endpoint /tmp/dlt" >&2
        exit 1
    fi
    sleep 0.1
done
# Small margin so the remaining sockets (TCP/control) are up as well
sleep 0.2

"$TEST_BIN" &
TEST_PID=$!

# Wait for the test to finish so the daemon has processed and reported
# all messages before it is stopped
wait "$TEST_PID" || true
sleep 0.5

if [ -f "$PID_FILE" ]; then
    while IFS= read -r pid; do
        kill -9 "$pid" 2>/dev/null || true
    done < "$PID_FILE"
fi

exit 0
