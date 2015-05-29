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

