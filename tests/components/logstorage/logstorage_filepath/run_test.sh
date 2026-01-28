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
# Args: <dlt-daemon-bin> <test-bin> <ctxnum> <dlt-convert-bin>
DAEMON_BIN="$1"
TEST_BIN="$2"
CTXNUM="$3"
CONVERT_BIN="$4"

DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR"

# Ensure logstorage config is present (CMake will usually copy it)

# Prepare dirs and PID file
rm -rf "$DIR/test"
rm -rf "$DIR/aa"
mkdir -p "$DIR/test"
rm -f /tmp/dlt.pid

# Create `aa` as a regular file (not a directory) so attempts to create aa/FPTH_FAIL fail with ENOTDIR
touch "$DIR/aa"
chmod 444 "$DIR/aa" || true

# Ensure FIFO exists with world-write so daemon can open it
if [ ! -p /tmp/dlt ]; then
    rm -f /tmp/dlt
    mkfifo /tmp/dlt
    chmod 666 /tmp/dlt
fi

# Start a background reader so the daemon can open the FIFO for writing
cat /tmp/dlt > /dev/null 2>&1 &
READER_PID=$!

# Start daemon and record PID
"$DAEMON_BIN" -c "$DIR/dlt.conf" &
echo $! >> /tmp/dlt.pid
sleep 0.2

# Run the test binary
"$TEST_BIN" -c "$CTXNUM" &
sleep 1

# Kill daemon(s) from PID file
if [ -f /tmp/dlt.pid ]; then
    while IFS= read -r pid; do
        kill -9 "$pid" 2>/dev/null || true
    done < /tmp/dlt.pid
fi
sleep 0.5

# Convert any generated files
if [ -f "$DIR/aa/FPTH_FAIL.dlt" ]; then
    echo 'FPTH_FAIL.dlt exists'
    "$CONVERT_BIN" -a "$DIR/aa/FPTH_FAIL.dlt"
fi
if [ -f "$DIR/test/FPTH_OK.dlt" ]; then
    echo 'FPTH_OK.dlt exists'
    "$CONVERT_BIN" -a "$DIR/test/FPTH_OK.dlt"
fi

# Restore `aa` as a directory for cleanup
rm -f "$DIR/aa" || true
mkdir -p "$DIR/aa" && chmod 755 "$DIR/aa" || true

# Stop background FIFO reader
if [ -n "$READER_PID" ]; then
    kill "$READER_PID" 2>/dev/null || true
fi

exit 0
