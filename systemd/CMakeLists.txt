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
    set(SYSTEMD_CONFIGURATIONS_FILES_DIR "/lib/systemd/system" )
        
    if(WITH_SYSTEMD_WATCHDOG)
        set( DLT_WatchdogSec 2 )
        message( STATUS "The systemd watchdog is enabled for DLT daemon - timeout is set to ${DLT_WatchdogSec} seconds")
    else(WITH_SYSTEMD_WATCHDOG) 
        set( DLT_WatchdogSec 0 )
        message( STATUS "The systemd watchdog is disabled for DLT daemon")
    endif(WITH_SYSTEMD_WATCHDOG)
    
    configure_file(${CMAKE_SOURCE_DIR}/systemd/dlt.service.cmake ${PROJECT_BINARY_DIR}/systemd/dlt.service)
    install(FILES ${PROJECT_BINARY_DIR}/systemd/dlt.service DESTINATION ${SYSTEMD_CONFIGURATIONS_FILES_DIR} )
    message( STATUS "Configured systemd unit file:dlt.service" )
    
    set( DLT_SYSLOG_APPID "SYS" )
    set( DLT_SYSLOG_CTID "LOG" )
    set( DLT_SYSLOG_PORT 4712 )
    message(STATUS "DLT-syslog configuration: APPID:${DLT_SYSLOG_APPID} CTID:${DLT_SYSLOG_CTID} PORT:${DLT_SYSLOG_PORT}" )
    configure_file(${CMAKE_SOURCE_DIR}/systemd/dlt-syslog.service.cmake ${PROJECT_BINARY_DIR}/systemd/dlt-syslog.service)
    install(FILES ${PROJECT_BINARY_DIR}/systemd/dlt-syslog.service DESTINATION ${SYSTEMD_CONFIGURATIONS_FILES_DIR} )
    message( STATUS "Configured systemd unit file:dlt-syslog.service" )
    
endif(WITH_SYSTEMD)