cmake_minimum_required(VERSION 3.17)


project(glad)

### GLAD START
    add_library(glad STATIC)
    # set_target_properties(glad PROPERTIES CXX_STANDARD 17)
    target_include_directories(
            glad
        PRIVATE 
            ${CMAKE_CURRENT_LIST_DIR}/include
    )

    target_sources(
            glad
        PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/src/glad.c
    )
### GLAD END
