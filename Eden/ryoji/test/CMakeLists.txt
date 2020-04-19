# This will consolidate all the tests into one project

cmake_minimum_required(VERSION 3.10)

project(test-all VERSION 1.0)

add_subdirectory(allocators)
add_subdirectory(math)
add_subdirectory(utility)