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

if (WITH_DLT_ADAPTOR_STDIN OR WITH_DLT_ADAPTOR)
	set(dlt_adaptor_stdin_SRCS dlt-adaptor-stdin.c)
	add_executable(dlt-adaptor-stdin ${dlt_adaptor_stdin_SRCS})
	target_link_libraries(dlt-adaptor-stdin dlt)
	set_target_properties(dlt-adaptor-stdin PROPERTIES LINKER_LANGUAGE C)

	install(TARGETS dlt-adaptor-stdin
		RUNTIME DESTINATION bin
		COMPONENT base)
endif()

if (WITH_DLT_ADAPTOR_UDP OR WITH_DLT_ADAPTOR)
	set(dlt_adaptor_udp_SRCS dlt-adaptor-udp.c)
	add_executable(dlt-adaptor-udp ${dlt_adaptor_udp_SRCS})
	target_link_libraries(dlt-adaptor-udp dlt)
	set_target_properties(dlt-adaptor-udp PROPERTIES LINKER_LANGUAGE C)

	install(TARGETS dlt-adaptor-udp
		RUNTIME DESTINATION bin
		COMPONENT base)
endif()
