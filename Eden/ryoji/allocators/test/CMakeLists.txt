cmake_minimum_required(VERSION 3.17)

project(ryoji-allocators-test VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(../include)

add_executable(ryoji-allocators-test main.cpp)