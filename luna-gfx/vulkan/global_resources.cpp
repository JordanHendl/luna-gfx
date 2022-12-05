#include "luna-gfx/vulkan/vulkan_defines.hpp"
#include "luna-gfx/vulkan/global_resources.hpp"
#include "luna-gfx/error/error.hpp"
#include "luna-gfx/vulkan/instance.hpp"
#include "luna-gfx/vulkan/device.hpp"
#include "luna-gfx/vulkan/data_types.hpp"
#include "luna-gfx/vulkan/pipeline.hpp"
#include "luna-gfx/vulkan/descriptor.hpp"
#include "luna-gfx/vulkan/swapchain.hpp"
#include "luna-gfx/vulkan/render_pass.hpp"
#include "luna-gfx/vulkan/window.hpp"
#include "luna-gfx/vulkan/utils/helper_functions.hpp"
//#include "luna-gfx/vulkan/pipeline.hpp"
#include <memory>
#include <utility>
#include <vector>
namespace luna {
namespace vulkan {
constexpr auto MAX_OBJECT_AMT = 1024;
constexpr auto MAX_CMD_AMT = 64;
constexpr auto MAX_WINDOW_AMT = 10;

static auto get_pool_map() -> std::map<vk::Device, std::map<int, vk::CommandPool>>& {
  return global_resources().pool_map;
}

inline auto reset_command_pools(std::vector<Device>& devices) -> void {
  auto& pool_map = get_pool_map();
  for(auto& device_map : pool_map) {
    for(auto& pool : device_map.second) {
      for(auto& tmp : devices) {
        if(tmp.gpu == device_map.first && pool.second) {
          device_map.first.destroyCommandPool(pool.second, tmp.allocate_cb, tmp.m_dispatch);
        }
      }
    }
  }
}

auto create_pool(Device& device, int queue_family) -> vk::CommandPool {
  const auto flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;  // TODO make this configurable.
  
  auto& pool_map = get_pool_map();
  auto& queue_map = pool_map[device.gpu];
  auto iter = queue_map.find(queue_family);
  if (iter == queue_map.end()) {
    auto info = vk::CommandPoolCreateInfo();
    info.setFlags(flags);
    info.setQueueFamilyIndex(queue_family);
    auto pool = error(device.gpu.createCommandPool(
        info, device.allocate_cb, device.m_dispatch));
    iter = queue_map.insert(iter, {queue_family, pool});
  };
  return iter->second;
}

inline auto GlobalResources::make_instance() -> void {
  auto& loader = this->vulkan_loader;
#ifdef _WIN32
  loader.load("vulkan-1.dll");
#elif __linux__
  loader.load("libvulkan.so.1");
#endif

  LunaAssert(loader, "Failed to load the vulkan dynamic library (libvulkan.so/vulkan-1.dll).");
  this->instance = std::make_unique<Instance>(loader, nullptr, "luna_temp_app_name");
}

inline auto GlobalResources::find_gpus() -> void {
  this->devices.reserve(this->instance->m_devices.size());
  this->allocators.resize(this->instance->m_devices.size());
  for (auto& p_device : this->instance->m_devices) {
    auto device = Device();
    device = std::move(Device(this->vulkan_loader, nullptr,
                        p_device, *this->instance, this->instance->m_dispatch));

    this->devices.emplace_back(std::move(device));
  }

  // Need to sort the devices by score now (most 'powerful' gpu in the 0'th slot)
  auto compare = [](Device& a, Device& b) {
    return a.score() > b.score();
  };

  std::sort(this->devices.begin(), this->devices.end(), compare);

  //Now, we need to create the allocators for each valid device.
  for(auto index = 0u; index < this->devices.size(); index++) {
    auto alloc_create_info = VmaAllocatorCreateInfo();
    auto vulkan_funcs = VmaVulkanFunctions();
    auto sym = this->vulkan_loader.symbol("vkGetInstanceProcAddr");
    auto sym2 = this->vulkan_loader.symbol("vkGetDeviceProcAddr");
    vulkan_funcs = {};
    vulkan_funcs.vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(sym);
    vulkan_funcs.vkGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(sym2);

    alloc_create_info = {};
    alloc_create_info.vulkanApiVersion = VK_API_VERSION_1_2;
    alloc_create_info.physicalDevice = this->devices[index].physical_device;
    alloc_create_info.device = this->devices[index].gpu;
    alloc_create_info.pVulkanFunctions = &vulkan_funcs;
    alloc_create_info.instance = this->instance->m_instance;

    if(alloc_create_info.device && alloc_create_info.physicalDevice)
      vmaCreateAllocator(&alloc_create_info, &this->allocators[index++]);
  }
}

GlobalResources::GlobalResources() {
  this->make_instance();
  this->find_gpus();
  this->buffers.resize(MAX_OBJECT_AMT);
  this->images.resize(MAX_OBJECT_AMT);
  this->pipelines.resize(MAX_OBJECT_AMT);
  this->descriptors.resize(MAX_OBJECT_AMT);
  this->render_passes.resize(MAX_OBJECT_AMT);
  this->cmds.resize(MAX_CMD_AMT);
  this->swapchains.resize(MAX_WINDOW_AMT);
  this->windows.resize(MAX_WINDOW_AMT);
}

GlobalResources::~GlobalResources() {
  /** In vulkan, we have to be very careful of the order we destroy objects. Since this is a global
   * that contains all our vulkan objects, we need to be explicit in the order they are destroyed.
   * 
   * This is called when static deconstruction occurs at the end of the program, so we need to make
   * sure to clean up appropriately, and in order.
   * 
   * If a test fails because of this, FIX IT!!!!!
   */

  // These move-only objects need to be moved out of to properly be destroyed (have their deconstructors called).
  for(auto& dev : this->cmds) {
    auto tmp = std::move(dev);
  }

  for(auto& rp : this->render_passes) {
    auto tmp = std::move(rp);
  }

  for(auto& dev : this->swapchains) {
    auto tmp = std::move(dev);
  }

  for(auto& dev : this->descriptors) {
    auto tmp = std::move(dev);
  }

  for(auto& dev : this->pipelines) {
    auto tmp = std::move(dev);
  }

  for(auto& dev : this->windows) {
    auto tmp = std::move(dev);
  }


  // For data types, explicitly destroy them.
  auto index = 0;
  for(auto& a : this->buffers) {
    if(a.valid()) luna::vulkan::destroy_buffer(index++);
  }

  index = 0;
  for(auto& a : this->images) {
    if(a.valid()) luna::vulkan::destroy_image(index++);
  }

  for(auto& alloc : this->allocators) {
    if(alloc) vmaDestroyAllocator(alloc);
  }

  this->buffers.clear();
  this->images.clear();
  
  reset_command_pools(this->devices);

  for(auto& dev : this->devices) {
    auto tmp = std::move(dev);
  }

  {
    auto tmp = std::move(this->instance);
  }


}

auto global_resources() -> GlobalResources& {
  static auto global = GlobalResources();
  return global;
}
}
}