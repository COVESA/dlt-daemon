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

# Source Configuration ========================================================
set(BASE_SRC_DIR "${PROJECT_SOURCE_DIR}/src")

# Target Configuration ========================================================
## Daemon core targets
set(DLT_DAEMON_CORE_TARGETS
    dlt-daemon
    dlt_daemon_client
    dlt_daemon_common
    dlt_daemon_connection
    dlt_daemon_event_handler
    dlt_daemon_offline_logstorage
    dlt_daemon_serial
    dlt_daemon_socket
    dlt_daemon_unix_socket
)

## Shared component targets
set(DLT_SHARED_COMPONENT_TARGETS
    gateway/dlt_gateway
    lib/dlt_client
    shared/dlt_common
    shared/dlt_config_file_parser
    shared/dlt_log
    shared/dlt_multiple_files
    shared/dlt_offline_trace
    shared/dlt_protocol
    shared/dlt_user_shared
    offlinelogstorage/dlt_offline_logstorage
    offlinelogstorage/dlt_offline_logstorage_behavior
)

# Source Resolution ===========================================================
## Convert targets to source files
foreach(TARGET_NAME IN LISTS DLT_DAEMON_CORE_TARGETS)
    set(SRC_FILE "${TARGET_NAME}.c")
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FILE}")
        message(FATAL_ERROR "Missing core source: ${SRC_FILE}")
    endif()
    list(APPEND DLT_DAEMON_CORE_SOURCES "${SRC_FILE}")
endforeach()

foreach(COMPONENT IN LISTS DLT_SHARED_COMPONENT_TARGETS)
    set(SRC_FILE "${BASE_SRC_DIR}/${COMPONENT}.c")
    if(NOT EXISTS "${SRC_FILE}")
        message(FATAL_ERROR "Missing shared component: ${SRC_FILE}")
    endif()
    list(APPEND DLT_SHARED_COMPONENTS "${SRC_FILE}")
endforeach()

# Main Executable Configuration ===============================================
add_executable(dlt-daemon)

## Core sources
target_sources(dlt-daemon PRIVATE
    ${DLT_DAEMON_CORE_SOURCES}
    ${DLT_SHARED_COMPONENTS}
    ${systemd_SRCS}
)

# Optional Components =========================================================
## Shared Memory Support
if(WITH_DLT_SHM_ENABLE)
    set(SHM_TARGET "shared/dlt_shm")
    set(SHM_SOURCE "${BASE_SRC_DIR}/${SHM_TARGET}.c")
    if(NOT EXISTS "${SHM_SOURCE}")
        message(FATAL_ERROR "Missing SHM source: ${SHM_SOURCE}")
    endif()
    target_sources(dlt-daemon PRIVATE "${SHM_SOURCE}")
endif()

## UDP Connection Support
if(WITH_UDP_CONNECTION)
    set(UDP_TARGET "daemon/udp_connection/dlt_daemon_udp_socket")
    set(UDP_SOURCE "${BASE_SRC_DIR}/${UDP_TARGET}.c")
    if(NOT EXISTS "${UDP_SOURCE}")
        message(FATAL_ERROR "Missing UDP source: ${UDP_SOURCE}")
    endif()

    target_include_directories(dlt-daemon PRIVATE
        ${BASE_SRC_DIR}/daemon/udp_connection
    )
    target_compile_definitions(dlt-daemon PRIVATE UDP_CONNECTION_SUPPORT)
    target_sources(dlt-daemon PRIVATE "${UDP_SOURCE}")
endif()