# Actually builds the libraries into lib
cmake_minimum_required(VERSION 3.17)

project(eden VERSION 0.1)
##### RYOJI START
    source_group(
        TREE 
            ryoji/math/
        PREFIX 
            src/math
        FILES 
            ryoji/math/matrix.cpp
            ryoji/math/vector.cpp )
    source_group(
        TREE 
            ryoji/allocators
        PREFIX 
            inc/allocators
        FILES 
            ryoji/allocators/fallback_allocator.h
            ryoji/allocators/freelist_allocator.h
            ryoji/allocators/heap_allocator.h
            ryoji/allocators/linear_allocator.h
            ryoji/allocators/local_allocator.h
            ryoji/allocators/log_allocator.h
            ryoji/allocators/null_allocator.h
            ryoji/allocators/segregator_allocator.h
            ryoji/allocators/slab_allocator.h
            ryoji/allocators/stack_allocator.h
            ryoji/allocators/stl_adapter.h
            ryoji/allocators/zawarudo/pointer.h
            ryoji/allocators/zawarudo/predef_freelist_strategies.h )
    source_group(
        TREE 
            ryoji/math
        PREFIX 
            inc/math 
        FILES 
            ryoji/math/matrix.h
            ryoji/math/utils.h
            ryoji/math/vector.h
            ryoji/math/zawarudo/constants.h )
    source_group(
        TREE 
            ryoji/utils
        PREFIX 
            inc/utils 
        FILES 
            ryoji/utils/defer.h )
    
    add_library(ryoji)
    target_compile_features(ryoji PRIVATE cxx_std_17)
    target_sources(
            ryoji 
        PRIVATE
            ryoji/allocators/fallback_allocator.h
            ryoji/allocators/freelist_allocator.h
            ryoji/allocators/heap_allocator.h
            ryoji/allocators/linear_allocator.h
            ryoji/allocators/local_allocator.h
            ryoji/allocators/log_allocator.h
            ryoji/allocators/null_allocator.h
            ryoji/allocators/segregator_allocator.h
            ryoji/allocators/slab_allocator.h
            ryoji/allocators/stack_allocator.h
            ryoji/allocators/stl_adapter.h
            ryoji/allocators/zawarudo/pointer.h
            ryoji/allocators/zawarudo/predef_freelist_strategies.h

            ryoji/math/matrix.h
            ryoji/math/utils.h
            ryoji/math/vector.h
            ryoji/math/zawarudo/constants.h

            ryoji/utils/defer.h
            
            ryoji/math/matrix.cpp
            ryoji/math/vector.cpp
    )
##### RYOJI END

##### TEST_RYOJI START
    source_group(
        TREE 
            ryoji_test/
        PREFIX 
            src
        FILES 
            ryoji_test/test_allocators.cpp
            ryoji_test/test_math.cpp
            ryoji_test/test_utils.cpp )


    
    add_executable(             ryoji-test-allocators   ryoji_test/test_allocators.cpp )
    target_include_directories( ryoji-test-allocators   PRIVATE  ryoji/allocators)
    target_compile_features(    ryoji-test-allocators   PRIVATE  cxx_std_17)

    add_executable(             ryoji-test-math         ryoji_test/test_math.cpp )
    target_include_directories( ryoji-test-math         PRIVATE  ryoji/math)
    target_link_libraries(      ryoji-test-math         PRIVATE  ryoji)
    target_compile_features(    ryoji-test-math         PRIVATE  cxx_std_17)

    add_executable(             ryoji-test-utils        ryoji_test/test_utils.cpp )
    target_include_directories( ryoji-test-utils        PRIVATE  ryoji/utils)
    target_compile_features(    ryoji-test-utils        PRIVATE  cxx_std_17)
##### TEST_RYOJI END

##### YUU START
    source_group(
        TREE 
            yuu/utils/
        PREFIX 
            src/utils
        FILES 
            yuu/utils/window.cpp
            yuu/utils/vector.cpp )
    source_group(
        TREE 
            yuu/utils/
        PREFIX 
            inc/utils
        FILES 
            yuu/utils/window.h )

    add_library(yuu)
    target_include_directories( yuu     PUBLIC      ${PROJECT_SOURCE_DIR}/ deps/sdl2/include/ deps/glew/include/)
    target_compile_features(    yuu     PRIVATE     cxx_std_17)
    target_link_libraries(      yuu     PRIVATE     ryoji)
    target_sources(
            yuu 
        PRIVATE
            yuu/utils/window.h
            yuu/utils/window.cpp
    )
##### YUU END


##### TEST_YUU START
    source_group(
        TREE 
            yuu_test/
        PREFIX 
            src
        FILES 
            yuu_test/test.cpp )


    
    add_executable(             yuu-test    yuu_test/test.cpp )
    target_include_directories( yuu-test    PRIVATE  yuu)
    target_compile_features(    yuu-test    PRIVATE  cxx_std_17)

##### TEST_YUU END
