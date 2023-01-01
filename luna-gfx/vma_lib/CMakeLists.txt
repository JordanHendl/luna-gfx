add_library(vma_lib STATIC main.cpp)
target_link_libraries(vma_lib PRIVATE Vulkan::Headers vulkan-memory-allocator::vulkan-memory-allocator)