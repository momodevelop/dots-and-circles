include(FindPackageHandleStandardArgs)

set(ryoji-include-dir ${CMAKE_CURRENT_SOURCE_DIR}/../ryoji/include)

find_package_handle_standard_args(glew REQUIRED_VARS ryoji-include-dir )
