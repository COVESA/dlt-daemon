#!/bin/bash

file="testfile_filetransfer.txt"
fullpath="$(pwd)/testfile_filetransfer.txt"
#start dlt-daemon
dlt-daemon &
sleep 1
#start dlt-test-receiver
./../build/tests/dlt_test_receiver -f localhost &
sleep 1
#send file to daemon
dlt-example-filetransfer $fullpath &
sleep 1
#create md5 sum
md5_1=($(md5sum $file))
md5_2=($(md5sum /tmp/$file))
echo $md5_1
echo $md5_2
#verify the sums
tput setaf 1
if [ $md5_1 == $md5_2 ]
then
	echo "Files are equal. Transfer succuess."
else
	echo "File not equal. Error on transmission"
fi
tput setaf 7
pkill dlt-daemon
