set(NAME logstorage_gzip)

add_executable(${NAME} ${NAME}.cpp)
target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(${NAME} PRIVATE dlt)

set(CTXNUM 2)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dlt.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf)

add_test(NAME ${NAME} COMMAND /bin/sh -e -c "\
if [ ${CMAKE_CURRENT_SOURCE_DIR} != ${CMAKE_CURRENT_BINARY_DIR} ];
then cp ${CMAKE_CURRENT_SOURCE_DIR}/dlt_logstorage.conf ${CMAKE_CURRENT_BINARY_DIR}/;
fi
rm -f ${CMAKE_CURRENT_BINARY_DIR}/LGZI*.dlt
$<TARGET_FILE:dlt-daemon> -c ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf &
sleep 0.2
$<TARGET_FILE:${NAME}> -c ${CTXNUM} &
sleep 0.1
$<TARGET_FILE:dlt-logstorage-ctrl> -s -C ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf
sleep 2
killall $<TARGET_FILE_NAME:dlt-daemon>
sleep 0.5
gunzip -d LGZIP*.dlt.gz
$<TARGET_FILE:dlt-convert> -a LGZIP1.dlt
$<TARGET_FILE:dlt-convert> -a LGZIP2.dlt
")

set_tests_properties(${NAME} PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${CTEST_LD_PATHS}")

set_tests_properties(${NAME} PROPERTIES PASS_REGULAR_EXPRESSION "\
ECU1 LGZI CT01 log info V 4 \\[Log message 1 # 0\\].*\
ECU1 LGZI CT01 log info V 4 \\[Log message 1 # 200\\].*\
gzip: LGZIP2.dlt.gz: No such file or directory*\
[13658.259302]~DLT~1405994~WARNING  ~File LGZIP2.dlt cannot be opened!*\
ERROR: Selected first message 0 is out of range!*\
")
set_tests_properties(${NAME} PROPERTIES FAIL_REGULAR_EXPRESSION "\
ECU1 LGZI CT02 log info V 4 \\[Log message 2 # 0\\].*\
ECU1 LGZI CT02 log info V 4 \\[Log message 2 # 200\\].*\
")
