#!/bin/sh
set -e
# cleanup on exit to avoid hanging ctest
PID_FILE="/tmp/dlt.pid.logstorage_one_file"
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
# Args: <dlt-daemon> <test-bin> <ctxnum> <dlt-convert>
DAEMON_BIN="$1"
TEST_BIN="$2"
CTXNUM="$3"
CONVERT_BIN="$4"

DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR"

rm -f "$PID_FILE"
rm -f "$DIR"/LONE*.dlt
mkdir -p "$DIR"

# The daemon creates its IPC endpoint /tmp/dlt itself (FIFO or UNIX socket
# depending on the build). It opens the FIFO O_RDWR, so no background reader
# is needed: an extra reader would steal messages non-deterministically.

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

"$TEST_BIN" -c "$CTXNUM" &
sleep 0.1

if [ -f "$PID_FILE" ]; then
    while IFS= read -r pid; do
        kill -9 "$pid" 2>/dev/null || true
    done < "$PID_FILE"
fi
sleep 0.5

find "$DIR" -name '*.dlt' -print0 | xargs -0 -r "$CONVERT_BIN" -a

exit 0
