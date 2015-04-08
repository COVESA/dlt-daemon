# Setup testing
enable_testing()
find_package(GTest)
include_directories(${gtest_SOURCE_DIR}/include)

add_executable(hello hello.cpp)

# Link test executable against gtest & gtest_main
target_link_libraries(hello gtest gtest_main dlt)

