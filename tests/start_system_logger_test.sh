#!/bin/bash

#enable logging of files and setup
sudo sed -i 's/LogFileEnable = 0/LogFileEnable = 1/g' /usr/local/etc/dlt-system.conf
echo "# TEST LOG TO SYSTEMLOGGER_PROC" | sudo tee -a /usr/local/etc/dlt-system.conf
echo "LogFileFilename = /proc/systemlogger" | sudo tee -a /usr/local/etc/dlt-system.conf
echo "LogFileMode = 1" | sudo tee -a /usr/local/etc/dlt-system.conf
echo "LogFileTimeDelay = 3" | sudo tee -a /usr/local/etc/dlt-system.conf
echo "LogFileContextId = PROC" | sudo tee -a /usr/local/etc/dlt-system.conf
#comile the kernel module for system logging
cd mod_system_logger
make
cd ..
#enable mod
sudo insmod mod_system_logger/mod_system_logger.ko
#start dlt-daemon
dlt-daemon &
sleep 1
#start dlt-system
dlt-system &
sleep 1
#start dlt-receiver
../build/tests/dlt_test_receiver -l localhost &
sleep 1
pid=$!
wait $pid
exitcode=$?
#kill processes and remove mod
pkill dlt-daemon
pkill dlt-system
sudo rmmod mod_system_logger
cd mod_system_logger
make clean
cd ..
# if exit code == 159 , test successfull
tput setaf 1
if [ $exitcode == 159 ]; then
        echo "System Logger tests successfull."
else
        echo "System Logger tests failed."
	echo "Maybe missing kernel-heaers"
	echo "for compiling the test module"
fi
tput setaf 7
