find_package(SDL2 REQUIRED)
set(luna_gfx_interface_sources buffer.cpp
                               image.cpp
                               device.cpp
                               render_pass.cpp
                               pipeline.cpp
                               bind_group.cpp
                               window.cpp
                               command_list.cpp
                               event.cpp
   )
add_library(luna_gfx_interface STATIC ${luna_gfx_interface_sources})
target_link_libraries(luna_gfx_interface PUBLIC luna_vulkan_impl SDL2::SDL2)