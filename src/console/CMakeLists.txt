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

set(dlt_control_common_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/dlt-control-common.c)
add_library(dlt_control_common_lib STATIC ${dlt_control_common_SRCS})
target_link_libraries(dlt_control_common_lib dlt ${DLT_JSON_LIBRARY})

add_subdirectory(logstorage)

set(dlt-sortbytimestamp_SRCS dlt-sortbytimestamp.c)
set(dlt-convert_SRCS dlt-convert.c)
set(dlt-receive_SRCS dlt-receive.c)
set(dlt-control_SRCS dlt-control.c ${dlt_control_common_SRCS})
set(dlt-passive-node-ctrl_SRCS dlt-passive-node-ctrl.c ${dlt_control_common_SRCS})

foreach(target
        dlt-sortbytimestamp
        dlt-convert
        dlt-receive
        dlt-control
        dlt-passive-node-ctrl
        )
    add_executable(${target} ${${target}_SRCS})
    target_link_libraries(${target} dlt dlt_control_common_lib)
    set_target_properties(${target} PROPERTIES LINKER_LANGUAGE C)

    install(TARGETS ${target}
            RUNTIME DESTINATION bin
            COMPONENT base)
endforeach()
