cmake_minimum_required(VERSION 3.17)

set(includes
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/utils.h
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

    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/maths/matrix.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/maths/utils.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/maths/vector.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/maths/bitfield.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/maths/constants.h
        
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/defer.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/maybe.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/either.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/reflection.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/fmt.h
)

set(srcs 
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/utils.h
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

    ${CMAKE_CURRENT_LIST_DIR}/src/maths/matrix.h
    ${CMAKE_CURRENT_LIST_DIR}/src/maths/matrix.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/maths/vector.h
    ${CMAKE_CURRENT_LIST_DIR}/src/maths/vector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/maths/bitfield.h
    ${CMAKE_CURRENT_LIST_DIR}/src/maths/constants.h
    ${CMAKE_CURRENT_LIST_DIR}/src/maths/utils.h
    
    ${CMAKE_CURRENT_LIST_DIR}/src/defer.h
    ${CMAKE_CURRENT_LIST_DIR}/src/maybe.h
    ${CMAKE_CURRENT_LIST_DIR}/src/either.h
    ${CMAKE_CURRENT_LIST_DIR}/src/reflection.h
    ${CMAKE_CURRENT_LIST_DIR}/src/fmt.h
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
    set_target_properties(ryoji PROPERTIES CXX_STANDARD 20)
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
    set_target_properties(ryoji-test PROPERTIES CXX_STANDARD 20)
    set_target_properties(ryoji-test PROPERTIES LINKER_LANGUAGE CXX)
    target_include_directories(ryoji-test PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/)
    target_link_libraries(ryoji-test PRIVATE ryoji)
    target_sources(ryoji-test PRIVATE  ${src-tests})



    #add_executable(ryoji-test-containers)
    #set_target_properties(ryoji-test-containers PROPERTIES CXX_STANDARD 17)
    #target_include_directories(ryoji-test-containers PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/)
    #target_sources(ryoji-test-containers PRIVATE  ${CMAKE_CURRENT_LIST_DIR}/test/test-containers.cpp)

### TEST END