#!/bin/sh
clear
processname="dlt-daemon"
pidofdlt=`pidof $processname`

if [ $pidofdlt ]
 then
   echo "DLT Deamon is running with PID:$pidofdlt"
 else
  echo "DLT Daemon is NOT running"
fi

if [ -f /var/log/messages ] 
 then
echo "------- messages  ----------------"
cat /var/log/messages | grep -a DLT | tail
fi

if [ -f /var/log/syslog ] 
 then
 echo "------- SYSLOG -------------------"
 cat /var/log/syslog | grep -a DLT | tail
fi

