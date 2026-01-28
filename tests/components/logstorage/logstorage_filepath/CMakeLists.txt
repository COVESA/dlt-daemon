set(NAME logstorage_filepath)

add_executable(${NAME} ${NAME}.cpp)
target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(${NAME} PRIVATE dlt)

set(CTXNUM 2)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dlt.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dlt_logstorage.conf ${CMAKE_CURRENT_BINARY_DIR}/dlt_logstorage.conf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run_test.sh ${CMAKE_CURRENT_BINARY_DIR}/run_test.sh COPYONLY)
if(WITH_DLT_INSTALLED_TESTS)
    install(TARGETS ${NAME} RUNTIME DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_filepath)
    install(FILES dlt.conf.in DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_filepath RENAME dlt.conf)
    install(FILES dlt_logstorage.conf DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_filepath)
endif(WITH_DLT_INSTALLED_TESTS)

add_test(NAME ${NAME} COMMAND /bin/sh -e
    ${CMAKE_CURRENT_BINARY_DIR}/run_test.sh
    $<TARGET_FILE:dlt-daemon>
    $<TARGET_FILE:${NAME}>
    ${CTXNUM}
    $<TARGET_FILE:dlt-convert>
)

set_tests_properties(${NAME} PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${CTEST_LD_PATHS}")

set_tests_properties(${NAME} PROPERTIES PASS_REGULAR_EXPRESSION
"Skip filename \\[aa/FPTH_FAIL\\] because maxmimum trial has been reached..*\
FPTH_OK.dlt exists.*\
ECU1 FPTH CT02 log info V 4 \\[Log message 2 # 200\\]\
")

set_tests_properties(${NAME} PROPERTIES FAIL_REGULAR_EXPRESSION
"FPTH_FAIL.dlt exists\
ECU1 FPTH CT01 log info V 4 \\[Log message 1\
")

set_tests_properties(${NAME} PROPERTIES PASS_REGULAR_EXPRESSION
"Skip filename \\\[aa/FPTH_FAIL\\\] because maxmimum trial has been reached..*\
FPTH_OK.dlt exists.*\
ECU1 FPTH CT02 log info V 4 \\\[Log message 2 # [0-9]+\\\]\
")
