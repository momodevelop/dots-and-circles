cmake_minimum_required(VERSION 3.17)

project(allocators-test VERSION 1.0)

add_executable(allocators-test 
    main.cpp 
    ../../include/allocators/blk.h
    ../../include/allocators/fallback_allocator.h
    ../../include/allocators/freelist_allocator.h
    ../../include/allocators/heap_allocator.h
    ../../include/allocators/linear_allocator.h
    ../../include/allocators/local_allocator.h
    ../../include/allocators/log_allocator.h
    ../../include/allocators/null_allocator.h
    ../../include/allocators/segregator_allocator.h
    ../../include/allocators/slab_allocator.h
    ../../include/allocators/stack_allocator.h
    ../../include/allocators/stl_adapter.h
    ../../include/allocators/zawarudo/pointer.h
    ../../include/allocators/zawarudo/predef_freelist_strategies.h
)
target_include_directories(allocators-test PUBLIC INTERFACE ../../include/allocators)