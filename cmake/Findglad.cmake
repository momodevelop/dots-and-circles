include(FindPackageHandleStandardArgs)

set(glad-include-dir ${CMAKE_CURRENT_LIST_DIR}/../glad/include)

find_package_handle_standard_args(glad REQUIRED_VARS glad-include-dir )
