include(FindPackageHandleStandardArgs)

set(sdl2-include-dir ${CMAKE_CURRENT_LIST_DIR}/../sdl2/include)


if(${CMAKE_SIZEOF_VOID_P} MATCHES 8)
    set(sdl2-libs-dir ${CMAKE_CURRENT_LIST_DIR}/../sdl2/lib/x64)
else() 
    set(sdl2-libs-dir ${CMAKE_CURRENT_LIST_DIR}/../sdl2/lib/x86)
    
endif()

find_package_handle_standard_args(sdl2 REQUIRED_VARS sdl2-include-dir sdl2-include-dir )
