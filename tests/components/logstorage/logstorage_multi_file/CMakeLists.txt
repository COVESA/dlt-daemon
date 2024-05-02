set(NAME logstorage_multi_file)

add_executable(${NAME} ${NAME}.cpp)
target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(${NAME} PRIVATE dlt)

set(CTXNUM 12)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dlt.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf)
if(WITH_DLT_INSTALLED_TESTS)
    install(TARGETS ${NAME} RUNTIME DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_multi_file)
    install(FILES dlt.conf.in DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_multi_file RENAME dlt.conf)
    install(FILES dlt_logstorage.conf DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_multi_file)
endif(WITH_DLT_INSTALLED_TESTS)

add_test(NAME ${NAME} COMMAND /bin/sh -e -c "\
if [ ${CMAKE_CURRENT_SOURCE_DIR} != ${CMAKE_CURRENT_BINARY_DIR} ];
then cp ${CMAKE_CURRENT_SOURCE_DIR}/dlt_logstorage.conf ${CMAKE_CURRENT_BINARY_DIR}/;
fi
rm -f ${CMAKE_CURRENT_BINARY_DIR}/MLTI*.dlt
$<TARGET_FILE:dlt-daemon> -c ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf &
sleep 0.2
$<TARGET_FILE:${NAME}> -c ${CTXNUM} &
sleep 0.1
$<TARGET_FILE:dlt-logstorage-ctrl> -s -C ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf
sleep 2
killall $<TARGET_FILE_NAME:dlt-daemon>
sleep 0.5
if [ `stat -c%s MLTISTOP_1.dlt` -gt 1000 ]; then echo 'MLTISTOP_1.dlt is bigger than 1000'; fi
if [ `stat -c%s MLTISTOP_2.dlt` -gt 1000 ]; then echo 'MLTISTOP_2.dlt is bigger than 1000'; fi
if [ `stat -c%s MLTI_18.dlt` -gt 1000 ]; then echo 'MLTI_18.dlt is bigger than 1000'; fi
if [ `stat -c%s MLTI_19.dlt` -gt 1000 ]; then echo 'MLTI_19.dlt is bigger than 1000'; fi
if [ `stat -c%s MLTIFILESTOP_1.dlt` -gt 1000 ]; then echo 'MLTIFILESTOP_1.dlt is bigger than 1000'; fi
if [ `stat -c%s MLTIFILESTOP_2.dlt` -gt 1000 ]; then echo 'MLTIFILESTOP_2.dlt is bigger than 1000'; fi
if [ `stat -c%s MLTIFILE_17.dlt` -gt 1000 ]; then echo 'MLTIFILE_17.dlt is bigger than 1000'; fi
if [ `stat -c%s MLTIFILE_18.dlt` -gt 1000 ]; then echo 'MLTIFILE_18.dlt is bigger than 1000'; fi
if [ `stat -c%s MLTIDEMANDSTOP_1.dlt` -gt 1000 ]; then echo 'MLTIDEMANDSTOP_1.dlt is bigger than 1000'; fi
if [ `stat -c%s MLTIDEMAND_1.dlt` -gt 1000 ]; then echo 'MLTIDEMAND_1.dlt is bigger than 1000'; fi
if [ `stat -c%s MLTIEXITSTOP_1.dlt` -gt 1000 ]; then echo 'MLTIEXITSTOP_1.dlt is bigger than 1000'; fi
if [ `stat -c%s MLTIEXIT_1.dlt` -gt 1000 ]; then echo 'MLTIEXIT_1.dlt is bigger than 1000'; fi
if [ `stat -c%s MLTISPSTOP_1.dlt` -gt 1000 ]; then echo 'MLTISPSTOP_1.dlt is bigger than 1000'; fi
if [ `stat -c%s MLTISPSTOP_2.dlt` -gt 1000 ]; then echo 'MLTISPSTOP_2.dlt is bigger than 1000'; fi
if [ `stat -c%s MLTISP_17.dlt` -gt 1000 ]; then echo 'MLTISP_17.dlt is bigger than 1000'; fi
if [ `stat -c%s MLTISP_18.dlt` -gt 1000 ]; then echo 'MLTISP_18.dlt is bigger than 1000'; fi
if [ `stat -c%s MLTICOMBSTOP_1.dlt` -gt 1000 ]; then echo 'MLTIEXITSTOP_1.dlt is bigger than 1000'; fi
if [ `stat -c%s MLTICOMBSTOP_2.dlt` -gt 1000 ]; then echo 'MLTICOMBSTOP_2.dlt is bigger than 1000'; fi
if [ `stat -c%s MLTICOMB_18.dlt` -gt 1000 ]; then echo 'MLTIEXIT_18.dlt is bigger than 1000'; fi
if [ `stat -c%s MLTICOMB_19.dlt` -gt 1000 ]; then echo 'MLTIEXIT_19.dlt is bigger than 1000'; fi
$<TARGET_FILE:dlt-convert> -a MLTISTOP_1.dlt
$<TARGET_FILE:dlt-convert> -a MLTISTOP_2.dlt
$<TARGET_FILE:dlt-convert> -a MLTI_18.dlt
$<TARGET_FILE:dlt-convert> -a MLTI_19.dlt
$<TARGET_FILE:dlt-convert> -a MLTIFILESTOP_1.dlt
$<TARGET_FILE:dlt-convert> -a MLTIFILESTOP_2.dlt
$<TARGET_FILE:dlt-convert> -a MLTIFILE_17.dlt
$<TARGET_FILE:dlt-convert> -a MLTIFILE_18.dlt
$<TARGET_FILE:dlt-convert> -a MLTIDEMANDSTOP_1.dlt
$<TARGET_FILE:dlt-convert> -a MLTIDEMAND_1.dlt
$<TARGET_FILE:dlt-convert> -a MLTIEXITSTOP_1.dlt
$<TARGET_FILE:dlt-convert> -a MLTIEXIT_1.dlt
$<TARGET_FILE:dlt-convert> -a MLTISPSTOP_1.dlt
$<TARGET_FILE:dlt-convert> -a MLTISPSTOP_2.dlt
$<TARGET_FILE:dlt-convert> -a MLTISP_17.dlt
$<TARGET_FILE:dlt-convert> -a MLTISP_18.dlt
$<TARGET_FILE:dlt-convert> -a MLTICOMBSTOP_1.dlt
$<TARGET_FILE:dlt-convert> -a MLTICOMBSTOP_2.dlt
$<TARGET_FILE:dlt-convert> -a MLTICOMB_18.dlt
$<TARGET_FILE:dlt-convert> -a MLTICOMB_19.dlt
")

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
