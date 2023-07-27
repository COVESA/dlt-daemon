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

include_directories(
    ${PROJECT_SOURCE_DIR}/
    ${PROJECT_SOURCE_DIR}/include/dlt
    ${DBUS_INCLUDE_DIRS}
)

set(dlt_dbus_SRCS dlt-dbus.c dlt-dbus-options.c)
add_executable(dlt-dbus ${dlt_dbus_SRCS})
target_link_libraries(dlt-dbus dlt ${DBUS_LIBRARIES})

set_target_properties(dlt-dbus PROPERTIES LINKER_LANGUAGE C)

install(TARGETS dlt-dbus
	RUNTIME DESTINATION bin
	COMPONENT base)

INSTALL(FILES dlt-dbus.conf
	DESTINATION ${CONFIGURATION_FILES_DIR}
	COMPONENT base)
