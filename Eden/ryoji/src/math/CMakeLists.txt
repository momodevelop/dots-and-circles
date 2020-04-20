cmake_minimum_required(VERSION 3.17)

set(include_dir ../../include/math)
set(src_list vector.cpp matrix.cpp)
set(include_list 
    ${include_dir}/vector.h
    ${include_dir}/utils.h
    ${include_dir}/matrix.h
    ${include_dir}/zawarudo/constants.h
)

# For IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX src FILES ${src_list})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/${include_dir} PREFIX include FILES ${include_list})

project(math VERSION 0.1)
add_library(math STATIC ${src_list} ${include_list})

target_include_directories(math PRIVATE ${MY_INCLUDE_DIR})
target_compile_features(math PUBLIC cxx_std_17)
set_target_properties(math PROPERTIES CXX_EXTENTIONS OFF)



