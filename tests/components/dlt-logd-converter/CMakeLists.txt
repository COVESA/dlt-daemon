##########################
# DLT logd-converter test
##########################
configure_file(${PROJECT_SOURCE_DIR}/tests/components/dlt-logd-converter/dlt.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/tests/components/dlt-logd-converter/dlt-logd-converter.conf ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/tests/components/dlt-logd-converter/dlt-logdctxt.json ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/tests/components/dlt-logd-converter/abnormal-dlt-logd-converter.conf ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
set(dlt_logd_converter_SRC ${PROJECT_SOURCE_DIR}/src/android/dlt-logd-converter.cpp)
set(gtest_dlt_logd_converter_SRC ${PROJECT_SOURCE_DIR}/tests/components/dlt-logd-converter/gtest_dlt_logd_converter.cpp)
add_executable(gtest_dlt_logd_converter ${dlt_logd_converter_SRC} ${gtest_dlt_logd_converter_SRC})
target_link_libraries(gtest_dlt_logd_converter ${DLT_LIBRARIES})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run_test.sh ${CMAKE_CURRENT_BINARY_DIR}/run_test.sh COPYONLY)
add_test(NAME gtest_dlt_logd_converter
        COMMAND /bin/sh -e ${CMAKE_CURRENT_BINARY_DIR}/run_test.sh
            $<TARGET_FILE:dlt-daemon>
            $<TARGET_FILE:gtest_dlt_logd_converter>)
set_tests_properties(gtest_dlt_logd_converter PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${CTEST_LD_PATHS}")