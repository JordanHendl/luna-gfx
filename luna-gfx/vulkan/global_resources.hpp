#pragma once
#include "luna-gfx/common/dlloader.hpp"
#include "luna-gfx/interface/image.hpp"
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <vector>
#include <utility>
#include <atomic>
#include <unordered_map>
#include <array>
#include <map>
namespace luna {
namespace vulkan {
class Descriptor;
class Pipeline;
class Instance;
class Device;
class Buffer;
class Image;
class CommandBuffer;
class Descriptor;
class RenderPass;
class Swapchain;
class Window;

auto create_pool(Device& device, int queue_family) -> vk::CommandPool;
//inline auto begin_command_buffer(int32_t handle) -> void;
//inline auto end_command_buffer(int32_t handle) -> void;
//inline auto transition_image(int32_t cmd_id, int32_t image_id, vk::ImageLayout layout) -> void;
//inline auto copy_buffer_to_image(int32_t cmd_id, int32_t buffer_id, int32_t image_id) -> void;
//inline auto submit_command_buffer(int32_t handle) -> void;
//inline auto synchronize(int32_t cmd_buffer) -> void;

struct GlobalResources {
  std::map<vk::Device, std::map<int, vk::CommandPool>> pool_map;
  
  gfx::Dlloader vulkan_loader;
  std::unique_ptr<Instance> instance;
  std::vector<VmaAllocator> allocators;
  std::vector<Device> devices;
  std::vector<Buffer> buffers;
  std::vector<Image> images;
  //std::vector<vk::SurfaceKHR> surfaces;
  std::vector<CommandBuffer> cmds;
  std::vector<Pipeline> pipelines;
  std::vector<Descriptor> descriptors;
  std::vector<RenderPass> render_passes;
  std::vector<Swapchain> swapchains;
  std::vector<Window> windows;
  private:
    GlobalResources();
    ~GlobalResources();
    auto make_instance() -> void;
    auto find_gpus() -> void ;
    friend GlobalResources& global_resources();
};

auto global_resources() -> GlobalResources&;

template<typename T>
auto find_valid_entry(const T& container) -> size_t {
  auto index = 0u;
  for(const auto& a : container) {
    if(!a.valid()) {return index;}
    index++;
  }
  LunaAssert(false, "Ran out of space!");
  return 0;
}

//inline auto make_mappable_buffer(int gpu, size_t size) -> int32_t {
//  auto& res  = luna::vulkan::global_resources();
//  auto info = vk::BufferCreateInfo();
//  auto alloc_info = VmaAllocationCreateInfo{};
//  auto index = luna::vulkan::find_valid_entry(res.buffers);
//  auto& buffer = res.buffers[index];
//
//  luna::log_debug("Vulkan -> Creating mappable buffer on gpu ", gpu, " with size ", size);
//  buffer.gpu = gpu;
//  alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
//  alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
//  info.size = size;
//  info.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eStorageBuffer;
//
//  auto& c_info = static_cast<VkBufferCreateInfo&>(info);
//  auto c_buffer = static_cast<VkBuffer>(buffer.buffer);
//  vmaCreateBuffer(res.allocators[gpu], &c_info, &alloc_info, &c_buffer, &buffer.alloc, nullptr);
//  buffer.buffer = c_buffer;
//  vmaGetAllocationInfo(res.allocators[gpu], buffer.alloc, &buffer.info);
//  LunaAssert(c_buffer, "Could not create buffer given input parameters.");
//  return index;
//}
//
//inline auto destroy_buffer(int32_t handle) -> void {
//  auto& res  = luna::vulkan::global_resources();
//  auto& buffer = res.buffers[handle];
//  auto c_buffer = static_cast<VkBuffer>(buffer.buffer);
//  
//  luna::log_debug("Vulkan -> Creating destroying buffer ", handle);
//  LunaAssert(handle >= 0, "Attempting to use an invalid image.");
//  vmaDestroyBuffer(res.allocators[buffer.gpu], c_buffer, buffer.alloc);
//  buffer.buffer = nullptr;
//}
//
//
//inline auto create_cmd(int gpu, CommandBuffer* parent = nullptr) -> int32_t {
//  auto& res = global_resources();
//  auto index = find_valid_entry(res.cmds);
//  auto& cmd = res.cmds[index];
//  auto& device = res.devices[gpu];
//  auto pool = create_pool(device, device.graphics().id);
//  auto info = vk::CommandBufferAllocateInfo();
//  auto fence_info = vk::FenceCreateInfo();
//  info.setCommandBufferCount(1);
//  info.setCommandPool(pool);
//  info.setLevel(parent ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary);
//  cmd.cmd = error(device.gpu.allocateCommandBuffers(info, device.m_dispatch)).data()[0];
//  cmd.fence = error(device.gpu.createFence(fence_info, device.allocate_cb, device.m_dispatch));
//
//  cmd.signal_sems.resize(1);
//  cmd.signal_sems[0] = error(device.gpu.createSemaphore({}, device.allocate_cb, device.m_dispatch));
//  cmd.gpu = gpu;
//  return index;
//}
//
//inline auto destroy_cmd(int32_t handle) {
//  auto& res = global_resources();
//  auto& cmd = res.cmds[handle];
//  cmd.cmd = nullptr;
//}
//

//

//


}
}