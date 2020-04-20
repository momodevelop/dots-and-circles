# Actually builds the libraries into lib
cmake_minimum_required(VERSION 3.17)

project(ryoji VERSION 0.1)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_subdirectory(src)
