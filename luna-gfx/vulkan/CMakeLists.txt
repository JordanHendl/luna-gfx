find_package(SDL2 REQUIRED)
add_subdirectory(utils)
set(vulkan_impl_files
  global_resources.cpp
  device.cpp
  instance.cpp
  swapchain.cpp
  instance.cpp
  swapchain.cpp
  shader.cpp
  pipeline.cpp
  window.cpp
  descriptor.cpp
  render_pass.cpp
)

add_library(luna_vulkan_impl STATIC ${vulkan_impl_files})
target_link_libraries(luna_vulkan_impl PUBLIC luna_gfx_common luna_gfx_extended vma_lib SDL2::SDL2 luna_gfx_interface)