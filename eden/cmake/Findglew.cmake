include(FindPackageHandleStandardArgs)

set(glew-include-dir ${CMAKE_CURRENT_SOURCE_DIR}/../glew/include)


if(${CMAKE_SIZEOF_VOID_P} MATCHES 8)
    set(glew-libs-dir ${CMAKE_CURRENT_SOURCE_DIR}/../glew/lib/x64)
else() 
    set(glew-libs-dir ${CMAKE_CURRENT_SOURCE_DIR}/../glew/lib/x86)
endif()

set(glew-libs
    ${glew-libs-dir}/glew32.lib
    ${glew-libs-dir}/glew32s.lib
)
find_package_handle_standard_args(glew REQUIRED_VARS glew-include-dir glew-include-dir )
