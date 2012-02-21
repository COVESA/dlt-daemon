#  This file is for starting the dlt-daemon as a service with systemd at the multi-user.target
#
#  For more informations about starting options of dlt-daemon use the command
#  "dlt-daemon -h".
#
#  Multi-user.target is a special target unit for setting up a multi-user system (non-graphical). 
#  For more details about the multi-user.target see systemd.special(7).

[Unit]
Description=DLT Daemon for logging and tracing

[Service]
ExecStart=/usr/local/bin/dlt-daemon
#Restart=always

[Install]
WantedBy=multi-user.target
