#######
# @licence make begin@
# SPDX license identifier: MPL-2.0
#
# Copyright (C) 2011-2015, BMW AG"
#
# This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License (MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.genivi.org/.
# @licence end@
#######

[Unit]
Description=GENIVI DLT receive. Receive DLT messages from DLT daemon and print or store the messages.
Documentation=man:dlt-receive(1)
Wants=dlt.service

[Service]
Type=Simple
User=genivi
ExecStart=@CMAKE_INSTALL_PREFIX@/bin/dlt-receive -o /tmp/dlt_receive_log.dlt localhost
LimitCORE=infinity