cmake_minimum_required(VERSION 3.17)

project(utils VERSION 0.1 LANGUAGES CXX)

### IDE START 
    source_group(
        TREE 
            ${CMAKE_CURRENT_SOURCE_DIR}/test
        PREFIX 
            src 
        FILES 
            test/main.cpp
    )
    source_group(
        TREE 
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        PREFIX 
            include 
        FILES 
            include/defer.h
    )
### IDE END
    

### LIBRARY START
    add_library(lib-utils INTERFACE)
    target_include_directories(
            lib-utils 
        INTERFACE 
            ${PROJECT_SOURCE_DIR}/include
    )
    target_sources(lib-utils INTERFACE
        include/defer.h
    )
### LIBRARY END

### TEST START
    add_executable(test-utils)
    target_sources(test-utils PRIVATE test/main.cpp)
    target_compile_features(test-utils PRIVATE cxx_std_17)
    target_link_libraries(test-utils lib-utils)
### TEST END 


