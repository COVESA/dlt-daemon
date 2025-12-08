SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -isystem ${gtest_SOURCE_DIR}/include")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -isystem ${gtest_SOURCE_DIR}/include -std=gnu++0x")

configure_file(${PROJECT_SOURCE_DIR}/tests/testfile.dlt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/tests/testfilter.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/tests/testfile_filetransfer.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/tests/test_dlt.conf ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

if(WITH_DLT_INSTALLED_TESTS)
    set(DLT_TEST_DIR ${CMAKE_INSTALL_BINDIR}/dlt_tests)
    install(FILES testfile.dlt testfilter.txt testfile_filetransfer.txt DESTINATION ${DLT_TEST_DIR})
endif(WITH_DLT_INSTALLED_TESTS)

if("${GTEST_BOTH_LIBRARIES}" STREQUAL "")
    set(GTEST_LIBS gtest gtest_main)
    set(GTEST_LIBS ${GTEST_LIBS} CACHE STRING "Gtest libraries")
else()
    set(GTEST_LIBS ${GTEST_BOTH_LIBRARIES})
endif()

if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux|CYGWIN")
    set(LIBRARIES "")
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "QNX")
    set(LIBRARIES regex)
else()
    set(LIBRARIES socket)
endif()

if (WITH_SYSTEMD
        OR WITH_SYSTEMD_SOCKET_ACTIVATION
        OR WITH_SYSTEMD_WATCHDOG
        OR WITH_SYSTEMD_JOURNAL)
    set(LIBRARIES ${LIBRARIES} systemd)
endif()

set(DLT_LIBRARIES dlt ${GTEST_LIBS} ${LIBRARIES})
set(DLT_DAEMON_LIBRARIES dlt_daemon ${GTEST_LIBS} ${LIBRARIES} ${SYSTEMD_LIBS})
set(DLT_CONTROL_LIBRARIES dlt dlt_control_common_lib ${GTEST_LIBS})

# Receiver used for QTs. add_test() is not required
add_executable(dlt_test_receiver dlt_test_receiver.c)
target_link_libraries(dlt_test_receiver ${DLT_LIBRARIES})

# Prepare string for testing command sequence
set(CMD_SEQ_SLEEP "sleep 0.5")
set(CMD_SEQ_CLEAN "killall -v dlt-daemon || echo 'Could not find any daemon to kill'")
set(CMD_SEQ_EXIT "exit $TESTS_FAILED")
string(CONCAT CMD_SEQ "${CMD_SEQ_SLEEP};"
                      "${CMD_SEQ_CLEAN};"
                      "${CMD_SEQ_EXIT}")

# Prepare string for setting environment variables
set(TEST_ENV_PATH "PATH=$<TARGET_FILE_DIR:dlt-daemon>:$ENV{PATH}")
set(TEST_ENV_DAEMON "DLT_UT_DAEMON_PATH=$<TARGET_FILE:dlt-daemon>")
set(TEST_ENV_DAEMON_CONFIG "DLT_UT_CONFIG_PATH=$<TARGET_FILE_DIR:dlt-daemon>/dlt.conf")
set(TEST_ENV_LO_IPv6 "DLT_IPv6_LO=::1")
string(CONCAT TEST_ENV "${TEST_ENV_PATH};"
                       "${TEST_ENV_DAEMON};"
                       "${TEST_ENV_DAEMON_CONFIG}")
string(CONCAT TEST_ENV_IPv6 "${TEST_ENV};"
                            "${TEST_ENV_LO_IPv6}")

####################
# DLT library tests
####################
set(TARGET_LIST gtest_dlt_common
                gtest_dlt_user
                gtest_dlt_daemon
                gtest_dlt_daemon_common
                dlt_env_ll_unit_test)

