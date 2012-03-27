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

set(dlt_adaptor_stdin_SRCS dlt-adaptor-stdin)
add_executable(dlt-adaptor-stdin ${dlt_adaptor_stdin_SRCS})
target_link_libraries(dlt-adaptor-stdin dlt)
set_target_properties(dlt-adaptor-stdin PROPERTIES LINKER_LANGUAGE C)

set(dlt_adaptor_udp_SRCS dlt-adaptor-udp)
add_executable(dlt-adaptor-udp ${dlt_adaptor_udp_SRCS})
target_link_libraries(dlt-adaptor-udp dlt)
set_target_properties(dlt-adaptor-udp PROPERTIES LINKER_LANGUAGE C)

install(TARGETS dlt-adaptor-stdin dlt-adaptor-udp
	RUNTIME DESTINATION bin
	COMPONENT base)


