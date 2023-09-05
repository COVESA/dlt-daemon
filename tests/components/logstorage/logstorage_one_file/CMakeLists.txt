set(NAME logstorage_one_file)

add_executable(${NAME} ${NAME}.cpp)
target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_link_libraries(${NAME} PRIVATE dlt)

set(CTXNUM 12)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dlt.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf)

add_test(NAME ${NAME} COMMAND /bin/sh -e -c "\
if [ ${CMAKE_CURRENT_SOURCE_DIR} != ${CMAKE_CURRENT_BINARY_DIR} ];
then cp ${CMAKE_CURRENT_SOURCE_DIR}/dlt_logstorage.conf ${CMAKE_CURRENT_BINARY_DIR}/;
fi
rm -f ${CMAKE_CURRENT_BINARY_DIR}/LONE*.dlt
$<TARGET_FILE:dlt-daemon> -c ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf &
sleep 0.2
$<TARGET_FILE:${NAME}> -c ${CTXNUM} &
sleep 0.1
$<TARGET_FILE:dlt-logstorage-ctrl> -s -C ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf
sleep 1
killall $<TARGET_FILE_NAME:dlt-daemon>
sleep 0.5
if [ `stat -c%s LONESTOP.dlt` -gt 2000 ]; then echo 'LONESTOP.dlt is bigger than 2000'; fi
if [ `stat -c%s LONE.dlt` -gt 2000 ]; then echo 'LONE.dlt is bigger than 2000'; fi
if [ `stat -c%s LONEFILESTOP.dlt` -gt 2000 ]; then echo 'LONEFILESTOP.dlt is bigger than 2000'; fi
if [ `stat -c%s LONEFILE.dlt` -gt 2000 ]; then echo 'LONEFILE.dlt is bigger than 2000'; fi
if [ `stat -c%s LONEDEMANDSTOP.dlt` -gt 2000 ]; then echo 'LONEDEMANDSTOP.dlt is bigger than 2000'; fi
if [ `stat -c%s LONEDEMAND.dlt` -gt 2000 ]; then echo 'LONEDEMAND.dlt is bigger than 2000'; fi
if [ `stat -c%s LONEEXITSTOP.dlt` -gt 2000 ]; then echo 'LONEEXITSTOP.dlt is bigger than 2000'; fi
if [ `stat -c%s LONEEXIT.dlt` -gt 2000 ]; then echo 'LONEEXIT.dlt is bigger than 2000'; fi
if [ `stat -c%s LONESPSTOP.dlt` -gt 2000 ]; then echo 'LONESPSTOP.dlt is bigger than 2000'; fi
if [ `stat -c%s LONESP.dlt` -gt 2000 ]; then echo 'LONESP.dlt is bigger than 2000'; fi
if [ `stat -c%s LONECOMBSTOP.dlt` -gt 2000 ]; then echo 'LONECOMBSTOP.dlt is bigger than 2000'; fi
if [ `stat -c%s LONECOMB.dlt` -gt 2000 ]; then echo 'LONECOMB.dlt is bigger than 2000'; fi
$<TARGET_FILE:dlt-convert> -a LONESTOP.dlt
$<TARGET_FILE:dlt-convert> -a LONE.dlt
$<TARGET_FILE:dlt-convert> -a LONEFILESTOP.dlt
$<TARGET_FILE:dlt-convert> -a LONEFILE.dlt
$<TARGET_FILE:dlt-convert> -a LONEDEMANDSTOP.dlt
$<TARGET_FILE:dlt-convert> -a LONEDEMAND.dlt
$<TARGET_FILE:dlt-convert> -a LONEEXITSTOP.dlt
$<TARGET_FILE:dlt-convert> -a LONEEXIT.dlt
$<TARGET_FILE:dlt-convert> -a LONESPSTOP.dlt
$<TARGET_FILE:dlt-convert> -a LONESP.dlt
$<TARGET_FILE:dlt-convert> -a LONECOMBSTOP.dlt
$<TARGET_FILE:dlt-convert> -a LONECOMB.dlt
")

set_tests_properties(${NAME} PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${CTEST_LD_PATHS}")

# List of values which shall appear in log messages of each filters
# The order corresponds to dlt_logstorage.conf filter order
# If the filter shall be disabled, -1 is set
set(PASSVALUES 0 200 0 183 60 60 200 200 0 199 0 200)
set(PASSEXPRESSION "dlt_logstorage_open_log_file: logstorage limit reached, stopping capture for filter: LONE")
set(FAILVALUES 200 0 200 0 200 200 0 0 200 0 200 0)
set(FAILEXPRESSION "LONE.*.dlt is bigger than 2000")
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
        string(APPEND PASSEXPRESSION ".*ECU1 LONE CT${CTXID} log info V 4 \\[Log message ${CTX} # ${PVAL}\\]")
        string(APPEND FAILEXPRESSION ".*ECU1 LONE CT${CTXID} log info V 4 \\[Log message ${CTX} # ${FVAL}\\]")
    endif()
endwhile()

set_tests_properties(${NAME} PROPERTIES PASS_REGULAR_EXPRESSION ${PASSEXPRESSION})
set_tests_properties(${NAME} PROPERTIES FAIL_REGULAR_EXPRESSION ${FAILEXPRESSION})
