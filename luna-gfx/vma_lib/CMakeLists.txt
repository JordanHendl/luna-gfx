add_library(vma_lib STATIC main.cpp)
target_include_directories(vma_lib PRIVATE ${vulkan-memory-allocator_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS})