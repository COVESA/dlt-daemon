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

set(dlt_LIB_SRCS
    dlt_user.c
    dlt_client.c
    dlt_filetransfer.c
    dlt_env_ll.c
    ${PROJECT_SOURCE_DIR}/src/shared/dlt_common.c
    ${PROJECT_SOURCE_DIR}/src/shared/dlt_multiple_files.c
    ${PROJECT_SOURCE_DIR}/src/shared/dlt_protocol.c
    ${PROJECT_SOURCE_DIR}/src/shared/dlt_user_shared.c
    )

if(WITH_DLT_SHM_ENABLE)
    set(dlt_LIB_SRCS ${dlt_LIB_SRCS} ${PROJECT_SOURCE_DIR}/src/shared/dlt_shm.c)
endif()

add_library(dlt ${dlt_LIB_SRCS})

if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux|CYGWIN")
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

target_link_libraries(dlt ${RT_LIBRARY} ${SOCKET_LIBRARY} Threads::Threads)

target_include_directories(dlt
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/dlt>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include/dlt>
        $<INSTALL_INTERFACE:include>
)
# With this option set, a legacy include path is added in addition to the regular one.
if(WITH_LEGACY_INCLUDE_PATH)
    target_include_directories(dlt
        PUBLIC
            $<INSTALL_INTERFACE:include/dlt>
    )
endif()

if(WITH_LIB_SHORT_VERSION)
    set_target_properties(dlt PROPERTIES VERSION ${PROJECT_VERSION_MAJOR} SOVERSION ${PROJECT_VERSION_MAJOR})
else()
    set_target_properties(dlt PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})
endif()

add_library(Genivi::dlt ALIAS dlt)

install(TARGETS dlt
    EXPORT
        ${PROJECT_NAME}-targets
    RUNTIME
        DESTINATION bin
        COMPONENT base
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT base
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
        COMPONENT base)

# Install *-targets.cmake file
install(
    EXPORT ${PROJECT_NAME}-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    NAMESPACE Genivi::
)

# Create *-config.cmake file
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}-config.cmake.in
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    NO_SET_AND_CHECK_MACRO
)

# Install *-config.cmake file
install(
    FILES
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
