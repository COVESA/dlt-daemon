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
# Args: <dlt-daemon> <dlt-receive> <test-bin> <ctxnum> <dlt-convert>
DAEMON_BIN="$1"
RECEIVE_BIN="$2"
TEST_BIN="$3"
CTXNUM="$4"
CONVERT_BIN="$5"

DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR"

rm -f /tmp/dlt.pid
rm -rf "$DIR"/*.dlt

if [ ! -p /tmp/dlt ]; then
    rm -f /tmp/dlt
    mkfifo /tmp/dlt
    chmod 666 /tmp/dlt
fi

# Start background FIFO reader so daemon can open /tmp/dlt for writing
cat /tmp/dlt > /dev/null 2>&1 &
READER_PID=$!

"$DAEMON_BIN" -c "$DIR/dlt.conf" &
echo $! >> /tmp/dlt.pid
sleep 0.2

"$RECEIVE_BIN" -o "$DIR/dlt-receive.dlt" localhost &
sleep 0.2

"$TEST_BIN" -c "$CTXNUM" &
sleep 1

if [ -f /tmp/dlt.pid ]; then
    while IFS= read -r pid; do
        kill -9 "$pid" 2>/dev/null || true
    done < /tmp/dlt.pid
fi
sleep 0.5

if [ -f "$DIR/dlt-receive.dlt" ]; then
    echo 'Result from network'
    "$CONVERT_BIN" -a "$DIR/dlt-receive.dlt"
    sleep 0.5
fi

echo 'Result from file'
find "$DIR" -name '*.dlt' -not -name 'dlt-receive.dlt' -print0 | xargs -0 -r "$CONVERT_BIN" -a
sleep 0.5

# Stop background FIFO reader
if [ -n "$READER_PID" ]; then
    kill "$READER_PID" 2>/dev/null || true
fi

exit 0
