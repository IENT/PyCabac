cmake_minimum_required(VERSION 3.10)

project("pycabac")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set (CMAKE_CXX_STANDARD 11)

include_directories("${CMAKE_SOURCE_DIR}/src/libs")
include_directories("${CMAKE_SOURCE_DIR}/thirdparty")

add_subdirectory(thirdparty/pybind11)

add_subdirectory(src)
add_subdirectory(tests)
