#pragma once 
#include "luna-gfx/vulkan/vulkan_defines.hpp"
#include "luna-gfx/interface/image.hpp"
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <vector>
namespace luna {
namespace vulkan {
struct Buffer {
  vk::Buffer buffer = {};
  VmaAllocation alloc = {};
  VmaAllocationInfo info = {};
  std::size_t size = 0;
  int gpu = -1;
  auto valid() const -> bool {return this->buffer;}
};

struct Image {
  vk::Image image = {};
  vk::Sampler sampler = {};
  vk::ImageView view = {};
  vk::ImageUsageFlags usage = {};
  vk::Format format = {};
  vk::ImageViewType view_type = {};
  vk::ImageSubresourceLayers subresource = {};
  vk::ImageMemoryBarrier barrier = {};
  std::size_t layer = 0;
  vk::ImageLayout layout = {};
  VmaAllocation alloc = {};
  gfx::ImageInfo info = {};
  bool imported = false;

  auto valid() const -> bool {return this->image;}
};

struct CommandBuffer {
  vk::CommandBuffer cmd = {};
  vk::CommandBufferBeginInfo begin_info = {};
  std::vector<vk::Semaphore> wait_sems = {};
  std::vector<vk::Semaphore> signal_sems = {};
  vk::SubmitInfo submit_info = {};
  vk::Fence fence = {};
  int gpu = -1;
  int32_t rp_id = -1;
  
  CommandBuffer* parent = nullptr;
  bool signaled = false;
  auto valid() const -> bool {return this->cmd;}
};
}
}