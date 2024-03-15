set(NAME logstorage_filepath)

add_executable(${NAME} ${NAME}.cpp)
target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(${NAME} PRIVATE dlt)

set(CTXNUM 2)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dlt.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf)
if(WITH_DLT_INSTALLED_TESTS)
    install(TARGETS ${NAME} RUNTIME DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_filepath)
    install(FILES dlt.conf.in DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_filepath RENAME dlt.conf)
    install(FILES dlt_logstorage.conf DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_filepath)
endif(WITH_DLT_INSTALLED_TESTS)

add_test(NAME ${NAME} COMMAND /bin/sh -e -c "\
if [ ${CMAKE_CURRENT_SOURCE_DIR} != ${CMAKE_CURRENT_BINARY_DIR} ];
then cp ${CMAKE_CURRENT_SOURCE_DIR}/dlt_logstorage.conf ${CMAKE_CURRENT_BINARY_DIR}/;
fi
rm -rf ${CMAKE_CURRENT_BINARY_DIR}/test
rm -rf ${CMAKE_CURRENT_BINARY_DIR}/aa
mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/test
$<TARGET_FILE:dlt-daemon> -c ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf &
sleep 0.2
$<TARGET_FILE:${NAME}> -c ${CTXNUM} &
sleep 1
killall $<TARGET_FILE_NAME:dlt-daemon>
sleep 0.5
if [ `find ${CMAKE_CURRENT_BINARY_DIR}/aa -name 'FPTH_FAIL.dlt'` ];
then echo 'FPTH_FAIL.dlt exists';
$<TARGET_FILE:dlt-convert> -a ${CMAKE_CURRENT_BINARY_DIR}/aa/FPTH_FAIL.dlt;
fi
if [ `find ${CMAKE_CURRENT_BINARY_DIR}/test -name 'FPTH_OK.dlt'` ];
then echo 'FPTH_OK.dlt exists';
$<TARGET_FILE:dlt-convert> -a ${CMAKE_CURRENT_BINARY_DIR}/test/FPTH_OK.dlt;
fi
")

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
