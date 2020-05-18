cmake_minimum_required(VERSION 3.17)

find_package(ryoji REQUIRED)
find_package(sdl2 REQUIRED)
find_package(OpenGL REQUIRED)


set(includes 
    ${CMAKE_CURRENT_LIST_DIR}/include/yuu/graphics/canvases.h
    ${CMAKE_CURRENT_LIST_DIR}/include/yuu/graphics/windows.h
    ${CMAKE_CURRENT_LIST_DIR}/include/yuu/inputs/keyboard.h
)

set(srcs
    ${CMAKE_CURRENT_LIST_DIR}/src/inputs/keyboard.h
    ${CMAKE_CURRENT_LIST_DIR}/src/inputs/keyboard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/graphics/canvases.h
    ${CMAKE_CURRENT_LIST_DIR}/src/graphics/canvases.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/graphics/windows.h
    ${CMAKE_CURRENT_LIST_DIR}/src/graphics/windows.cpp
)


set(src-test
    ${CMAKE_CURRENT_LIST_DIR}/test/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/test/scene.h
    ${CMAKE_CURRENT_LIST_DIR}/test/input-test-scene.h
    ${CMAKE_CURRENT_LIST_DIR}/test/input-test-scene.cpp
)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/include/yuu/ PREFIX "include" FILES  ${includes})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/src/ PREFIX "src" FILES ${srcs})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/test/ PREFIX "src" FILES ${src-test})

project(yuu)

### YUU START
    add_library(yuu STATIC)
    set_target_properties(yuu PROPERTIES CXX_STANDARD 17)
    set_target_properties(yuu PROPERTIES LINKER_LANGUAGE CXX)
    target_include_directories(
            yuu 
        PRIVATE 
            ${CMAKE_CURRENT_LIST_DIR}/include/
            ${ryoji-include-dir}
            ${sdl2-include-dir}
    )
    target_sources(
            yuu
        PRIVATE
            ${includes}
            ${srcs}
    )

### YUU END

### TEST START

    add_executable(yuu-test)
    set_target_properties(yuu-test PROPERTIES CXX_STANDARD 17)
    set_target_properties(yuu-test PROPERTIES LINKER_LANGUAGE CXX)
    target_include_directories(
            yuu-test 
        PRIVATE 
            ${CMAKE_CURRENT_LIST_DIR}/include/
            ${ryoji-include-dir}
            ${sdl2-include-dir}
            ${glad-include-dir}
    )

    target_link_directories(
            yuu-test 
        PRIVATE 
            ${sdl2-libs-dir}
    )
    target_link_libraries(
            yuu-test 
        PRIVATE 
            ryoji
            glad
            yuu
            opengl32
            SDL2
            SDL2main
            SDL2test
    )
    target_sources(yuu-test PRIVATE  ${src-test})

    if(WIN32)
        # Copy DLL files to output directory
        add_custom_command(
            TARGET 
                yuu-test 
            POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${sdl2-libs-dir}/SDL2.dll $<TARGET_FILE_DIR:yuu-test>
        )

    endif()

### TEST END