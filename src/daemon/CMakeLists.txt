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


if(WITH_SYSTEMD_WATCHDOG OR WITH_SYSTEMD)
    message(STATUS "Added ${systemd_SRCS} to dlt-daemon")
endif()

set(dlt_daemon_SRCS
    dlt-daemon.c
    dlt_daemon_client.c
    dlt_daemon_common.c
    dlt_daemon_connection.c
    dlt_daemon_event_handler.c
    dlt_daemon_offline_logstorage.c
    dlt_daemon_serial.c
    dlt_daemon_socket.c
    dlt_daemon_unix_socket.c
    ${PROJECT_SOURCE_DIR}/src/gateway/dlt_gateway.c
    ${PROJECT_SOURCE_DIR}/src/lib/dlt_client.c
    ${PROJECT_SOURCE_DIR}/src/shared/dlt_common.c
    ${PROJECT_SOURCE_DIR}/src/shared/dlt_config_file_parser.c
    ${PROJECT_SOURCE_DIR}/src/shared/dlt_multiple_files.c
    ${PROJECT_SOURCE_DIR}/src/shared/dlt_offline_trace.c
    ${PROJECT_SOURCE_DIR}/src/shared/dlt_protocol.c
    ${PROJECT_SOURCE_DIR}/src/shared/dlt_user_shared.c
    ${PROJECT_SOURCE_DIR}/src/offlinelogstorage/dlt_offline_logstorage.c
    ${PROJECT_SOURCE_DIR}/src/offlinelogstorage/dlt_offline_logstorage_behavior.c
    )

if(WITH_DLT_SHM_ENABLE)
    set(dlt_daemon_SRCS
        ${dlt_daemon_SRCS}
        ${PROJECT_SOURCE_DIR}/src/shared/dlt_shm.c)
endif()

if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux|CYGWIN")
    set(RT_LIBRARY rt)
    set(SOCKET_LIBRARY "")
else()
    set(RT_LIBRARY "")
    set(SOCKET_LIBRARY socket)
endif()

if(WITH_UDP_CONNECTION)
    include_directories(${PROJECT_SOURCE_DIR}/src/daemon/udp_connection)
    add_definitions(-DUDP_CONNECTION_SUPPORT)
    set(dlt_daemon_SRCS ${dlt_daemon_SRCS} ${PROJECT_SOURCE_DIR}/src/daemon/udp_connection/dlt_daemon_udp_socket.c)
endif(WITH_UDP_CONNECTION)

add_executable(dlt-daemon ${dlt_daemon_SRCS} ${systemd_SRCS})
target_link_libraries(dlt-daemon ${RT_LIBRARY} ${SOCKET_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
if (WITH_SYSTEMD_SOCKET_ACTIVATION)
    target_link_libraries(dlt-daemon systemd)
endif()
if (WITH_DLT_LOGSTORAGE_GZIP)
	target_link_libraries(dlt-daemon ${ZLIB_LIBRARY})
endif()

install(TARGETS dlt-daemon
        RUNTIME DESTINATION bin
        PERMISSIONS
            OWNER_EXECUTE OWNER_WRITE OWNER_READ
            GROUP_EXECUTE GROUP_READ
            WORLD_EXECUTE WORLD_READ
        COMPONENT base)

if (WITH_DLT_UNIT_TESTS)
    set(library_SRCS ${dlt_daemon_SRCS})

    if (WITH_SYSTEMD_WATCHDOG OR WITH_SYSTEMD)
        set(library_SRCS ${library_SRCS} ${systemd_SRCS})
    endif(WITH_SYSTEMD_WATCHDOG OR WITH_SYSTEMD)

    add_library(dlt_daemon ${library_SRCS})
    target_link_libraries(dlt_daemon ${RT_LIBRARY} ${SOCKET_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

    install(TARGETS dlt_daemon
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
            COMPONENT base)
endif(WITH_DLT_UNIT_TESTS)

INSTALL(FILES dlt.conf
	DESTINATION ${CONFIGURATION_FILES_DIR}
	COMPONENT base)

if (WITH_DLT_LOG_LEVEL_APP_CONFIG)
    INSTALL(FILES dlt-log-levels.conf
            DESTINATION ${CONFIGURATION_FILES_DIR}
            COMPONENT base)
endif()
