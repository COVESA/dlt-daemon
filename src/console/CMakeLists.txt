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

set(dlt_sortbytimestamp_SRCS dlt-sortbytimestamp.c)
add_executable(dlt-sortbytimestamp ${dlt_sortbytimestamp_SRCS} ${dlt_most_SRCS})
target_link_libraries(dlt-sortbytimestamp dlt ${EXPAT_LIBRARIES})
set_target_properties(dlt-sortbytimestamp PROPERTIES LINKER_LANGUAGE C)

set(dlt_convert_SRCS dlt-convert.c)
add_executable(dlt-convert ${dlt_convert_SRCS} ${dlt_most_SRCS})
target_link_libraries(dlt-convert dlt ${EXPAT_LIBRARIES})
set_target_properties(dlt-convert PROPERTIES LINKER_LANGUAGE C)

set(dlt_receive_SRCS dlt-receive.c)
add_executable(dlt-receive ${dlt_receive_SRCS} ${dlt_most_SRCS})
target_link_libraries(dlt-receive dlt ${EXPAT_LIBRARIES})
set_target_properties(dlt-receive PROPERTIES LINKER_LANGUAGE C)

set(dlt_control_SRCS dlt-control.c dlt-control-common.c)
add_executable(dlt-control ${dlt_control_SRCS} ${dlt_most_SRCS})
target_link_libraries(dlt-control dlt ${EXPAT_LIBRARIES})
set_target_properties(dlt-control PROPERTIES LINKER_LANGUAGE C)

set(dlt_control_common_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/dlt-control-common.c)
add_subdirectory( logstorage )

set(dlt_passive_node_ctrl_SRCS dlt-passive-node-ctrl.c dlt-control-common.c)
add_executable(dlt-passive-node-ctrl ${dlt_passive_node_ctrl_SRCS} ${dlt_most_SRCS})
target_link_libraries(dlt-passive-node-ctrl dlt ${EXPAT_LIBRARIES})

install(TARGETS dlt-sortbytimestamp dlt-convert dlt-receive dlt-control dlt-passive-node-ctrl
	RUNTIME DESTINATION bin
	COMPONENT base)
