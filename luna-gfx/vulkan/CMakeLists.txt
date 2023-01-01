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

add_library(vulkan_impl STATIC ${vulkan_impl_files})
target_link_libraries(vulkan_impl PRIVATE gfx_common vma_lib SDL2::SDL2  Vulkan::Headers  vulkan-memory-allocator::vulkan-memory-allocator) 