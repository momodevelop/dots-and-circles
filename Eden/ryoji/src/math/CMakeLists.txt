cmake_minimum_required(VERSION 3.17)

include_directories(../../include/math)
project(math)
add_library(math STATIC vector.cpp matrix.cpp)

