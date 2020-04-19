# Builds the math library

cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# NOTE: might only be compatible with VS2019?
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}/${CMAKE_GENERATOR_PLATFORM}/) 

include_directories(${CMAKE_SOURCE_DIR}/include/math)
project(math)
add_library(math STATIC vector.cpp matrix.cpp)

