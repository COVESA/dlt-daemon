# Setup testing
enable_testing()

SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -isystem ${gtest_SOURCE_DIR}/include")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -isystem ${gtest_SOURCE_DIR}/include -std=gnu++0x")

configure_file(${PROJECT_SOURCE_DIR}/tests/testfile.dlt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/tests/testfilter.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/tests/testfile_filetransfer.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

if("${GTEST_BOTH_LIBRARIES}" STREQUAL "")
    set(GTEST_LIBS gtest gtest_main)
    set(GTEST_LIBS ${GTEST_LIBS} CACHE STRING "Gtest libraries")
else()
    set(GTEST_LIBS ${GTEST_BOTH_LIBRARIES})
endif()

if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux|CYGWIN")
    set(LIBRARIES "")
else()
    set(LIBRARIES socket)
endif()

if(WITH_SYSTEMD)
  set(SYSTEMD_LIBS systemd)
endif(WITH_SYSTEMD)

set(DLT_LIBRARIES dlt ${GTEST_LIBS} ${LIBRARIES})
set(DLT_DAEMON_LIBRARIES dlt_daemon ${GTEST_LIBS} ${LIBRARIES} ${SYSTEMD_LIBS})
set(DLT_CONTROL_LIBRARIES dlt dlt_control_common_lib ${GTEST_LIBS})

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
        set(target_SRCS ${target_SRCS} ${PROJECT_SOURCE_DIR}/src/daemon/dlt_daemon_common.c)
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
                gtest_dlt_daemon_event_handler
                gtest_dlt_daemon_multiple_files_logging
                )
if(WITH_DLT_SHM_ENABLE)
    list(APPEND TARGET_LIST gtest_dlt_shm)
endif()

foreach(target IN LISTS TARGET_LIST)
    set(target_SRCS ${target})
    add_executable(${target} ${target_SRCS} ${systemd_SRCS})
    target_link_libraries(${target} ${DLT_DAEMON_LIBRARIES} ${ZLIB_LIBRARY})
    if(EXISTS ${PROJECT_SOURCE_DIR}/tests/${target}.sh)
        configure_file(${PROJECT_SOURCE_DIR}/tests/${target}.sh ${PROJECT_BINARY_DIR}/tests COPYONLY)
        add_test(NAME ${target}
                 COMMAND /bin/sh -e -c "\
                         sh $<TARGET_FILE:${target}>.sh ; \
                         $<TARGET_FILE:${target}> || TESTS_FAILED=1 && TESTS_FAILED=0 \
                         sleep 0.5 ; \
                         killall -v dlt-daemon || echo 'Could not find any daemon to kill' ;
                         exit $TESTS_FAILED")
        set_tests_properties(${target} PROPERTIES ENVIRONMENT "PATH=$<TARGET_FILE_DIR:dlt-daemon>:$ENV{PATH}; \
                                                               DLT_UT_DAEMON_PATH=$<TARGET_FILE:dlt-daemon>")
    else()
        add_test(NAME ${target}
                 COMMAND ${target})
    endif()
    set_tests_properties(${target} PROPERTIES TIMEOUT "${seconds}")
endforeach()

#####################
# DLT control tests
#####################
if(WITH_EXTENDED_FILTERING)
    configure_file(${PROJECT_SOURCE_DIR}/tests/testfile_extended.dlt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
    configure_file(${PROJECT_SOURCE_DIR}/tests/testfilter.json ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

    add_executable(gtest_dlt_json_filter gtest_dlt_json_filter.cpp)
    target_link_libraries(gtest_dlt_json_filter ${DLT_CONTROL_LIBRARIES})
    add_test(NAME gtest_dlt_json_filter
             COMMAND gtest_dlt_json_filter)
    set_tests_properties(${target} PROPERTIES TIMEOUT "${seconds}")
endif()
