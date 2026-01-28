set(NAME logstorage_max_cache_size)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dlt.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dlt_logstorage.conf ${CMAKE_CURRENT_BINARY_DIR}/dlt_logstorage.conf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run_test.sh ${CMAKE_CURRENT_BINARY_DIR}/run_test.sh COPYONLY)

add_executable(${NAME} ${NAME}.cpp)
target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(${NAME} PRIVATE dlt)
if(WITH_DLT_INSTALLED_TESTS)
    install(TARGETS ${NAME} RUNTIME DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_max_cache_size)
    install(FILES dlt.conf.in DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_max_cache_size RENAME dlt.conf)
    install(FILES dlt_logstorage.conf DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_max_cache_size)
endif(WITH_DLT_INSTALLED_TESTS)

add_test(NAME ${NAME} COMMAND /bin/sh -e
    ${CMAKE_CURRENT_BINARY_DIR}/run_test.sh
    $<TARGET_FILE:dlt-daemon>
    $<TARGET_FILE:${NAME}>
    $<TARGET_FILE:dlt-convert>
)

set(PASSEXPRESSION "Max size of Logstorage Cache already used")

set_tests_properties(${NAME} PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${CTEST_LD_PATHS}")
set_tests_properties(${NAME} PROPERTIES PASS_REGULAR_EXPRESSION ${PASSEXPRESSION})
