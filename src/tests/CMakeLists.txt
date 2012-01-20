#######
# Dlt - Diagnostic Log and Trace
# @licence make begin@
 #
 # Copyright (C) 2011, BMW AG - Alexander Wenzel <alexander.wenzel@bmw.de>
 # 
 # This program is free software; you can redistribute it and/or modify it under the terms of the 
 # GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 # This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even 
 # the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General 
 # Public License, version 2.1, for more details.
 # 
 # You should have received a copy of the GNU Lesser General Public License, version 2.1, along 
 # with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 # 
 # Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may 
 # also be applicable to programs even in cases in which the program is not a library in the technical sense.
 # 
 # Linking DLT statically or dynamically with other modules is making a combined work based on DLT. You may 
 # license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to 
 # license your linked modules under the GNU Lesser General Public License, version 2.1, you 
 # may use the program under the following exception.
 # 
 # As a special exception, the copyright holders of DLT give you permission to combine DLT 
 # with software programs or libraries that are released under any license unless such a combination is not
 # permitted by the license of such a software program or library. You may copy and distribute such a 
 # system following the terms of the GNU Lesser General Public License, version 2.1, including this
 # special exception, for DLT and the licenses of the other code concerned.
 # 
 # Note that people who make modified versions of DLT are not obligated to grant this special exception 
 # for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, 
 # version 2.1, gives permission to release a modified version without this exception; this exception 
 # also makes it possible to release a modified version which carries forward this exception.
 #
 # @licence end@
########

set(dlt_test_multi_process_SRCS dlt-test-multi-process)
add_executable(dlt-test-multi-process ${dlt_test_multi_process_SRCS})
IF(GPROF_DLT_TESTS)
	SET(CMAKE_C_FLAGS "-pg")
ENDIF(GPROF_DLT_TESTS)
target_link_libraries(dlt-test-multi-process dlt)
set_target_properties(dlt-test-multi-process PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_multi_process_client_SRCS dlt-test-multi-process-client)
add_executable(dlt-test-multi-process-client ${dlt_test_multi_process_client_SRCS})
IF(GPROF_DLT_TESTS)
	SET(CMAKE_C_FLAGS "-pg")
ENDIF(GPROF_DLT_TESTS)
target_link_libraries(dlt-test-multi-process-client dlt)
set_target_properties(dlt-test-multi-process-client PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_user_SRCS dlt-test-user)
add_executable(dlt-test-user ${dlt_test_user_SRCS})
IF(GPROF_DLT_TESTS)
	SET(CMAKE_C_FLAGS "-pg")
ENDIF(GPROF_DLT_TESTS)
target_link_libraries(dlt-test-user dlt)
set_target_properties(dlt-test-user PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_client_SRCS dlt-test-client)
add_executable(dlt-test-client ${dlt_test_client_SRCS})
IF(GPROF_DLT_TESTS)
	SET(CMAKE_C_FLAGS "-pg")
ENDIF(GPROF_DLT_TESTS)
target_link_libraries(dlt-test-client dlt)
set_target_properties(dlt-test-client PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_stress_user_SRCS dlt-test-stress-user)
add_executable(dlt-test-stress-user ${dlt_test_stress_user_SRCS})
IF(GPROF_DLT_TESTS)
	SET(CMAKE_C_FLAGS "-pg")
ENDIF(GPROF_DLT_TESTS)
target_link_libraries(dlt-test-stress-user dlt)
set_target_properties(dlt-test-stress-user PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_stress_client_SRCS dlt-test-stress-client)
add_executable(dlt-test-stress-client ${dlt_test_stress_client_SRCS})
IF(GPROF_DLT_TESTS)
	SET(CMAKE_C_FLAGS "-pg")
ENDIF(GPROF_DLT_TESTS)
target_link_libraries(dlt-test-stress-client dlt)
set_target_properties(dlt-test-stress-client PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_stress_SRCS dlt-test-stress)
add_executable(dlt-test-stress ${dlt_test_stress_SRCS})
IF(GPROF_DLT_TESTS)
	SET(CMAKE_C_FLAGS "-pg")
ENDIF(GPROF_DLT_TESTS)
target_link_libraries(dlt-test-stress dlt)
set_target_properties(dlt-test-stress PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_internal_SRCS dlt-test-internal)
add_executable(dlt-test-internal ${dlt_test_internal_SRCS})
IF(GPROF_DLT_TESTS)
	SET(CMAKE_C_FLAGS "-pg")
ENDIF(GPROF_DLT_TESTS)
target_link_libraries(dlt-test-internal dlt)
set_target_properties(dlt-test-internal PROPERTIES LINKER_LANGUAGE C)

set(dlt_test_filetransfer_SRCS dlt-test-filetransfer)
add_executable(dlt-test-filetransfer ${dlt_test_filetransfer_SRCS})
IF(GPROF_DLT_TESTS)
	SET(CMAKE_C_FLAGS "-pg")
ENDIF(GPROF_DLT_TESTS)
target_link_libraries(dlt-test-filetransfer dlt)
set_target_properties(dlt-test-filetransfer PROPERTIES LINKER_LANGUAGE C)

install(TARGETS dlt-test-multi-process dlt-test-multi-process-client dlt-test-user dlt-test-client dlt-test-stress-user dlt-test-stress-client dlt-test-stress dlt-test-internal dlt-test-filetransfer	
	RUNTIME DESTINATION bin
	COMPONENT base)

INSTALL(FILES dlt-test-filetransfer-file dlt-test-filetransfer-image.png
	DESTINATION /usr/share/dlt-filetransfer
)
