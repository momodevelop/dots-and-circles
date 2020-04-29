cmake_minimum_required(VERSION 3.17)

set(include-allocators 
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/blk.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/fallback-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/freelist-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/heap-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/linear-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/local-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/log-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/null-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/segregator-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/slab-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/stack-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/allocators/stl-adapter.h
)
set(include-maths 
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/maths/matrix.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/maths/utils.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/maths/vector.h
)

set(include-utils 
    ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/utils/defer.h
)

set(src-allocators 
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/blk.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/fallback-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/freelist-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/heap-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/linear-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/local-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/log-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/null-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/segregator-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/slab-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/stack-allocator.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/stl-adapter.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/zawarudo/pointer.h
    ${CMAKE_CURRENT_LIST_DIR}/src/allocators/zawarudo/predef-freelist-strategies.h
)
set(src-maths 
    ${CMAKE_CURRENT_LIST_DIR}/src/maths/matrix.h
    ${CMAKE_CURRENT_LIST_DIR}/src/maths/matrix.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/maths/utils.h
    ${CMAKE_CURRENT_LIST_DIR}/src/maths/vector.h
    ${CMAKE_CURRENT_LIST_DIR}/src/maths/vector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/maths/zawarudo/constants.h
)

set(src-utils 
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/defer.h
)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/include/ryoji/ PREFIX "include" FILES ${include-allocators} ${include-maths} ${include-utils})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/src/ PREFIX "src" FILES ${src-allocators} ${src-maths} ${src-utils})


project(ryoji)

### RYOJI START
    add_library(ryoji STATIC)
    set_target_properties(ryoji PROPERTIES CXX_STANDARD 17)
    target_sources(
            ryoji
        PRIVATE
            ${include-allocators}
            ${include-utils}
            ${include-maths}
            ${src-allocators}
            ${src-utils}
            ${src-maths}
    )
### RYOJI END

### TEST START

    add_executable(ryoji-test-allocators)
    set_target_properties(ryoji-test-allocators PROPERTIES CXX_STANDARD 17)
    target_include_directories(ryoji-test-allocators PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/)
    target_sources(ryoji-test-allocators PRIVATE  ${CMAKE_CURRENT_LIST_DIR}/test/test-allocators.cpp)

    add_executable(ryoji-test-maths)
    set_target_properties(ryoji-test-maths PROPERTIES CXX_STANDARD 17)
    target_include_directories(ryoji-test-maths PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/)
    target_link_libraries(ryoji-test-maths PRIVATE ryoji)
    target_sources(ryoji-test-maths PRIVATE  ${CMAKE_CURRENT_LIST_DIR}/test/test-maths.cpp)

    add_executable(ryoji-test-utils)
    set_target_properties(ryoji-test-utils PROPERTIES CXX_STANDARD 17)
    target_include_directories(ryoji-test-utils PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/)
    target_sources(ryoji-test-utils PRIVATE  ${CMAKE_CURRENT_LIST_DIR}/test/test-utils.cpp)

### TEST END