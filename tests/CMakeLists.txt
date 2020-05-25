#add_compile_options(-g -fsanitize=address)
add_compile_options(-isystem ${gtest_SOURCE_DIR}/include)

configure_file(${PROJECT_SOURCE_DIR}/tests/testfile.dlt ${PROJECT_BINARY_DIR}/tests COPYONLY)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(LIBRARIES gtest gtest_main)
else()
    set(LIBRARIES gtest socket)
endif()

set(DLT_LIBRARIES ${LIBRARIES} dlt)
set(DLT_DAEMON_LIBRARIES ${LIBRARIES} dlt_daemon)

if(WITH_SYSTEMD OR WITH_SYSTEMD_WATCHDOG OR WITH_SYSTEMD_JOURNAL)
  add_definitions( -DSD_EXPORT_SYMBOLS )
endif()

add_executable(gtest_dlt_common gtest_dlt_common.cpp)
target_link_libraries(gtest_dlt_common ${DLT_LIBRARIES})
add_test(NAME gtest_dlt_common
    COMMAND gtest_dlt_common
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(gtest_dlt_user gtest_dlt_user.cpp)
target_link_libraries(gtest_dlt_user ${DLT_LIBRARIES})
add_test(NAME gtest_dlt_user
    COMMAND gtest_dlt_user
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(gtest_dlt_daemon_common gtest_dlt_daemon_common.cpp ../src/daemon/dlt_daemon_common.c)
target_link_libraries(gtest_dlt_daemon_common ${DLT_LIBRARIES})
add_test(NAME gtest_dlt_daemon_common
    COMMAND gtest_dlt_daemon_common
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(dlt_test_receiver dlt_test_receiver.c)
target_link_libraries(dlt_test_receiver dlt)

add_executable(dlt_env_ll_unit_test dlt_env_ll_unit_test.cpp)
target_link_libraries(dlt_env_ll_unit_test ${DLT_LIBRARIES})
add_test(NAME dlt_env_ll_unit_test
    COMMAND gtest_dlt_common
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(dlt-test-preregister-context dlt-test-preregister-context.c)
target_link_libraries(dlt-test-preregister-context ${DLT_LIBRARIES})

add_executable(gtest_dlt_daemon_gateway gtest_dlt_daemon_gateway.cpp ${systemd_SRCS})
target_link_libraries(gtest_dlt_daemon_gateway ${DLT_DAEMON_LIBRARIES})

add_executable(gtest_dlt_daemon_event_handler gtest_dlt_daemon_event_handler.cpp ${systemd_SRCS})
target_link_libraries(gtest_dlt_daemon_event_handler ${DLT_DAEMON_LIBRARIES})
add_test(NAME gtest_dlt_daemon_event_handler
    COMMAND gtest_dlt_daemon_event_handler
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(gtest_dlt_daemon_offline_log gtest_dlt_daemon_offline_log.cpp ${systemd_SRCS})
target_link_libraries(gtest_dlt_daemon_offline_log ${DLT_DAEMON_LIBRARIES})

if(WITH_DLT_SHM_ENABLE)
    add_executable(gtest_dlt_shm gtest_dlt_shm.cpp)
endif(WITH_DLT_SHM_ENABLE)

if(WITH_DLT_SHM_ENABLE)
    target_link_libraries(gtest_dlt_shm ${DLT_DAEMON_LIBRARIES})
endif(WITH_DLT_SHM_ENABLE)

if(WITH_DLT_CXX11_EXT)
  add_executable(dlt-test-cpp-extension dlt-test-cpp-extension.cpp)
  target_link_libraries(dlt-test-cpp-extension ${DLT_LIBRARIES})
endif()
