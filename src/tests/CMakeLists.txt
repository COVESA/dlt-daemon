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

# Test target configuration ===================================================
# Base test targets
set(TEST_BASE_TARGETS
    dlt-test-multi-process
    dlt-test-multi-process-client
    dlt-test-user
    dlt-test-client
    dlt-test-stress-user
    dlt-test-stress-client
    dlt-test-stress
    dlt-test-fork-handler
    dlt-test-init-free
    dlt-test-preregister-context
    dlt-test-filetransfer
)

# Conditional test targets
set(TEST_CONDITIONAL_TARGETS)
if(WITH_DLT_CXX11_EXT)
    list(APPEND TEST_CONDITIONAL_TARGETS dlt-test-cpp-extension)
endif()

if(NOT WITH_DLT_DISABLE_MACRO)
    list(APPEND TEST_CONDITIONAL_TARGETS dlt-test-non-verbose)
endif()

if(WITH_DLT_QNX_SYSTEM)
    list(APPEND TEST_CONDITIONAL_TARGETS dlt-test-qnx-slogger)
endif()

# Combine all test targets
set(TEST_TARGETS_ALL
    ${TEST_BASE_TARGETS}
    ${TEST_CONDITIONAL_TARGETS}
)

# File installation configuration
set(TEST_DATA_FILES
    dlt-test-filetransfer-file
    dlt-test-filetransfer-image.png
)

install(FILES ${TEST_DATA_FILES}
    DESTINATION share/dlt-filetransfer
)

# Test target construction ====================================================
foreach(TEST_TARGET IN LISTS TEST_TARGETS_ALL)
    # Source file resolution
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_TARGET}.c")
        set(SOURCE_FILE "${TEST_TARGET}.c")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_TARGET}.cpp")
        set(SOURCE_FILE "${TEST_TARGET}.cpp")
    else()
        message(FATAL_ERROR "Missing source for ${TEST_TARGET} - checked .c and .cpp extensions")
    endif()

    # Target creation
    add_executable(${TEST_TARGET} ${SOURCE_FILE})

    # Language configuration
    if(SOURCE_FILE MATCHES "\\.cpp$")
        set_target_properties(${TEST_TARGET} PROPERTIES LINKER_LANGUAGE CXX)
    else()
        set_target_properties(${TEST_TARGET} PROPERTIES LINKER_LANGUAGE C)
    endif()

    # Dependencies
    target_link_libraries(${TEST_TARGET} PRIVATE dlt)

    # Installation
    install(TARGETS ${TEST_TARGET}
        RUNTIME DESTINATION bin
        COMPONENT base
    )
endforeach()