cmake_minimum_required(VERSION 2.8.12)
project(IMock)

add_definitions("-std=c++11")
add_definitions("-g")
add_definitions("-Wall")

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/test/include)

file(GLOB_RECURSE IMOCK_TEST_SRCS "${PROJECT_SOURCE_DIR}/test/src/*.cpp")

add_executable(IMockTest ${IMOCK_TEST_SRCS})

# Link IMockTest with -lgcov.
target_link_libraries(IMockTest -lgcov)

# Enable test coverage.
target_compile_options(IMockTest PRIVATE "--coverage")
target_compile_options(IMockTest PRIVATE "-fprofile-arcs")
