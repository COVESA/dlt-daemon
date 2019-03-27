# Setup testing
enable_testing()

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -isystem ${gtest_SOURCE_DIR}/include")

#SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g")
#SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize=address")

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=gnu++0x")

configure_file(${CMAKE_SOURCE_DIR}/tests/testfile.dlt ${PROJECT_BINARY_DIR}/tests COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/tests/testfilter.txt ${PROJECT_BINARY_DIR}/tests COPYONLY)

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
add_executable(gtest_dlt_user gtest_dlt_user.cpp)
add_executable(gtest_dlt_daemon_common gtest_dlt_daemon_common.cpp ../src/daemon/dlt_daemon_common.c)
add_executable(dlt_test_receiver dlt_test_receiver.c)
add_executable(dlt_env_ll_unit_test dlt_env_ll_unit_test.cpp)
add_executable(dlt-test-preregister-context dlt-test-preregister-context.c)
add_executable(gtest_dlt_daemon_gateway gtest_dlt_daemon_gateway.cpp ${systemd_SRCS})
add_executable(gtest_dlt_daemon_event_handler gtest_dlt_daemon_event_handler.cpp ${systemd_SRCS})
add_executable(gtest_dlt_daemon_offline_log gtest_dlt_daemon_offline_log.cpp ${systemd_SRCS})

if(WITH_DLT_SHM_ENABLE)
    add_executable(gtest_dlt_shm gtest_dlt_shm.cpp)
endif(WITH_DLT_SHM_ENABLE)

target_link_libraries(gtest_dlt_common ${DLT_LIBRARIES})
target_link_libraries(gtest_dlt_user ${DLT_LIBRARIES})
target_link_libraries(gtest_dlt_daemon_common ${DLT_LIBRARIES})
target_link_libraries(dlt_test_receiver dlt)
target_link_libraries(dlt_env_ll_unit_test ${DLT_LIBRARIES})
target_link_libraries(dlt-test-preregister-context ${DLT_LIBRARIES})
target_link_libraries(gtest_dlt_daemon_gateway ${DLT_DAEMON_LIBRARIES})
target_link_libraries(gtest_dlt_daemon_event_handler ${DLT_DAEMON_LIBRARIES})
target_link_libraries(gtest_dlt_daemon_offline_log ${DLT_DAEMON_LIBRARIES})

if(WITH_DLT_SHM_ENABLE)
    target_link_libraries(gtest_dlt_shm ${DLT_DAEMON_LIBRARIES})
endif(WITH_DLT_SHM_ENABLE)

if(${WITH_DLT_CXX11_EXT})
  add_executable(dlt-test-cpp-extension dlt-test-cpp-extension.cpp)
  set_target_properties(dlt-test-cpp-extension PROPERTIES COMPILE_FLAGS "-std=gnu++0x")
  target_link_libraries(dlt-test-cpp-extension ${DLT_LIBRARIES})
endif()
