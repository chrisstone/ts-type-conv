cmake_minimum_required(VERSION 3.14)
project(ts-type-conv CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/inc)

add_subdirectory(src)

enable_testing()
add_subdirectory(test/blackbox)
