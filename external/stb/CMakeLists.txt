execute_process( 
  COMMAND tar -xf ${CMAKE_CURRENT_SOURCE_DIR}/stb_tools.tar.gz
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

set(stb_tools_include_dirs ${CMAKE_CURRENT_BINARY_DIR}/stb_tools/include CACHE INTERNAL "stb tools include dirs")
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/stb_tools ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/stb_tools)