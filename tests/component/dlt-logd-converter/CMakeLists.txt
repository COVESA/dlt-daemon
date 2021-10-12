##########################
# DLT logd-converter test
##########################
configure_file(${PROJECT_SOURCE_DIR}/tests/component/dlt-logd-converter/dlt-logd-converter.conf ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/tests/component/dlt-logd-converter/dlt-logdctxt.json ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/tests/component/dlt-logd-converter/abnormal-dlt-logd-converter.conf ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

set(dlt_logd_converter_SRC ${PROJECT_SOURCE_DIR}/src/android/dlt-logd-converter.cpp)
set(gtest_dlt_logd_converter_SRC ${PROJECT_SOURCE_DIR}/tests/component/dlt-logd-converter/gtest_dlt_logd_converter.cpp)

add_executable(gtest_dlt_logd_converter ${dlt_logd_converter_SRC} ${gtest_dlt_logd_converter_SRC})
target_link_libraries(gtest_dlt_logd_converter ${DLT_LIBRARIES})

add_test(NAME gtest_dlt_logd_converter
        COMMAND gtest_dlt_logd_converter)
