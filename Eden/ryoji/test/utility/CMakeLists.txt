cmake_minimum_required(VERSION 3.10)

project(utility-test VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(../../include/utility)

add_executable(utility-test main.cpp)