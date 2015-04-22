# Setup testing
enable_testing()
find_package(GTest)
include_directories(${gtest_SOURCE_DIR}/include)

add_executable(gtest_dlt_user gtest_dlt_user.cpp)

# Link test executable against gtest & gtest_main
target_link_libraries(gtest_dlt_user gtest gtest_main dlt)

