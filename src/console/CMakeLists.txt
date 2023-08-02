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

set(dlt_control_common_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/dlt-control-common.c)
add_library(dlt_control_common_lib STATIC ${dlt_control_common_SRCS})
target_link_libraries(dlt_control_common_lib dlt ${DLT_JSON_LIBRARY})

set(TARGET_LIST "")

if (WITH_DLT_CONSOLE_RECEIVE)
    list(APPEND TARGET_LIST dlt-receive)
endif()

if (WITH_DLT_CONSOLE_CONVERT)
    list(APPEND TARGET_LIST dlt-convert)
endif()

if(NOT WITH_DLT_CONSOLE_WO_CTRL)
    add_subdirectory(logstorage)
    if (WITH_DLT_CONSOLE_CONTROL)
        list(APPEND TARGET_LIST dlt-control)
    endif()
    if (WITH_DLT_CONSOLE_PASSIVE_NODE_CTRL)
        list(APPEND TARGET_LIST dlt-passive-node-ctrl)
    endif()
endif()

if(NOT WITH_DLT_CONSOLE_WO_SBTM)
    list(APPEND TARGET_LIST dlt-sortbytimestamp)
endif()

foreach(target IN LISTS TARGET_LIST)
    set(target_SRCS ${target})
    add_executable(${target} ${target_SRCS})
    target_link_libraries(${target} dlt dlt_control_common_lib)
    set_target_properties(${target} PROPERTIES LINKER_LANGUAGE C)

    install(TARGETS ${target}
            RUNTIME DESTINATION bin
            COMPONENT base)
endforeach()
