cmake_minimum_required(VERSION 3.17)

set(SRC_FILES main.cpp)
set(INCLUDE_FILES 
    ../include/fallback_allocator.h
    ../include/freelist_allocator.h
    ../include/heap_allocator.h
    ../include/linear_allocator.h
    ../include/local_allocator.h
    ../include/log_allocator.h
    ../include/null_allocator.h
    ../include/segregator_allocator.h
    ../include/slab_allocator.h
    ../include/stack_allocator.h
    ../include/stl_adapter.h
    ../include/zawarudo/pointer.h
    ../include/zawarudo/predef_freelist_strategies.h
)




source_group(
    TREE 
        ${CMAKE_CURRENT_SOURCE_DIR} 
    PREFIX 
        src 
    FILES 
       main.cpp
)
source_group(
    TREE 
        ${CMAKE_CURRENT_SOURCE_DIR}/../include 
    PREFIX 
        include 
    FILES 
        ../include/fallback_allocator.h
        ../include/freelist_allocator.h
        ../include/heap_allocator.h
        ../include/linear_allocator.h
        ../include/local_allocator.h
        ../include/log_allocator.h
        ../include/null_allocator.h
        ../include/segregator_allocator.h
        ../include/slab_allocator.h
        ../include/stack_allocator.h
        ../include/stl_adapter.h
        ../include/zawarudo/pointer.h
        ../include/zawarudo/predef_freelist_strategies.h
)

add_executable(test-allocators 
    main.cpp 
    ../include/fallback_allocator.h
    ../include/freelist_allocator.h
    ../include/heap_allocator.h
    ../include/linear_allocator.h
    ../include/local_allocator.h
    ../include/log_allocator.h
    ../include/null_allocator.h
    ../include/segregator_allocator.h
    ../include/slab_allocator.h
    ../include/stack_allocator.h
    ../include/stl_adapter.h
    ../include/zawarudo/pointer.h
    ../include/zawarudo/predef_freelist_strategies.h)

