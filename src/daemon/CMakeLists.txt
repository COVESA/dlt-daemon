#######
# @licence make begin@
# SPDX license identifier: MPL-2.0
#
# Copyright (C) 2011-2015, BMW AG
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


if(WITH_SYSTEMD_WATCHDOG OR WITH_SYSTEMD)
    message( STATUS "Added ${systemd_SRCS} to dlt-daemon")
endif(WITH_SYSTEMD_WATCHDOG OR WITH_SYSTEMD)

set(dlt_daemon_SRCS dlt-daemon.c dlt_daemon_common.c dlt_daemon_connection.c dlt_daemon_event_handler.c ${CMAKE_SOURCE_DIR}/src/gateway/dlt_gateway.c dlt_daemon_socket.c dlt_daemon_unix_socket.c dlt_daemon_serial.c dlt_daemon_client.c dlt_daemon_offline_logstorage.c ${CMAKE_SOURCE_DIR}/src/shared/dlt_user_shared.c ${CMAKE_SOURCE_DIR}/src/shared/dlt_common.c ${CMAKE_SOURCE_DIR}/src/shared/dlt_shm.c ${CMAKE_SOURCE_DIR}/src/shared/dlt_offline_trace.c ${CMAKE_SOURCE_DIR}/src/offlinelogstorage/dlt_offline_logstorage.c ${CMAKE_SOURCE_DIR}/src/lib/dlt_client.c ${CMAKE_SOURCE_DIR}/src/shared/dlt_config_file_parser.c ${CMAKE_SOURCE_DIR}/src/offlinelogstorage/dlt_offline_logstorage_behavior.c)
add_executable(dlt-daemon ${dlt_daemon_SRCS} ${systemd_SRCS})
target_link_libraries(dlt-daemon rt ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS dlt-daemon
	RUNTIME DESTINATION bin
	PERMISSIONS 	OWNER_EXECUTE OWNER_WRITE OWNER_READ
			GROUP_EXECUTE GROUP_READ
			WORLD_EXECUTE WORLD_READ
	COMPONENT base)

if (WITH_DLT_UNIT_TESTS)
    add_library(dlt_daemon ${dlt_daemon_SRCS})
    target_link_libraries(dlt_daemon rt ${CMAKE_THREAD_LIBS_INIT})
    install(TARGETS dlt_daemon
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
            COMPONENT base)
endif(WITH_DLT_UNIT_TESTS)

INSTALL(FILES dlt.conf
	DESTINATION ${CONFIGURATION_FILES_DIR}
	COMPONENT base)

