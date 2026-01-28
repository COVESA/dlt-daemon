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
# Args: <dlt-daemon> <test-bin> <dlt-convert>
DAEMON_BIN="$1"
TEST_BIN="$2"

DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR"

rm -f /tmp/dlt.pid

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

"$TEST_BIN" &
sleep 0.2

if [ -f /tmp/dlt.pid ]; then
    while IFS= read -r pid; do
        kill -9 "$pid" 2>/dev/null || true
    done < /tmp/dlt.pid
fi

# Stop background FIFO reader
if [ -n "$READER_PID" ]; then
    kill "$READER_PID" 2>/dev/null || true
fi

exit 0
