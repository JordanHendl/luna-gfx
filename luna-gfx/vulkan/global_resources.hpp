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
struct Instance;
class Descriptor;
class RenderPass;
class Swapchain;
class Window;
struct Device;
struct CommandBuffer;
struct Image;
struct Buffer;
struct Semaphore;
auto create_pool(Device& device, int queue_family) -> vk::CommandPool;

struct GlobalResources {
  std::map<vk::Device, std::map<int, vk::CommandPool>> pool_map;
  
  gfx::Dlloader vulkan_loader;
  std::unique_ptr<Instance> instance;
  std::vector<VmaAllocator> allocators;
  std::vector<Device> devices;
  std::vector<Buffer> buffers;
  std::vector<Image> images;
  std::vector<std::vector<Semaphore>> semaphores;
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
}
}