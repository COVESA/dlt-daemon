#  This file is for starting dlt-adaptor-udp
#
#  For more informations about starting options of dlt-daemon use the command "dlt-adaptor-udp -h".
#
#  basic.target A special target unit covering early boot-up.
#  Usually this should pull-in all sockets, mount points, swap devices and 
#  other basic initialization necessary for the general purpose daemons. 
#  Most normal daemons should have dependencies of type After and Requires on this unit

[Unit]
Description=DLT Syslog Adapter

[Service]
ExecStart=@CMAKE_INSTALL_PREFIX@/bin/dlt-adaptor-udp -a @DLT_SYSLOG_APPID@ -c @DLT_SYSLOG_CTID@ -p @DLT_SYSLOG_PORT@
# Restart=always

[Install]
WantedBy=basic.target