foreach(target IN LISTS TARGET_LIST)
    set(target_SRCS)
    if(${target} STREQUAL "dlt_env_ll_unit_test")
        set(target_SRCS ${target}.cpp)
    elseif(${target} STREQUAL "gtest_dlt_daemon_common")
        set(target_SRCS ${target}.cpp ${PROJECT_SOURCE_DIR}/src/daemon/dlt_daemon_common.c)
    elseif(${target} STREQUAL "gtest_dlt_daemon")
        set(target_SRCS ${target}.cpp
            ../src/daemon/dlt-daemon.c
            ../src/daemon/dlt_daemon_client.c
            ../src/daemon/dlt_daemon_common.c
            ../src/daemon/dlt_daemon_connection.c
            ../src/daemon/dlt_daemon_event_handler.c
            ../src/daemon/dlt_daemon_offline_logstorage.c
            ../src/daemon/dlt_daemon_serial.c
            ../src/daemon/dlt_daemon_socket.c
            ../src/daemon/dlt_daemon_unix_socket.c
            ../src/gateway/dlt_gateway.c
            ../src/offlinelogstorage/dlt_offline_logstorage_behavior.c
            ../src/offlinelogstorage/dlt_offline_logstorage.c
            ../src/shared/dlt_config_file_parser.c
            ../src/shared/dlt_offline_trace.c
        )
        add_compile_definitions(${target} DLT_DAEMON_UNIT_TESTS_NO_MAIN)
    else()
        set(target_SRCS ${target}.cpp)
    endif()

    add_executable(${target} ${target_SRCS})
    target_link_libraries(${target} ${DLT_LIBRARIES} ${ZLIB_LIBRARY})
    if(WITH_DLT_INSTALLED_TESTS)
        install(TARGETS ${target} RUNTIME DESTINATION ${DLT_TEST_DIR})
        if(EXISTS ${PROJECT_SOURCE_DIR}/tests/${target}.sh)
            install(PROGRAMS ${target}.sh DESTINATION ${DLT_TEST_DIR})
        endif()
    endif()
    if(EXISTS ${PROJECT_SOURCE_DIR}/tests/${target}.sh)
        configure_file(${PROJECT_SOURCE_DIR}/tests/${target}.sh ${PROJECT_BINARY_DIR}/tests COPYONLY)
        set(CMD_SEQ_SETUP "sh $<TARGET_FILE:${target}>.sh")
        set(CMD_SEQ_BOOL "$<TARGET_FILE:${target}> || TESTS_FAILED=1 && TESTS_FAILED=0")
        add_test(NAME ${target} COMMAND /bin/sh -e -c "${CMD_SEQ_SETUP};${CMD_SEQ_BOOL};${CMD_SEQ}")
        set_tests_properties(${target} PROPERTIES ENVIRONMENT "${TEST_ENV}")
    else()
        add_test(NAME ${target} COMMAND ${target})
    endif()
endforeach()

###################
# DLT daemon tests
###################
set(TARGET_LIST gtest_dlt_daemon_gateway
                gtest_dlt_daemon_offline_log
                gtest_dlt_daemon_event_handler
                gtest_dlt_daemon_multiple_files_logging)

if(WITH_DLT_LOG_STATISTIC)
    list(APPEND TARGET_LIST gtest_dlt_daemon_statistics)
endif()

if(WITH_DLT_SHM_ENABLE)
    list(APPEND TARGET_LIST gtest_dlt_shm)
endif()

foreach(target IN LISTS TARGET_LIST)
    set(target_SRCS ${target}.cpp)
    add_executable(${target} ${target_SRCS} ${systemd_SRCS})
    target_link_libraries(${target} ${DLT_DAEMON_LIBRARIES} ${ZLIB_LIBRARY})
    if(WITH_DLT_INSTALLED_TESTS)
        install(TARGETS ${target} RUNTIME DESTINATION ${DLT_TEST_DIR})
        if(EXISTS ${PROJECT_SOURCE_DIR}/tests/${target}.sh)
            install(PROGRAMS ${target}.sh DESTINATION ${DLT_TEST_DIR})
        endif()
    endif(WITH_DLT_INSTALLED_TESTS)
    if(EXISTS ${PROJECT_SOURCE_DIR}/tests/${target}.sh)
        configure_file(${PROJECT_SOURCE_DIR}/tests/${target}.sh ${PROJECT_BINARY_DIR}/tests COPYONLY)
        set(CMD_SEQ_SETUP "sh $<TARGET_FILE:${target}>.sh")
        set(CMD_SEQ_BOOL "$<TARGET_FILE:${target}> || TESTS_FAILED=1 && TESTS_FAILED=0")
        add_test(NAME ${target} COMMAND /bin/sh -e -c "${CMD_SEQ_SETUP};${CMD_SEQ_BOOL};${CMD_SEQ}")
        if(WITH_DLT_USE_IPv6)
            set_tests_properties(${target} PROPERTIES ENVIRONMENT "${TEST_ENV_IPv6}")
        else()
            set_tests_properties(${target} PROPERTIES ENVIRONMENT "${TEST_ENV}")
        endif()
    else()
        add_test(NAME ${target} COMMAND ${target})
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
    if(WITH_DLT_INSTALLED_TESTS)
        install(FILES testfile_extended.dlt testfilter.json DESTINATION ${DLT_TEST_DIR})
        install(TARGETS gtest_dlt_json_filter RUNTIME DESTINATION ${DLT_TEST_DIR})
    endif(WITH_DLT_INSTALLED_TESTS)
    add_test(NAME gtest_dlt_json_filter COMMAND gtest_dlt_json_filter)
    set_tests_properties(${target} PROPERTIES TIMEOUT "${seconds}")
endif()

add_subdirectory(components)
