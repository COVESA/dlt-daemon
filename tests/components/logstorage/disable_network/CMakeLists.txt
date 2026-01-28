set(NAME disable_network)

add_executable(${NAME} ${NAME}.cpp)
target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(${NAME} PRIVATE dlt)

set(APPID "DSNW")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dlt.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dlt_logstorage.conf ${CMAKE_CURRENT_BINARY_DIR}/dlt_logstorage.conf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run_test.sh ${CMAKE_CURRENT_BINARY_DIR}/run_test.sh COPYONLY)
if(WITH_DLT_INSTALLED_TESTS)
    install(TARGETS ${NAME} RUNTIME DESTINATION ${DLT_TEST_DIR}/components/logstorage/disable_network)
    install(FILES dlt.conf.in DESTINATION ${DLT_TEST_DIR}/components/logstorage/disable_network RENAME dlt.conf)
    install(FILES dlt_logstorage.conf DESTINATION ${DLT_TEST_DIR}/components/logstorage/disable_network)
endif(WITH_DLT_INSTALLED_TESTS)

add_test(NAME ${NAME} COMMAND /bin/sh -e ${CMAKE_CURRENT_BINARY_DIR}/run_test.sh
    $<TARGET_FILE:dlt-daemon>
    $<TARGET_FILE:dlt-receive>
    $<TARGET_FILE:${NAME}>
    2
    $<TARGET_FILE:dlt-convert>
)

set_tests_properties(${NAME} PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${CTEST_LD_PATHS}")

set_tests_properties(${NAME} PROPERTIES PASS_REGULAR_EXPRESSION "\
(Result from network.*\
ECU1 DSNW CT02 log info V 4 \\\[Log message 2 # [0-9]+\\\].*\
ECU1 DSNW CT02 log info V 4 \\\[Log message 2 # [0-9]+\\\].*\
Result from file.*\
ECU1 DSNW CT01 log info V 4 \\\[Log message 1 # [0-9]+\\\].*\
ECU1 DSNW CT01 log info V 4 \\\[Log message 1 # [0-9]+\\\].*\
)|(Result from file.*\
ECU1 DSNW CT01 log info V 4 \\\[Log message 1 # [0-9]+\\\].*\
ECU1 DSNW CT01 log info V 4 \\\[Log message 1 # [0-9]+\\\].*\
Result from network.*\
ECU1 DSNW CT02 log info V 4 \\\[Log message 2 # [0-9]+\\\].*\
ECU1 DSNW CT02 log info V 4 \\\[Log message 2 # [0-9]+\\\].*\
)")
set_tests_properties(${NAME} PROPERTIES FAIL_REGULAR_EXPRESSION "\
Result from network.*\
ECU1 DSNW CT01 log info V 4 \\\[Log message 1 #.*\
Result from file.*\
ECU1 DSNW CT02 log info V 4 \\\[Log message 2 #.*\
")