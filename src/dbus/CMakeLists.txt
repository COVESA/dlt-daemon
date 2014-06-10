#######
# Dlt - Diagnostic Log and Trace
# @licence make begin@
#
# Copyright (C) 2011-2014, BMW AG - Alexander Wenzel <alexander.aw.wenzel@bmw.de>
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

include_directories( 
    ${CMAKE_SOURCE_DIR}/
    ${CMAKE_SOURCE_DIR}/include/dlt
    ${DBUS_INCLUDE_DIRS}
)

set(dlt_dbus_SRCS dlt-dbus.c dlt-dbus-options.c)
add_executable(dlt-dbus ${dlt_dbus_SRCS})
target_link_libraries(dlt-dbus dlt ${DBUS_LIBRARIES})
	
set_target_properties(dlt-dbus PROPERTIES LINKER_LANGUAGE C)

install(TARGETS dlt-dbus
	RUNTIME DESTINATION bin
	COMPONENT base)

INSTALL(FILES 	${CMAKE_CURRENT_SOURCE_DIR}/dlt-dbus.conf
	DESTINATION /etc
	COMPONENT base)
