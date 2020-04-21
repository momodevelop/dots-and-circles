cmake_minimum_required(VERSION 3.17)

project(allocators VERSION 0.1 LANGUAGES CXX)

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
            include/fallback_allocator.h
            include/freelist_allocator.h
            include/heap_allocator.h
            include/linear_allocator.h
            include/local_allocator.h
            include/log_allocator.h
            include/null_allocator.h
            include/segregator_allocator.h
            include/slab_allocator.h
            include/stack_allocator.h
            include/stl_adapter.h
            include/zawarudo/pointer.h
            include/zawarudo/predef_freelist_strategies.h
    )
### IDE END
    

### LIBRARY START
    add_library(lib-allocators INTERFACE)
    target_include_directories(
            lib-allocators 
        INTERFACE 
            ${PROJECT_SOURCE_DIR}/include
    )
    target_sources(lib-allocators INTERFACE
        include/fallback_allocator.h
        include/freelist_allocator.h
        include/heap_allocator.h
        include/linear_allocator.h
        include/local_allocator.h
        include/log_allocator.h
        include/null_allocator.h
        include/segregator_allocator.h
        include/slab_allocator.h
        include/stack_allocator.h
        include/stl_adapter.h
        include/zawarudo/pointer.h
        include/zawarudo/predef_freelist_strategies.h
    )

### LIBRARY END

### TEST START
    add_executable(test-allocators)
    target_sources(test-allocators PRIVATE test/main.cpp)
    target_compile_features(test-allocators PRIVATE cxx_std_17)
    target_link_libraries(test-allocators lib-allocators)
### TEST END 


