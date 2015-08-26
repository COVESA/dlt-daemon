#!/bin/bash

# check if dlt-daemon is running
daemon_running=`/usr/bin/ps -C dlt-daemon | wc -l`
daemon_pid=0

if [ "$daemon_running" -lt "2" ]; then
  echo "No daemon running, starting one myself"
  /usr/bin/dlt-daemon > /tmp/dlt_daemon_dlt_receiver_test.txt & 
  daemon_pid=$!
  echo "daemon pid: " ${daemon_pid}
else
  echo "dlt-daemon already running"
fi

# create a directory in /tmp where all logs will be stored
output_dir=`mktemp -d /tmp/DLT_TESTING_XXXXXX`
echo "Using directory " ${output_dir}

# start dlt-receive (in background) and store PID
echo "Starting dlt-receive"
/usr/bin/dlt-receive -o ${output_dir}/dlt_test.dlt localhost &
dlt_receive_pid=$!
disown

# start dlt-example-user to create some logs
# sleep time: 100ms
# number of messages: 10
/usr/bin/dlt-example-user -d 100 -n 10 TEST_MESSAGE_ONE

# stop dlt-receive
kill ${dlt_receive_pid}

# show content of /tmp
echo "log-file after first run"
ls -l ${output_dir}

# start dlt-receive (in background) and store PID
echo "Starting dlt-receive"
/usr/bin/dlt-receive -o ${output_dir}/dlt_test_no_log.dlt localhost &
dlt_receive_pid=$!
disown

# start dlt-example-user to create some logs, disable logging through environment variable
DLT_INITIAL_LOG_LEVEL=::-1 /usr/bin/dlt-example-user -d 100 -n 10 TEST_MESSAGE_TWO

# show content of /tmp, log should not contain log messages from dlt-example-user (TEST_MESSAGE_TWO)
kill ${dlt_receive_pid}
echo "log-file after second run"
ls -l ${output_dir}

# directory will not be cleaned up
echo "Used directory " ${output_dir}


if [ "${daemon_pid}" -ne "0" ]; then
  sleep 1
  kill ${daemon_pid}
fi
