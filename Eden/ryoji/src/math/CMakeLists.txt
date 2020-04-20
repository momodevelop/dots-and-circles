cmake_minimum_required(VERSION 3.17)

set(here ${CMAKE_CURRENT_SOURCE_DIR} )
set(include_dir ${here}/../../include/math)
set(src_list vector.cpp matrix.cpp)
set(include_list 
    ${include_dir}/vector.h
    ${include_dir}/utils.h
    ${include_dir}/matrix.h
    ${include_dir}/zawarudo/constants.h
)
set(output_dir ${here}/../../lib/${CMAKE_SYSTEM_NAME})


set(target_name math)

source_group(TREE ${here} PREFIX src FILES ${src_list})
source_group(TREE ${include_dir} PREFIX include FILES ${include_list})

project(mat${target_name}h VERSION 0.1)
add_library(${target_name} STATIC ${src_list} ${include_list})

target_include_directories(${target_name} PRIVATE ${include_dir})
target_compile_features(${target_name} PUBLIC cxx_std_17)
set_target_properties(${target_name} PROPERTIES CXX_EXTENTIONS OFF)