#!/bin/sh
#
# Starts the DLT Daemon
#
# chkconfig: 2345 21 87
# description: Provides DLT Debug Logging & Trace functionality 
# processname: dlt-daemon

# Source function library.
. /etc/rc.d/init.d/functions


processname=dlt-daemon
processpath=@CMAKE_INSTALL_PREFIX@/bin
servicename=dlt-daemon


[ -x $processpath ] || exit 0

start() {
    echo -n $"Starting $processname daemon: "
    @CMAKE_INSTALL_PREFIX@/bin/dlt-daemon -d
}

stop() {
    echo -n $"Stopping $processname daemon: "
    killproc $servicename -TERM
    RETVAL=$?
    echo
    [ $RETVAL -eq 0 ] && rm -f /var/lock/subsys/$servicename
}

RETVAL=0

case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    restart)
        stop
        start
        ;;
    condrestart)
        if [ -f /var/lock/subsys/$servicename ]; then
            stop
            start
        fi
        ;;
    reload)
        restart
        ;;
    status)
        status $processname
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
    RETVAL=$?
        ;;
    *)
        echo $"Usage: $0 {start|stop|status|restart|condrestart|reload}"
        ;;
esac

exit $RETVAL
