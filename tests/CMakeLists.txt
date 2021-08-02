# Setup testing
enable_testing()

SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -isystem ${gtest_SOURCE_DIR}/include")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -isystem ${gtest_SOURCE_DIR}/include -std=gnu++0x")

configure_file(${PROJECT_SOURCE_DIR}/tests/testfile.dlt ${PROJECT_BINARY_DIR}/tests COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/tests/testfilter.txt ${PROJECT_BINARY_DIR}/tests COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/tests/testfile_filetransfer.txt ${PROJECT_BINARY_DIR}/tests COPYONLY)

set(GTEST_LIBS gtest gtest_main)
set(GTEST_LIBS ${GTEST_LIBS} CACHE STRING "Gtest libraries")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(LIBRARIES "")
else()
    set(LIBRARIES socket)
endif()

set(DLT_LIBRARIES dlt ${GTEST_BOTH_LIBRARIES} ${LIBRARIES})
set(DLT_DAEMON_LIBRARIES dlt_daemon ${GTEST_BOTH_LIBRARIES} ${LIBRARIES})
set(DLT_CONTROL_LIBRARIES dlt dlt_control_common_lib ${GTEST_BOTH_LIBRARIES})

#Receiver used for QTs. add_test() is not required
add_executable(dlt_test_receiver dlt_test_receiver.c)
target_link_libraries(dlt_test_receiver ${DLT_LIBRARIES})

####################
# DLT library tests
####################
set(TARGET_LIST gtest_dlt_common
                gtest_dlt_user
                gtest_dlt_daemon_common
                dlt_env_ll_unit_test
                )

foreach(target IN LISTS TARGET_LIST)
    set(target_SRCS ${target})
    if(${target} STREQUAL "gtest_dlt_daemon_common")
        set(target_SRCS ${target_SRCS} ../src/daemon/dlt_daemon_common.c)
    endif()
    add_executable(${target} ${target_SRCS})
    target_link_libraries(${target} ${DLT_LIBRARIES})
    add_test(NAME ${target}
             COMMAND ${target})
endforeach()

###################
# DLT daemon tests
###################
set(TARGET_LIST gtest_dlt_daemon_gateway
                gtest_dlt_daemon_offline_log
                gtest_dlt_daemon_filter
                gtest_dlt_daemon_event_handler
                )
if(WITH_DLT_SHM_ENABLE)
    list(APPEND TARGET_LIST gtest_dlt_shm)
endif()

foreach(target IN LISTS TARGET_LIST)
    set(target_SRCS ${target})
    add_executable(${target} ${target_SRCS} ${systemd_SRCS})
    target_link_libraries(${target} ${DLT_DAEMON_LIBRARIES})
    if(${target} STREQUAL "gtest_dlt_daemon_event_handler" OR
       ${target} STREQUAL "gtest_dlt_shm")
        add_test(NAME ${target}
                 COMMAND ${target})
    else()
        configure_file(${PROJECT_SOURCE_DIR}/tests/${target}.sh ${PROJECT_BINARY_DIR}/tests COPYONLY)
        add_test(NAME ${target}
                 COMMAND /bin/sh "${target}.sh"
                         ${target})
    endif()
endforeach()

#####################
# DLT conotrol tests
#####################
if(WITH_EXTENDED_FILTERING)
    configure_file(${PROJECT_SOURCE_DIR}/tests/testfile_extended.dlt ${PROJECT_BINARY_DIR}/tests COPYONLY)
    configure_file(${PROJECT_SOURCE_DIR}/tests/testfilter.json ${PROJECT_BINARY_DIR}/tests COPYONLY)
    add_executable(gtest_dlt_json_filter gtest_dlt_json_filter.cpp)
    target_link_libraries(gtest_dlt_json_filter ${DLT_CONTROL_LIBRARIES})
    add_test(NAME gtest_dlt_json_filter
        COMMAND gtest_dlt_json_filter)
endif()

