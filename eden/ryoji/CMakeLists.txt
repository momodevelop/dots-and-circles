cmake_minimum_required(VERSION 3.17)

set(includes
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/blk.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/fallback-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/freelist-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/heap-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/linear-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/local-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/middleware-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/null-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/segregator-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/slab-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/stack-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/stl-adapter.h

    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/maths/matrices.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/maths/utils.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/maths/vectors.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/maths/bitfields.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/maths/constants.h
        
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/utils/defer.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/utils/expected.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/utils/reflection.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/utils/fmt.h
)

set(srcs 
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/blk.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/fallback-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/freelist-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/heap-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/linear-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/local-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/middleware-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/null-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/segregator-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/slab-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/stack-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/stl-adapter.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/zawarudo/pointer.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/zawarudo/predef-freelist-strategies.h

    ${CMAKE_CURRENT_LIST_DIR}/src/maths/matrices.h
    ${CMAKE_CURRENT_LIST_DIR}/src/maths/matrices.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/maths/vectors.h
    ${CMAKE_CURRENT_LIST_DIR}/src/maths/vectors.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/maths/bitfields.h
    ${CMAKE_CURRENT_LIST_DIR}/src/maths/constants.h
    ${CMAKE_CURRENT_LIST_DIR}/src/maths/utils.h
    
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/defer.h
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/expected.h
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/reflection.h
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/fmt.h
)

set(src-tests 
    ${CMAKE_CURRENT_LIST_DIR}/test/units/test-utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/test/units/test-allocators.cpp
    ${CMAKE_CURRENT_LIST_DIR}/test/units/test-maths.cpp
    ${CMAKE_CURRENT_LIST_DIR}/test/headers.h
    ${CMAKE_CURRENT_LIST_DIR}/test/main.cpp
)

source_group(
    TREE 
        ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/ 
    PREFIX 
        "include" 
    FILES 
        ${includes}
)
source_group(
    TREE 
        ${CMAKE_CURRENT_LIST_DIR}/src/ 
    PREFIX 
        "src" 
    FILES 
        ${srcs}
)
source_group(
    TREE 
        ${CMAKE_CURRENT_LIST_DIR}/test/ 
    PREFIX 
        "src" 
    FILES 
        ${src-tests}
)


project(ryoji)

### RYOJI START
    add_library(ryoji STATIC)
    set_target_properties(ryoji PROPERTIES CXX_STANDARD 17)
    set_target_properties(ryoji PROPERTIES LINKER_LANGUAGE CXX)
    target_sources(
            ryoji
        PRIVATE
            ${includes}
            ${srcs}
    )
### RYOJI END

### TEST START

    add_executable(ryoji-test)
    set_target_properties(ryoji-test PROPERTIES CXX_STANDARD 17)
    set_target_properties(ryoji-test PROPERTIES LINKER_LANGUAGE CXX)
    target_include_directories(ryoji-test PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/)
    target_link_libraries(ryoji-test PRIVATE ryoji)
    target_sources(ryoji-test PRIVATE  ${src-tests})



    #add_executable(ryoji-test-containers)
    #set_target_properties(ryoji-test-containers PROPERTIES CXX_STANDARD 17)
    #target_include_directories(ryoji-test-containers PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/)
    #target_sources(ryoji-test-containers PRIVATE  ${CMAKE_CURRENT_LIST_DIR}/test/test-containers.cpp)

### TEST END