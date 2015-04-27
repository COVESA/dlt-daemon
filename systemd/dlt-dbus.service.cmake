#######
# Dlt - Diagnostic Log and Trace
# @licence make begin@
#
# Copyright (C) 2011-2012, BMW AG - Alexander Wenzel <alexander.aw.wenzel@bmw.de>
# 
# Contributions are licensed to the GENIVI Alliance under one or more
# Contribution License Agreements.
# 
# This Source Code Form is subject to the terms of the
# Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
# this file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# @licence end@
########

[Unit]
Description=GENIVI DLT DBus. Application to forward DBus messages to DLT.
Documentation=man:dlt-dbus(1) man:dlt-dbus.conf(5)
Wants=dlt.service

[Service]
Type=Simple
User=root
ExecStart=@CMAKE_INSTALL_PREFIX@/bin/dlt-dbus
WatchdogSec=@DLT_WatchdogSec@
NotifyAccess=main
LimitCORE=infinity

[Install]
WantedBy=basic.target

