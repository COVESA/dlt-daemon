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

# Control Common Library ======================================================
set(CONTROL_COMMON_TARGET dlt-control-common)
set(CONTROL_COMMON_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/${CONTROL_COMMON_TARGET}.c")

if(NOT EXISTS "${CONTROL_COMMON_SOURCE}")
    message(FATAL_ERROR "Missing control common source: ${CONTROL_COMMON_SOURCE}")
endif()

add_library(dlt_control_common_lib STATIC "${CONTROL_COMMON_SOURCE}")
target_link_libraries(dlt_control_common_lib PRIVATE dlt ${DLT_JSON_LIBRARY})

# Console Targets Configuration ===============================================
set(CONSOLE_BASE_TARGETS)
set(CONSOLE_CONDITIONAL_TARGETS)

## Conditional targets
if(WITH_DLT_CONSOLE_RECEIVE)
    list(APPEND CONSOLE_CONDITIONAL_TARGETS dlt-receive)
endif()

if(WITH_DLT_CONSOLE_CONVERT)
    list(APPEND CONSOLE_CONDITIONAL_TARGETS dlt-convert)
endif()

if(NOT WITH_DLT_CONSOLE_WO_CTRL)
    add_subdirectory(logstorage)

    if(WITH_DLT_CONSOLE_CONTROL)
        list(APPEND CONSOLE_CONDITIONAL_TARGETS dlt-control)
    endif()

    if(WITH_DLT_CONSOLE_PASSIVE_NODE_CTRL)
        list(APPEND CONSOLE_CONDITIONAL_TARGETS dlt-passive-node-ctrl)
    endif()
endif()

if(NOT WITH_DLT_CONSOLE_WO_SBTM)
    list(APPEND CONSOLE_CONDITIONAL_TARGETS dlt-sortbytimestamp)
endif()

# Combine all console targets
set(CONSOLE_TARGETS_ALL
    ${CONSOLE_BASE_TARGETS}
    ${CONSOLE_CONDITIONAL_TARGETS}
)

# Console Target Construction =================================================
foreach(CONSOLE_TARGET IN LISTS CONSOLE_TARGETS_ALL)
    # Source file validation
    set(SOURCE_FILE "${CONSOLE_TARGET}.c")
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}")
        message(FATAL_ERROR "Missing console source: ${SOURCE_FILE}")
    endif()

    # Target configuration
    add_executable(${CONSOLE_TARGET} "${SOURCE_FILE}")
    target_link_libraries(${CONSOLE_TARGET} PRIVATE
        dlt
        dlt_control_common_lib
    )
    set_target_properties(${CONSOLE_TARGET} PROPERTIES LINKER_LANGUAGE C)

    # Installation
    install(TARGETS ${CONSOLE_TARGET}
        RUNTIME DESTINATION bin
        COMPONENT base
    )
endforeach()
