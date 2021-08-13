set(NAME logstorage_multi_file)

add_executable(${NAME} ${NAME}.cpp)

target_link_libraries(${NAME} PRIVATE dlt)

add_test(NAME ${NAME} COMMAND /bin/sh -e -c "\
cp ${CMAKE_CURRENT_SOURCE_DIR}/dlt_logstorage.conf ${CMAKE_CURRENT_BINARY_DIR}/
rm -f ${CMAKE_CURRENT_BINARY_DIR}/MULTI*.dlt
$<TARGET_FILE:dlt-daemon> -c ${CMAKE_CURRENT_SOURCE_DIR}/dlt.conf &
sleep 0.2
$<TARGET_FILE:${NAME}> -c 4
sleep 0.5
killall $<TARGET_FILE_NAME:dlt-daemon>
if [ `stat -c%s MULTISTOP_1.dlt` -gt 1000 ]; then echo 'MULTISTOP_1.dlt is bigger than 1000'; fi
if [ `stat -c%s MULTISTOP_2.dlt` -gt 1000 ]; then echo 'MULTISTOP_2.dlt is bigger than 1000'; fi
if [ `stat -c%s MULTI_18.dlt` -gt 1000 ]; then echo 'MULTI_18.dlt is bigger than 1000'; fi
if [ `stat -c%s MULTI_19.dlt` -gt 1000 ]; then echo 'MULTI_19.dlt is bigger than 1000'; fi
if [ `stat -c%s MULTISYNCSTOP_1.dlt` -gt 1000 ]; then echo 'MULTISYNCSTOP_1.dlt is bigger than 1000'; fi
if [ `stat -c%s MULTISYNCSTOP_2.dlt` -gt 1000 ]; then echo 'MULTISYNCSTOP_2.dlt is bigger than 1000'; fi
if [ `stat -c%s MULTISYNC_17.dlt` -gt 1000 ]; then echo 'MULTISYNC_17.dlt is bigger than 1000'; fi
if [ `stat -c%s MULTISYNC_18.dlt` -gt 1000 ]; then echo 'MULTISYNC_18.dlt is bigger than 1000'; fi
$<TARGET_FILE:dlt-convert> -a MULTISTOP_1.dlt
$<TARGET_FILE:dlt-convert> -a MULTISTOP_2.dlt
$<TARGET_FILE:dlt-convert> -a MULTI_18.dlt
$<TARGET_FILE:dlt-convert> -a MULTI_19.dlt
$<TARGET_FILE:dlt-convert> -a MULTISYNCSTOP_1.dlt
$<TARGET_FILE:dlt-convert> -a MULTISYNCSTOP_2.dlt
$<TARGET_FILE:dlt-convert> -a MULTISYNC_17.dlt
$<TARGET_FILE:dlt-convert> -a MULTISYNC_18.dlt
")

set_tests_properties(${NAME} PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${CTEST_LD_PATHS}")

set_tests_properties(${NAME} PROPERTIES PASS_REGULAR_EXPRESSION
    "dlt_logstorage_open_log_file: logstorage limit reached, stopping capture for filter: MULTIS.*ECU1 MLTI CT01 log info V 4 \\[Log message 1 # 0\\].*ECU1 MLTI CT02 log info V 4 \\[Log message 2 # 200\\].*ECU1 MLTI CT03 log info V 4 \\[Log message 3 # 0\\].*ECU1 MLTI CT04 log info V 4 \\[Log message 4 # 183\\]")
set_tests_properties(${NAME} PROPERTIES FAIL_REGULAR_EXPRESSION
    "MULTI.*.dlt is bigger than 2000.*ECU1 MLTI CT01 log info V 4 \\[Log message 1 # 200\\].*ECU1 MLTI CT02 log info V 4 \\[Log message 2 # 0\\].*ECU1 MLTI CT03 log info V 4 \\[Log message 3 # 200\\].*ECU1 MLTI CT04 log info V 4 \\[Log message 4 # 200\\]")
