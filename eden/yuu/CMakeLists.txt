cmake_minimum_required(VERSION 3.17)

# find_package(ryoji REQUIRED)
find_package(sdl2 REQUIRED)
find_package(glew REQUIRED)
find_package(OpenGL REQUIRED)

set(include-utils 
    ${CMAKE_CURRENT_LIST_DIR}/include/yuu/utils/window.h
)

set(src-utils 
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/window.h
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/window.cpp
)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/include/yuu/ PREFIX "include" FILES ${include-utils})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/src/ PREFIX "src" FILES ${src-utils})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/test/ PREFIX "test" FILES 
    ${CMAKE_CURRENT_LIST_DIR}/test/test.cpp
)

project(yuu)

### YUU START
    add_library(yuu STATIC)
    set_target_properties(yuu PROPERTIES CXX_STANDARD 17)
    target_include_directories(
            yuu 
        PRIVATE 
            ${CMAKE_CURRENT_LIST_DIR}/include/
            ${ryoji-include-dir}
            ${glew-include-dir}
            ${sdl2-include-dir}
    )
    target_sources(
            yuu
        PRIVATE
            ${include-utils}
            ${src-utils}
    )

### YUU END

### TEST START

    add_executable(yuu-test)
    set_target_properties(yuu-test PROPERTIES CXX_STANDARD 17)
    target_include_directories(
            yuu-test 
        PRIVATE 
            ${CMAKE_CURRENT_LIST_DIR}/include/
            ${ryoji-include-dir}
            ${glew-include-dir}
            ${sdl2-include-dir}
    )

    target_link_directories(
            yuu-test 
        PRIVATE 
            ${glew-libs-dir}
            ${sdl2-libs-dir}
    )
    target_link_libraries(
            yuu-test 
        PRIVATE 
            ryoji
            yuu
            opengl32
            SDL2
            SDL2main
            SDL2test
            glew32
            glew32s
    )
    target_sources(yuu-test PRIVATE  ${CMAKE_CURRENT_LIST_DIR}/test/test.cpp)

    if(WIN32)
        # Copy DLL files to output directory
        add_custom_command(
            TARGET 
                yuu-test 
            POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${sdl2-libs-dir}/SDL2.dll ${CMAKE_CURRENT_BINARY_DIR}
        )

    endif()

### TEST END