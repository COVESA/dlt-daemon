#######
# Dlt - Diagnostic Log and Trace
# @licence make begin@
#
# Copyright (C) 2011-2012, BMW AG - Alexander Wenzel <alexander.aw.wenzel@bmw.de>
# 
# Contributions are licensed to the GENIVI Alliance under one or more
# Contribution License Agreements.
# 
# This Source Code Form is subject to the terms of the
# Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
# this file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# @licence end@
########
set(dlt_test_queue_SRCS dlt-test-queue)
add_executable(dlt-test-queue ${dlt_test_queue_SRCS})
target_link_libraries(dlt-test-queue dlt)
set_target_properties(dlt-test-queue PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_multi_process_SRCS dlt-test-multi-process)
add_executable(dlt-test-multi-process ${dlt_test_multi_process_SRCS})
target_link_libraries(dlt-test-multi-process dlt)
set_target_properties(dlt-test-multi-process PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_multi_process_client_SRCS dlt-test-multi-process-client)
add_executable(dlt-test-multi-process-client ${dlt_test_multi_process_client_SRCS})
target_link_libraries(dlt-test-multi-process-client dlt)
set_target_properties(dlt-test-multi-process-client PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_user_SRCS dlt-test-user)
add_executable(dlt-test-user ${dlt_test_user_SRCS})
target_link_libraries(dlt-test-user dlt)
set_target_properties(dlt-test-user PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_client_SRCS dlt-test-client)
add_executable(dlt-test-client ${dlt_test_client_SRCS})
target_link_libraries(dlt-test-client dlt)
set_target_properties(dlt-test-client PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_stress_user_SRCS dlt-test-stress-user)
add_executable(dlt-test-stress-user ${dlt_test_stress_user_SRCS})
target_link_libraries(dlt-test-stress-user dlt)
set_target_properties(dlt-test-stress-user PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_stress_client_SRCS dlt-test-stress-client)
add_executable(dlt-test-stress-client ${dlt_test_stress_client_SRCS})
target_link_libraries(dlt-test-stress-client dlt)
set_target_properties(dlt-test-stress-client PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_stress_SRCS dlt-test-stress)
add_executable(dlt-test-stress ${dlt_test_stress_SRCS})
target_link_libraries(dlt-test-stress dlt)
set_target_properties(dlt-test-stress PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_filetransfer_SRCS dlt-test-filetransfer)
add_executable(dlt-test-filetransfer ${dlt_test_filetransfer_SRCS})
target_link_libraries(dlt-test-filetransfer dlt)
set_target_properties(dlt-test-filetransfer PROPERTIES LINKER_LANGUAGE C)

install(TARGETS dlt-test-queue dlt-test-multi-process dlt-test-multi-process-client dlt-test-user dlt-test-client dlt-test-stress-user dlt-test-stress-client dlt-test-stress dlt-test-filetransfer	
	RUNTIME DESTINATION bin
	COMPONENT base)

INSTALL(FILES dlt-test-filetransfer-file dlt-test-filetransfer-image.png
	DESTINATION /usr/share/dlt-filetransfer
)
