set(NAME logstorage_one_file)

add_executable(${NAME} ${NAME}.cpp)

target_link_libraries(${NAME} PRIVATE dlt)

add_test(NAME ${NAME} COMMAND /bin/sh -e -c "\
cp ${CMAKE_CURRENT_SOURCE_DIR}/dlt_logstorage.conf ${CMAKE_CURRENT_BINARY_DIR}/
rm -f ${CMAKE_CURRENT_BINARY_DIR}/LONE*.dlt
$<TARGET_FILE:dlt-daemon> -c ${CMAKE_CURRENT_SOURCE_DIR}/dlt.conf &
sleep 0.2
$<TARGET_FILE:${NAME}> -c 4
sleep 0.5
killall $<TARGET_FILE_NAME:dlt-daemon>
if [ `stat -c%s LONESTOP.dlt` -gt 2000 ]; then echo 'LONESTOP.dlt is bigger than 2000'; fi
if [ `stat -c%s LONE.dlt` -gt 2000 ]; then echo 'LONE.dlt is bigger than 2000'; fi
if [ `stat -c%s LONESYNCSTOP.dlt` -gt 2000 ]; then echo 'LONESYNCSTOP.dlt is bigger than 2000'; fi
if [ `stat -c%s LONESYNC.dlt` -gt 2000 ]; then echo 'LONESYNC.dlt is bigger than 2000'; fi
$<TARGET_FILE:dlt-convert> -a LONESTOP.dlt
$<TARGET_FILE:dlt-convert> -a LONE.dlt
$<TARGET_FILE:dlt-convert> -a LONESYNCSTOP.dlt
$<TARGET_FILE:dlt-convert> -a LONESYNC.dlt
")

set_tests_properties(${NAME} PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${CTEST_LD_PATHS}")

set_tests_properties(${NAME} PROPERTIES PASS_REGULAR_EXPRESSION
    "dlt_logstorage_open_log_file: logstorage limit reached, stopping capture for filter: LONES.*ECU1 LONE CT01 log info V 4 \\[Log message 1 # 0\\].*ECU1 LONE CT02 log info V 4 \\[Log message 2 # 200\\].*ECU1 LONE CT03 log info V 4 \\[Log message 3 # 0\\].*ECU1 LONE CT04 log info V 4 \\[Log message 4 # 183\\]")
set_tests_properties(${NAME} PROPERTIES FAIL_REGULAR_EXPRESSION
    "LONE.*.dlt is bigger than 2000.*ECU1 LONE CT01 log info V 4 \\[Log message 1 # 200\\].*ECU1 LONE CT02 log info V 4 \\[Log message 2 # 0\\].*ECU1 LONE CT03 log info V 4 \\[Log message 3 # 200\\].*ECU1 LONE CT04 log info V 4 \\[Log message 4 # 0\\]")
