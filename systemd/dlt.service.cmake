#######
# SPDX license identifier: MPL-2.0
#
# Copyright (C) 2011-2015, BMW AG
#
# This file is part of COVESA Project DLT - Diagnostic Log and Trace.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License (MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.covesa.org/.
#######

[Unit]
Description=COVESA DLT logging daemon
Documentation=man:dlt-daemon(1) man:dlt.conf(5)

[Service]
Type=simple
User=@DLT_USER@
ExecStart=@CMAKE_INSTALL_PREFIX@/bin/dlt-daemon
WatchdogSec=@DLT_WatchdogSec@
NotifyAccess=main
LimitCORE=infinity

[Install]
WantedBy=basic.target
