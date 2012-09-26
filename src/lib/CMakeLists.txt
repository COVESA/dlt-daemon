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
########q

set(dlt_LIB_SRCS dlt_user dlt_client dlt_filetransfer ${CMAKE_SOURCE_DIR}/src/shared/dlt_common.c ${CMAKE_SOURCE_DIR}/src/shared/dlt_user_shared.c ${CMAKE_SOURCE_DIR}/src/shared/dlt_shm.c)

add_library(dlt ${dlt_LIB_SRCS})
target_link_libraries(dlt rt ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(dlt PROPERTIES VERSION ${DLT_VERSION} SOVERSION ${DLT_MAJOR_VERSION})

install(TARGETS dlt
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib/static
	COMPONENT base)


