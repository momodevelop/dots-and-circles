cmake_minimum_required(VERSION 3.17)

project(math-test VERSION 1.0)


add_executable(math-test 
    main.cpp
    ../../include/math/matrix.h
    ../../include/math/utils.h
    ../../include/math/vector.h
    ../../include/math/zawarudo/constants.h    
)
target_include_directories(math-test PRIVATE ../../include/math)