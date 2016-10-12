# Setup testing
enable_testing()

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -isystem ${gtest_SOURCE_DIR}/include")

#SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g")
#SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize=address")

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=gnu++0x")

configure_file(${CMAKE_SOURCE_DIR}/tests/testfile.dlt ${PROJECT_BINARY_DIR}/tests COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/tests/testfilter.txt ${PROJECT_BINARY_DIR}/tests COPYONLY)

add_executable(gtest_dlt_common gtest_dlt_common.cpp)
add_executable(gtest_dlt_user gtest_dlt_user.cpp)
add_executable(gtest_dlt_daemon_common gtest_dlt_daemon_common.cpp ../src/daemon/dlt_daemon_common.c)
add_executable(dlt_test_receiver dlt_test_receiver.c)
add_executable(dlt_env_ll_unit_test dlt_env_ll_unit_test.cpp)
add_executable(dlt-test-preregister-context dlt-test-preregister-context.c)
add_executable(gtest_dlt_daemon_gateway gtest_dlt_daemon_gateway.cpp)
target_link_libraries(gtest_dlt_common gtest gtest_main dlt)
target_link_libraries(gtest_dlt_user gtest gtest_main dlt)
target_link_libraries(gtest_dlt_daemon_common gtest gtest_main dlt)
target_link_libraries(dlt_test_receiver dlt)
target_link_libraries(dlt_env_ll_unit_test gtest gtest_main dlt)
target_link_libraries(dlt-test-preregister-context gtest gtest_main dlt)
target_link_libraries(gtest_dlt_daemon_gateway gtest gtest_main dlt_daemon)

if(${WITH_DLT_CXX11_EXT})
  add_executable(dlt-test-cpp-extension dlt-test-cpp-extension.cpp)
  set_target_properties(dlt-test-cpp-extension PROPERTIES COMPILE_FLAGS "-std=gnu++0x")
  target_link_libraries(dlt-test-cpp-extension gtest gtest_main dlt)
endif()
