##########################
# DLT logd-converter test
##########################

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dlt-logd-converter.conf ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dlt-logdctxt.json ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/abnormal-dlt-logd-converter.conf ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

set(dlt_logd_converter_SRC ${CMAKE_SOURCE_DIR}/src/android/dlt-logd-converter.cpp)
set(gtest_dlt_logd_converter_SRC ${CMAKE_CURRENT_SOURCE_DIR}/gtest_dlt_logd_converter.cpp)

add_executable(gtest_dlt_logd_converter ${dlt_logd_converter_SRC} ${gtest_dlt_logd_converter_SRC})
target_link_libraries(gtest_dlt_logd_converter ${DLT_LIBRARIES})

if(WITH_DLT_INSTALLED_TESTS)
    install(TARGETS gtest_dlt_logd_converter RUNTIME DESTINATION ${DLT_TEST_DIR}/components/logd-converter)
    install(FILES dlt-logd-converter.conf dlt-logdctxt.json abnormal-dlt-logd-converter.conf DESTINATION ${DLT_TEST_DIR}/components/logd-converter)
endif(WITH_DLT_INSTALLED_TESTS)

add_test(NAME gtest_dlt_logd_converter
        COMMAND /bin/sh -e -c "\
                $<TARGET_FILE:dlt-daemon> &
                sleep 0.1
                $<TARGET_FILE:gtest_dlt_logd_converter> || TESTS_FAILED=1 && TESTS_FAILED=0 &
                sleep 1
                killall -v dlt-daemon || echo 'Could not find any daemon to kill'
                exit $TESTS_FAILED")

set_tests_properties(${target} PROPERTIES ENVIRONMENT "\
                PATH=$<TARGET_FILE_DIR:dlt-daemon>:$ENV{PATH}; \
                DLT_UT_DAEMON_PATH=$<TARGET_FILE:dlt-daemon>")

set_tests_properties(${target} PROPERTIES TIMEOUT "${seconds}")