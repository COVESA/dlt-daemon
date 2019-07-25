#######
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
#######

set(dlt_LIB_SRCS
    dlt_user.c
    dlt_client.c
    dlt_filetransfer.c
    dlt_env_ll.c
    ${PROJECT_SOURCE_DIR}/src/shared/dlt_common.c
    ${PROJECT_SOURCE_DIR}/src/shared/dlt_protocol.c
    ${PROJECT_SOURCE_DIR}/src/shared/dlt_user_shared.c
    )

if(WITH_DLT_SHM_ENABLE)
    set(dlt_LIB_SRCS ${dlt_LIB_SRCS} ${PROJECT_SOURCE_DIR}/src/shared/dlt_shm.c)
endif()

add_library(dlt ${dlt_LIB_SRCS})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(RT_LIBRARY rt)
    set(SOCKET_LIBRARY "")
else()
    set(RT_LIBRARY "")
    set(SOCKET_LIBRARY socket)
endif()

if(HAVE_FUNC_PTHREAD_SETNAME_NP)
    add_definitions(-DDLT_USE_PTHREAD_SETNAME_NP)
    message(STATUS "Using pthread_setname_np API to set thread name")
else()
    message(STATUS "pthread_setname_np API not available on this platform")
endif()

target_link_libraries(dlt ${RT_LIBRARY} ${SOCKET_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(dlt PROPERTIES VERSION ${DLT_VERSION} SOVERSION ${DLT_MAJOR_VERSION})

install(TARGETS dlt
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
        COMPONENT base)
