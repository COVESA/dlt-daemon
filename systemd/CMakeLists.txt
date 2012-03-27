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

if(WITH_SYSTEMD)
    SET( DLT_SYSLOG_APPID "SYS" )
    SET( DLT_SYSLOG_CTID "LOG" )
    SET( DLT_SYSLOG_PORT 4712 )

    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/systemd/dlt.service.cmake ${PROJECT_BINARY_DIR}/systemd/dlt.service @ONLY)
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/systemd/dlt-syslog.service.cmake ${PROJECT_BINARY_DIR}/systemd/dlt-syslog.service @ONLY)
endif(WITH_SYSTEMD)
