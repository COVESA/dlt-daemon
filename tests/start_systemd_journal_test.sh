#!/bin/bash

#build and install with SYSTEMD_JOURNAL=ON
mkdir ../build
cd ../build/
cmake -DWITH_SYSTEMD_JOURNAL=ON -DWITH_DLT_UNIT_TESTS=ON ..
make
sudo make install
#enable SYSTEMD_JOURNAL in config file
sudo vim -esnc '%s/JournalEnable = 0/JournalEnable = 1/g|:wq' /usr/local/etc/dlt-system.conf
#start dlt-daemon
dlt-daemon &
sleep 1
#start dlt_system
sudo dlt-system &
sleep 1
#send 10 times "DLT SYSTEM JOURNAL TEST"
for i in {1..1000}
do
        logger DLT SYSTEM JOURNAL TEST
done
#start receiver
./../build/tests/dlt_test_receiver -s localhost &
sleep 1
pid=$!
wait $pid
exitcode=$?
# kill processes, receiver automatically killed with daemon
pkill dlt-daemon
sudo pkill dlt-system
# if exit code == 159 , test successfull
tput setaf 1
if [ $exitcode == 159 ]; then
        echo "Systemd Journal tests successfull."
else
        echo "Systemd Journal tests failed."
fi
tput setaf 7
