#######
# @licence make begin@
# SPDX license identifier: MPL-2.0
#
# Copyright (C) 2011-2015, BMW AG"
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

set(dlt_LIB_SRCS dlt_user dlt_client dlt_filetransfer ${CMAKE_SOURCE_DIR}/src/shared/dlt_common.c ${CMAKE_SOURCE_DIR}/src/shared/dlt_user_shared.c ${CMAKE_SOURCE_DIR}/src/shared/dlt_shm.c)

add_library(dlt ${dlt_LIB_SRCS})
target_link_libraries(dlt rt ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(dlt PROPERTIES VERSION ${DLT_VERSION} SOVERSION ${DLT_MAJOR_VERSION})

install(TARGETS dlt
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION ${LIB_INSTALL_DIR}
	ARCHIVE DESTINATION ${LIB_INSTALL_DIR}/static
	COMPONENT base)


