set(luna_gfx_interface_sources buffer.cpp
                               image.cpp
                               render_pass.cpp
                               pipeline.cpp
                               window.cpp
                               command_list.cpp
   )
add_library(luna_gfx_interface STATIC ${luna_gfx_interface_sources})
target_link_libraries(luna_gfx_interface PUBLIC luna_vulkan_impl)