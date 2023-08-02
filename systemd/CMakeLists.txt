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

if(WITH_SYSTEMD)

    set(SYSTEMD_CONFIGURATIONS_FILES_DIR ${SYSTEMD_UNITDIR} )

    if(WITH_SYSTEMD_WATCHDOG)
        set( DLT_WatchdogSec 2 )
        message( STATUS "The systemd watchdog is enabled - timeout is set to ${DLT_WatchdogSec} seconds")
    else(WITH_SYSTEMD_WATCHDOG)
        set( DLT_WatchdogSec 0 )
        message( STATUS "The systemd watchdog is disabled")
    endif(WITH_SYSTEMD_WATCHDOG)

    configure_file(${PROJECT_SOURCE_DIR}/systemd/dlt.service.cmake ${PROJECT_BINARY_DIR}/systemd/dlt.service)
    message( STATUS "Configured systemd unit file:dlt.service" )
    install(FILES ${PROJECT_BINARY_DIR}/systemd/dlt.service DESTINATION ${SYSTEMD_CONFIGURATIONS_FILES_DIR} )

    if(WITH_DLT_SYSTEM)
        configure_file(${PROJECT_SOURCE_DIR}/systemd/dlt-system.service.cmake ${PROJECT_BINARY_DIR}/systemd/dlt-system.service)
        message( STATUS "Configured systemd unit file:dlt-system.service" )
        install(FILES ${PROJECT_BINARY_DIR}/systemd/dlt-system.service DESTINATION ${SYSTEMD_CONFIGURATIONS_FILES_DIR} )
    endif(WITH_DLT_SYSTEM)

    if(WITH_DLT_DBUS)
        configure_file(${PROJECT_SOURCE_DIR}/systemd/dlt-dbus.service.cmake ${PROJECT_BINARY_DIR}/systemd/dlt-dbus.service)
        message( STATUS "Configured systemd unit file:dlt-dbus.service" )
        install(FILES ${PROJECT_BINARY_DIR}/systemd/dlt-dbus.service DESTINATION ${SYSTEMD_CONFIGURATIONS_FILES_DIR} )
    endif(WITH_DLT_DBUS)

    if(WITH_DLT_CONSOLE AND WITH_DLT_EXAMPLES)
        configure_file(${PROJECT_SOURCE_DIR}/systemd/dlt-receive.service.cmake ${PROJECT_BINARY_DIR}/systemd/dlt-receive.service)
        message( STATUS "Configured systemd unit file:dlt-receive.service" )
        install(FILES ${PROJECT_BINARY_DIR}/systemd/dlt-receive.service DESTINATION ${SYSTEMD_CONFIGURATIONS_FILES_DIR} )
    endif(WITH_DLT_CONSOLE AND WITH_DLT_EXAMPLES)

    if(WITH_DLT_EXAMPLES)
        configure_file(${PROJECT_SOURCE_DIR}/systemd/dlt-example-user.service.cmake ${PROJECT_BINARY_DIR}/systemd/dlt-example-user.service)
        message( STATUS "Configured systemd unit file:dlt-example-user.service" )
        install(FILES ${PROJECT_BINARY_DIR}/systemd/dlt-example-user.service DESTINATION ${SYSTEMD_CONFIGURATIONS_FILES_DIR} )
    endif(WITH_DLT_EXAMPLES)

    if(WITH_DLT_ADAPTOR_UDP OR WITH_DLT_ADAPTOR)
        set( DLT_ADAPTOR_UDP_APPID "DUDP" )
        set( DLT_ADAPTOR_UDP_CTID "DCTI" )
        set( DLT_ADAPTOR_UDP_PORT 4712 )
        configure_file(${PROJECT_SOURCE_DIR}/systemd/dlt-adaptor-udp.service.cmake ${PROJECT_BINARY_DIR}/systemd/dlt-adaptor-udp.service)
        message( STATUS "Configured systemd unit file:dlt-adaptor-udp.service" )
        message(STATUS "DLT adaptor udp configuration: APPID=${DLT_ADAPTOR_UDP_APPID} CTID=${DLT_ADAPTOR_UDP_CTID} PORT=${DLT_ADAPTOR_UDP_PORT}" )
        install(FILES ${PROJECT_BINARY_DIR}/systemd/dlt-adaptor-udp.service DESTINATION ${SYSTEMD_CONFIGURATIONS_FILES_DIR} )
    endif(WITH_DLT_ADAPTOR_UDP)

    if (WITH_SYSTEMD_SOCKET_ACTIVATION)
        configure_file(${PROJECT_SOURCE_DIR}/systemd/dlt.socket.cmake ${PROJECT_BINARY_DIR}/systemd/dlt.socket)
        message( STATUS "Configured systemd socket file:dlt.socket" )
        install(FILES ${PROJECT_BINARY_DIR}/systemd/dlt.socket DESTINATION ${SYSTEMD_CONFIGURATIONS_FILES_DIR} )
    endif()

    message(STATUS "Unit files will be installed to ${SYSTEMD_CONFIGURATIONS_FILES_DIR} after make install" )

endif(WITH_SYSTEMD)
