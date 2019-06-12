#######
# @licence make begin@
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
# @licence end@
#######

set(dlt-test-multi-process_SRCS dlt-test-multi-process.c)
set(dlt-test-multi-process-client_SRCS dlt-test-multi-process-client.c)
set(dlt-test-user_SRCS dlt-test-user.c)
set(dlt-test-client_SRCS dlt-test-client.c)
set(dlt-test-stress-user_SRCS dlt-test-stress-user.c)
set(dlt-test-stress-client_SRCS dlt-test-stress-client.c)
set(dlt-test-stress_SRCS dlt-test-stress.c)
set(dlt-test-filetransfer_SRCS dlt-test-filetransfer.c)
set(dlt-test-fork-handler_SRCS dlt-test-fork-handler.c)
set(dlt-test-init-free_SRCS dlt-test-init-free.c)

foreach(target
        dlt-test-multi-process
        dlt-test-multi-process-client
        dlt-test-user
        dlt-test-client
        dlt-test-stress-user
        dlt-test-stress-client
        dlt-test-stress
        dlt-test-filetransfer
        dlt-test-fork-handler
        dlt-test-init-free
        )
    add_executable(${target} ${${target}_SRCS})
    target_link_libraries(${target} dlt)
    set_target_properties(${target} PROPERTIES LINKER_LANGUAGE C)
    install(TARGETS ${target}
            RUNTIME DESTINATION bin
            COMPONENT base)
endforeach()

install(FILES dlt-test-filetransfer-file dlt-test-filetransfer-image.png
        DESTINATION share/dlt-filetransfer)
