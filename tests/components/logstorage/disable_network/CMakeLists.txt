set(NAME disable_network)

add_executable(${NAME} ${NAME}.cpp)
target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(${NAME} PRIVATE dlt)

set(APPID "DSNW")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dlt.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf)

add_test(NAME ${NAME} COMMAND /bin/sh -e -c "\
if [ ${CMAKE_CURRENT_SOURCE_DIR} != ${CMAKE_CURRENT_BINARY_DIR} ];
then cp ${CMAKE_CURRENT_SOURCE_DIR}/dlt_logstorage.conf ${CMAKE_CURRENT_BINARY_DIR}/;
fi
rm -rf ${CMAKE_CURRENT_BINARY_DIR}/*.dlt
$<TARGET_FILE:dlt-daemon> -c ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf &
sleep 0.1
$<TARGET_FILE:dlt-receive> -o ${CMAKE_CURRENT_BINARY_DIR}/dlt-receive.dlt localhost &
sleep 0.1
$<TARGET_FILE:${NAME}> -c 2 &
sleep 1
killall $<TARGET_FILE:dlt-receive>
killall $<TARGET_FILE:dlt-daemon>
sleep 0.5
echo 'Result from network'
$<TARGET_FILE:dlt-convert> -a ${CMAKE_CURRENT_BINARY_DIR}/dlt-receive.dlt
echo 'Result from file'
$<TARGET_FILE:dlt-convert> -a ${CMAKE_CURRENT_BINARY_DIR}/${APPID}.dlt
")

set_tests_properties(${NAME} PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${CTEST_LD_PATHS}")

set_tests_properties(${NAME} PROPERTIES PASS_REGULAR_EXPRESSION "\
Result from network.*\
ECU1 DSNW CT02 log info V 4 \\[Log message 2 # 0\\].*\
ECU1 DSNW CT02 log info V 4 \\[Log message 2 # 200\\].*\
Result from file.*\
ECU1 DSNW CT01 log info V 4 \\[Log message 1 # 0\\].*\
ECU1 DSNW CT01 log info V 4 \\[Log message 1 # 22\\].*\
")
set_tests_properties(${NAME} PROPERTIES FAIL_REGULAR_EXPRESSION "\
Result from network.*\
ECU1 DSNW CT01 log info V 4 \\[Log message 1 #.*\
Result from file.*\
ECU1 DSNW CT02 log info V 4 \\[Log message 2 #.*\
")
