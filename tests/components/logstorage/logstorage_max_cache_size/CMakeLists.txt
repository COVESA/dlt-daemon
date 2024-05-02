set(NAME logstorage_max_cache_size)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dlt.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf)

add_executable(${NAME} ${NAME}.cpp)
target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(${NAME} PRIVATE dlt)
if(WITH_DLT_INSTALLED_TESTS)
    install(TARGETS ${NAME} RUNTIME DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_max_cache_size)
    install(FILES dlt.conf.in DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_max_cache_size RENAME dlt.conf)
    install(FILES dlt_logstorage.conf DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_max_cache_size)
endif(WITH_DLT_INSTALLED_TESTS)

add_test(NAME ${NAME} COMMAND /bin/sh -e -c "
if [ ${CMAKE_CURRENT_SOURCE_DIR} != ${CMAKE_CURRENT_BINARY_DIR} ];
then cp ${CMAKE_CURRENT_SOURCE_DIR}/dlt_logstorage.conf ${CMAKE_CURRENT_BINARY_DIR}/;
fi
$<TARGET_FILE:dlt-daemon> -c ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf &
sleep 0.5
$<TARGET_FILE:${NAME}>
sleep 0.5
killall $<TARGET_FILE:dlt-daemon>
")

set(PASSEXPRESSION "Max size of Logstorage Cache already used")

set_tests_properties(${NAME} PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${CTEST_LD_PATHS}")
set_tests_properties(${NAME} PROPERTIES PASS_REGULAR_EXPRESSION ${PASSEXPRESSION})
