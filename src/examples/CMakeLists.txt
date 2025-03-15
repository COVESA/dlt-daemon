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

# Example targets configuration ===============================================
# Base example targets
set(EXAMPLE_BASE_TARGETS
    dlt-example-user-func
    dlt-example-filetransfer
)

# Conditional example targets
set(EXAMPLE_CONDITIONAL_TARGETS)
if(NOT WITH_DLT_DISABLE_MACRO)
    list(APPEND EXAMPLE_CONDITIONAL_TARGETS
        dlt-example-user
        dlt-example-user-common-api
    )
endif()

if(WITH_UDP_CONNECTION)
    list(APPEND EXAMPLE_CONDITIONAL_TARGETS
        dlt-example-multicast-clientmsg-view
    )
endif()

# Combine all example targets
set(EXAMPLE_TARGETS_ALL
    ${EXAMPLE_BASE_TARGETS}
    ${EXAMPLE_CONDITIONAL_TARGETS}
)

# Example target construction =================================================
foreach(EXAMPLE_TARGET IN LISTS EXAMPLE_TARGETS_ALL)
    # Source file validation
    set(SOURCE_FILE "${EXAMPLE_TARGET}.c")
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}")
        message(FATAL_ERROR "Missing C source for ${EXAMPLE_TARGET} - ${SOURCE_FILE} not found")
    endif()

    # Target creation
    add_executable(${EXAMPLE_TARGET} ${SOURCE_FILE})

    # Dependencies and properties
    target_link_libraries(${EXAMPLE_TARGET} PRIVATE dlt)
    set_target_properties(${EXAMPLE_TARGET} PROPERTIES LINKER_LANGUAGE C)

    # Installation
    install(TARGETS ${EXAMPLE_TARGET}
        RUNTIME DESTINATION bin
        COMPONENT base
    )
endforeach()
