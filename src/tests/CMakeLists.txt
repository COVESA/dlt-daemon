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

set(dlt_test_multi_process_SRCS dlt-test-multi-process.c)
add_executable(dlt-test-multi-process ${dlt_test_multi_process_SRCS})
target_link_libraries(dlt-test-multi-process dlt)
set_target_properties(dlt-test-multi-process PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_multi_process_client_SRCS dlt-test-multi-process-client.c)
add_executable(dlt-test-multi-process-client ${dlt_test_multi_process_client_SRCS})
target_link_libraries(dlt-test-multi-process-client dlt)
set_target_properties(dlt-test-multi-process-client PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_user_SRCS dlt-test-user.c)
add_executable(dlt-test-user ${dlt_test_user_SRCS})
target_link_libraries(dlt-test-user dlt)
set_target_properties(dlt-test-user PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_client_SRCS dlt-test-client.c)
add_executable(dlt-test-client ${dlt_test_client_SRCS})
target_link_libraries(dlt-test-client dlt)
set_target_properties(dlt-test-client PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_stress_user_SRCS dlt-test-stress-user.c)
add_executable(dlt-test-stress-user ${dlt_test_stress_user_SRCS})
target_link_libraries(dlt-test-stress-user dlt)
set_target_properties(dlt-test-stress-user PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_stress_client_SRCS dlt-test-stress-client.c)
add_executable(dlt-test-stress-client ${dlt_test_stress_client_SRCS})
target_link_libraries(dlt-test-stress-client dlt)
set_target_properties(dlt-test-stress-client PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_stress_SRCS dlt-test-stress.c)
add_executable(dlt-test-stress ${dlt_test_stress_SRCS})
target_link_libraries(dlt-test-stress dlt)
set_target_properties(dlt-test-stress PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_filetransfer_SRCS dlt-test-filetransfer.c)
add_executable(dlt-test-filetransfer ${dlt_test_filetransfer_SRCS})
target_link_libraries(dlt-test-filetransfer dlt)
set_target_properties(dlt-test-filetransfer PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_fork_handler_SRCS dlt-test-fork-handler.c)
add_executable(dlt-test-fork-handler ${dlt_test_fork_handler_SRCS})
target_link_libraries(dlt-test-fork-handler dlt)
set_target_properties(dlt-test-fork-handler PROPERTIES LINKER_LANGUAGE C)

install(TARGETS dlt-test-multi-process dlt-test-multi-process-client dlt-test-user dlt-test-client dlt-test-stress-user dlt-test-stress-client dlt-test-stress dlt-test-filetransfer dlt-test-fork-handler
	RUNTIME DESTINATION bin
	COMPONENT base)

INSTALL(FILES dlt-test-filetransfer-file dlt-test-filetransfer-image.png
	DESTINATION share/dlt-filetransfer
)
