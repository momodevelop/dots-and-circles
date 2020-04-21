# Actually builds the libraries into lib
cmake_minimum_required(VERSION 3.17)

project(ryoji VERSION 0.1)

add_subdirectory(allocators)
add_subdirectory(math)
add_subdirectory(utils)

add_executable(test test-ryoji.cpp)
target_link_libraries(test lib-utils lib-allocators lib-math)
