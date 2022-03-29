set(NAME context_unreg_forbid)
set(MESSAGE_FILTER_CONFIGURATION_FILE ${CMAKE_CURRENT_SOURCE_DIR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dlt.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/dlt.conf)

add_executable(${NAME} ${NAME}.cpp)
target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_link_libraries(${NAME} PRIVATE dlt)

add_test(NAME ${NAME} COMMAND /bin/sh -e -c "\
$<TARGET_FILE:dlt-daemon> -c ${CMAKE_CURRENT_SOURCE_DIR}/dlt.conf &
sleep 0.2
$<TARGET_FILE:${NAME}> &
sleep 0.1
killall $<TARGET_FILE_NAME:dlt-daemon>
sleep 0.5
")

set_tests_properties(${NAME} PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${CTEST_LD_PATHS}")

set_tests_properties(${NAME} PROPERTIES PASS_REGULAR_EXPRESSION "\
dlt_unregister_context: Contexts and application are already unregistered")
