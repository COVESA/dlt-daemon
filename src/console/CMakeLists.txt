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

set(dlt_convert_SRCS dlt-convert.c)
add_executable(dlt-convert ${dlt_convert_SRCS} ${dlt_most_SRCS})
target_link_libraries(dlt-convert dlt ${EXPAT_LIBRARIES})
set_target_properties(dlt-convert PROPERTIES LINKER_LANGUAGE C)

set(dlt_receive_SRCS dlt-receive.c)
add_executable(dlt-receive ${dlt_receive_SRCS} ${dlt_most_SRCS})
target_link_libraries(dlt-receive dlt ${EXPAT_LIBRARIES})
set_target_properties(dlt-receive PROPERTIES LINKER_LANGUAGE C)

set(dlt_control_SRCS dlt-control.c)
add_executable(dlt-control ${dlt_control_SRCS} ${dlt_most_SRCS})
target_link_libraries(dlt-control dlt ${EXPAT_LIBRARIES})
set_target_properties(dlt-control PROPERTIES LINKER_LANGUAGE C)

install(TARGETS dlt-convert dlt-receive dlt-control
	RUNTIME DESTINATION bin
	COMPONENT base)

