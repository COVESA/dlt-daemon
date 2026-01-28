set(NAME logstorage_multi_file)

add_executable(${NAME} ${NAME}.cpp)
target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(${NAME} PRIVATE dlt)

set(CTXNUM 12)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dlt.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dlt_logstorage.conf ${CMAKE_CURRENT_BINARY_DIR}/dlt_logstorage.conf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run_test.sh ${CMAKE_CURRENT_BINARY_DIR}/run_test.sh COPYONLY)
if(WITH_DLT_INSTALLED_TESTS)
    install(TARGETS ${NAME} RUNTIME DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_multi_file)
    install(FILES dlt.conf.in DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_multi_file RENAME dlt.conf)
    install(FILES dlt_logstorage.conf DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_multi_file)
endif(WITH_DLT_INSTALLED_TESTS)

add_test(NAME ${NAME} COMMAND /bin/sh -e ${CMAKE_CURRENT_BINARY_DIR}/run_test.sh
    $<TARGET_FILE:dlt-daemon>
    $<TARGET_FILE:${NAME}>
    $<TARGET_FILE:dlt-logstorage-ctrl>
    ${CTXNUM}
    $<TARGET_FILE:dlt-convert>
)

set_tests_properties(${NAME} PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${CTEST_LD_PATHS}")

# List of values which shall appear in log messages of each filters
# The order corresponds to dlt_logstorage.conf filter order
# If the filter shall be disabled, -1 is set
set(PASSVALUES 0 200 0 183 70 70 200 200 0 183 0 200)
set(PASSEXPRESSION "dlt_logstorage_open_log_file: logstorage limit reached, stopping capture for filter: MLTIS")
set(FAILVALUES 200 0 200 0 200 200 0 0 200 0 200 0)
set(FAILEXPRESSION "MLTI.*.dlt is bigger than 1000")
set(CTX 0)

while(CTX LESS CTXNUM)
    list(GET PASSVALUES ${CTX} PVAL)
    list(GET FAILVALUES ${CTX} FVAL)
    MATH(EXPR CTX "${CTX} + 1")
    if(NOT ${VAL} EQUAL -1)
        STRING(LENGTH "${CTX}" CTXLEN)
        if(CTXLEN LESS 2)
            SET(CTXID "0${CTX}")
        else()
            SET(CTXID "${CTX}")
        endif()
        string(APPEND PASSEXPRESSION ".*ECU1 MLTI CT${CTXID} log info V 4 \\[Log message ${CTX} # ${PVAL}\\]")
        string(APPEND FAILEXPRESSION ".*ECU1 MLTI CT${CTXID} log info V 4 \\[Log message ${CTX} # ${FVAL}\\]")
    endif()
endwhile()
set_tests_properties(${NAME} PROPERTIES PASS_REGULAR_EXPRESSION ${PASSEXPRESSION})
set_tests_properties(${NAME} PROPERTIES FAIL_REGULAR_EXPRESSION ${FAILEXPRESSION})
