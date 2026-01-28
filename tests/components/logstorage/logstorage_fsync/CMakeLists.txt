set(NAME logstorage_fsync)

# Build c-mock function fsync
add_library(logfsync SHARED logfsync.cpp)
target_link_libraries(logfsync PRIVATE ${CMAKE_DL_LIBS})

# Build test itself
add_executable(${NAME} ${NAME}.cpp)
target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_link_libraries(${NAME} PRIVATE dlt)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dlt_logstorage.conf ${CMAKE_CURRENT_BINARY_DIR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run_test.sh ${CMAKE_CURRENT_BINARY_DIR}/run_test.sh COPYONLY)

set(APPID "FSNC")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dlt.conf.in ${CMAKE_CURRENT_BINARY_DIR}/dlt.conf)
if(WITH_DLT_INSTALLED_TESTS)
    install(TARGETS ${NAME} RUNTIME DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_fsync)
    install(FILES dlt.conf.in DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_fsync RENAME dlt.conf)
    install(FILES dlt_logstorage.conf DESTINATION ${DLT_TEST_DIR}/components/logstorage/logstorage_fsync)
endif(WITH_DLT_INSTALLED_TESTS)

add_test(NAME ${NAME} COMMAND /bin/sh -e
    ${CMAKE_CURRENT_BINARY_DIR}/run_test.sh
    $<TARGET_FILE:dlt-daemon>
    $<TARGET_FILE:${NAME}>
    1
    $<TARGET_FILE:dlt-convert>
)

set_tests_properties(${NAME} PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${CTEST_LD_PATHS};LD_PRELOAD=$<TARGET_FILE:logfsync>")

set_tests_properties(${NAME} PROPERTIES PASS_REGULAR_EXPRESSION "fsync")
