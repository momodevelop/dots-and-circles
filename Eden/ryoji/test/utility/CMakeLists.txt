cmake_minimum_required(VERSION 3.17)

project(utility-test VERSION 1.0)

add_executable(utility-test 
    main.cpp
    ../../include/utility/defer.h
)
target_include_directories(utility-test PRIVATE ../../include/utility)