#!/bin/sh
set -e
# cleanup on exit to avoid hanging ctest
cleanup() {
    if [ -n "$READER_PID" ]; then
        kill "$READER_PID" 2>/dev/null || true
    fi
    if [ -f /tmp/dlt.pid ]; then
        while IFS= read -r pid; do
            kill -9 "$pid" 2>/dev/null || true
        done < /tmp/dlt.pid
        rm -f /tmp/dlt.pid
    fi
    if [ -p /tmp/dlt ]; then
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

rm -f /tmp/dlt.pid
rm -f ${DIR}/${APPID}*.dlt 2>/dev/null || true

if [ ! -p /tmp/dlt ]; then
    rm -f /tmp/dlt
    mkfifo /tmp/dlt
    chmod 666 /tmp/dlt
fi

# The daemon opens the FIFO O_RDWR, so no background reader is needed.
# An extra reader would steal messages from the daemon non-deterministically.

"$DAEMON_BIN" -c "$DIR/dlt.conf" &
echo $! >> /tmp/dlt.pid
sleep 0.2

"$TEST_BIN" -c "$CTXNUM" -n 100 &
# Give the daemon enough time to fill the log file so that rotation
# (and therefore fsync) happens before the daemon is stopped.
sleep 0.5

if [ -f /tmp/dlt.pid ]; then
    while IFS= read -r pid; do
        kill -9 "$pid" 2>/dev/null || true
    done < /tmp/dlt.pid
fi
sleep 0.5

# Stop background FIFO reader
if [ -n "$READER_PID" ]; then
    kill "$READER_PID" 2>/dev/null || true
fi

find "$DIR" -name '*.dlt' -print0 | xargs -0 -r "$CONVERT_BIN" -a

exit 0
