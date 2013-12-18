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


if(WITH_SYSTEMD_WATCHDOG OR WITH_SYSTEMD)
    message( STATUS "Added ${systemd_SRCS} to dlt-daemon")
endif(WITH_SYSTEMD_WATCHDOG OR WITH_SYSTEMD)

set(dlt_daemon_SRCS dlt-daemon.c dlt_daemon_common.c dlt_daemon_socket.c dlt_daemon_serial.c ${CMAKE_SOURCE_DIR}/src/shared/dlt_user_shared.c ${CMAKE_SOURCE_DIR}/src/shared/dlt_common.c ${CMAKE_SOURCE_DIR}/src/shared/dlt_shm.c ${CMAKE_SOURCE_DIR}/src/shared/dlt_offline_trace.c)
add_executable(dlt-daemon ${dlt_daemon_SRCS} ${systemd_SRCS})
target_link_libraries(dlt-daemon rt ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS dlt-daemon
	RUNTIME DESTINATION bin
	PERMISSIONS 	OWNER_EXECUTE OWNER_WRITE OWNER_READ
			GROUP_EXECUTE GROUP_READ
			WORLD_EXECUTE WORLD_READ
	COMPONENT base)

INSTALL(FILES 	${CMAKE_CURRENT_SOURCE_DIR}/dlt.conf
	DESTINATION /etc
	COMPONENT base)
