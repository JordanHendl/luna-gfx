add_subdirectory(common)
add_subdirectory(vma_lib)
add_subdirectory(vulkan)
add_subdirectory(interface)
add_subdirectory(ext)
add_library(gfx STATIC gfx.cpp)
target_link_libraries(gfx PRIVATE gfx_extended
                                  vulkan_impl
                                  gfx_common
                                  gfx_interface
                                  vma_lib
                                  spirv_reflect
                                  stb_tools
                                  )

install(FILES gfx.hpp DESTINATION ${header_install_dir} COMPONENT devel)