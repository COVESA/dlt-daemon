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

set(TARGET_LIST ${TARGET_LIST} dlt-test-multi-process)
set(TARGET_LIST ${TARGET_LIST} dlt-test-multi-process-client)
set(TARGET_LIST ${TARGET_LIST} dlt-test-user)
set(TARGET_LIST ${TARGET_LIST} dlt-test-client)
set(TARGET_LIST ${TARGET_LIST} dlt-test-stress-user)
set(TARGET_LIST ${TARGET_LIST} dlt-test-stress-client)
set(TARGET_LIST ${TARGET_LIST} dlt-test-stress)
set(TARGET_LIST ${TARGET_LIST} dlt-test-fork-handler)
set(TARGET_LIST ${TARGET_LIST} dlt-test-init-free)
set(TARGET_LIST ${TARGET_LIST} dlt-test-preregister-context)
set(TARGET_LIST ${TARGET_LIST} dlt-test-filetransfer)
install(FILES dlt-test-filetransfer-file dlt-test-filetransfer-image.png
        DESTINATION share/dlt-filetransfer)

if(WITH_DLT_CXX11_EXT)
    set(TARGET_LIST ${TARGET_LIST} dlt-test-cpp-extension)
endif()

#TODO: Enable again once dlt-test-non-verbose is adapted to non-macro usage
if (NOT WITH_DLT_DISABLE_MACRO)
    set(TARGET_LIST ${TARGET_LIST} dlt-test-non-verbose)
    if(WITH_DLT_CXX11_EXT)
        set(TARGET_LIST ${TARGET_LIST} dlt-test-cpp-extension)
    endif()
endif()

if(WITH_DLT_QNX_SYSTEM)
    set(TARGET_LIST ${TARGET_LIST} dlt-test-qnx-slogger)
endif()
 
foreach(TARGET IN LISTS TARGET_LIST)
    set(TARGET_SRCS ${TARGET})
    add_executable(${TARGET} ${TARGET_SRCS})
    target_link_libraries(${TARGET} dlt)
    set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE C)
    install(TARGETS ${TARGET}
            RUNTIME DESTINATION bin
            COMPONENT base)
endforeach()
