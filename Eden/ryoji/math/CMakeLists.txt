cmake_minimum_required(VERSION 3.17)

project(math VERSION 0.1 LANGUAGES CXX)

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
            ${CMAKE_CURRENT_SOURCE_DIR}/src
        PREFIX 
            src 
        FILES 
            src/matrix.cpp
            src/vector.cpp
    )
    source_group(
        TREE 
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        PREFIX 
            include 
        FILES 
            include/matrix.h
            include/utils.h
            include/vector.h
            include/zawarudo/constants.h
    )
### IDE END
    

### LIBRARY START
    add_library(lib-math STATIC)
    target_include_directories(lib-math 
        PUBLIC 
            ${PROJECT_SOURCE_DIR}/include
    )
    target_sources(lib-math 
        PUBLIC
            include/matrix.h
            include/utils.h
            include/vector.h
            include/zawarudo/constants.h
        PRIVATE
            src/matrix.cpp
            src/vector.cpp
    )
    
    target_compile_features(lib-math PRIVATE cxx_std_17)
### LIBRARY END

### TEST START
    add_executable(test-math)
    target_sources(test-math PRIVATE test/main.cpp)
    target_compile_features(test-math PRIVATE cxx_std_17)
    target_link_libraries(test-math lib-math)
### TEST END 


