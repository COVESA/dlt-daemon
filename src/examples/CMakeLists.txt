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

set(TARGET_LIST dlt-example-user-func)
set(TARGET_LIST ${TARGET_LIST} dlt-example-filetransfer)
if(NOT WITH_DLT_DISABLE_MACRO)
    set(TARGET_LIST ${TARGET_LIST} dlt-example-user)
    set(TARGET_LIST ${TARGET_LIST} dlt-example-user-common-api)
endif()

if(WITH_UDP_CONNECTION)
    set(TARGET_LIST ${TARGET_LIST} dlt-example-multicast-clientmsg-view)
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
