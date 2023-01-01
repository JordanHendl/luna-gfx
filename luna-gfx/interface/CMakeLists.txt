set(luna_gfx_interface_headers buffer.hpp
                               image.hpp
                               device.hpp
                               render_pass.hpp
                               pipeline.hpp
                               bind_group.hpp
                               window.hpp
                               command_list.hpp
                               event.hpp
)

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
add_library(gfx_interface STATIC ${luna_gfx_interface_sources})
target_link_libraries(gfx_interface PRIVATE vulkan_impl SDL2::SDL2 Vulkan::Headers vulkan-memory-allocator::vulkan-memory-allocator)
install(FILES ${luna_gfx_interface_headers} DESTINATION ${header_install_dir}/interface COMPONENT devel)