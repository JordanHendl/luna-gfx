add_subdirectory(common)
add_subdirectory(vma_lib)
add_subdirectory(vulkan)
add_subdirectory(interface)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/backend)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/backend)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/backend)
set(DOCUMENTATION_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/backend)


add_library(luna_gfx SHARED gfx.cpp)
target_link_libraries(luna_gfx PUBLIC luna_vulkan_impl luna_gfx_common luna_gfx_interface)
