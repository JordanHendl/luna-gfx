#include "luna-gfx/vulkan/vulkan_defines.hpp"
#include "luna-gfx/vulkan/device.hpp"
#include "luna-gfx/error/error.hpp"
#include "luna-gfx/vulkan/instance.hpp"
#include "luna-gfx/vulkan/global_resources.hpp"
#include <memory>
#include <utility>
namespace luna {
namespace vulkan {

Device::Device() {
  this->allocate_cb = nullptr;
  this->m_score = 0.0f;
}

Device::Device(gfx::Dlloader& loader, vk::AllocationCallbacks* callback,
                        vk::PhysicalDevice device, Instance& instance, vk::DispatchLoaderDynamic& dispatch) {
  this->extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  this->extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
  this->m_score = 0.0f;
  this->physical_device = device;
  this->allocate_cb = callback;

  this->properties = device.getProperties(dispatch);
  if(this->properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
    this->m_score += 50;
  }
  if(this->properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu) return ;
  this->find_queue_families(dispatch);
  this->make_device(dispatch);

  this->find_memory_info();
  this->mem_prop = device.getMemoryProperties(dispatch);

  this->m_dispatch.init(instance.m_instance,
                        reinterpret_cast<PFN_vkGetInstanceProcAddr>(
                            loader.symbol("vkGetInstanceProcAddr")),
                        this->gpu);

  this->find_queues();
}

Device::Device(Device&& mv) { *this = std::move(mv); }

Device::~Device() {
  if (this->gpu) {
    this->gpu.destroy(this->allocate_cb, global_resources().instance->m_dispatch);
  }
}

auto Device::wait_idle() -> void {
  error(this->gpu.waitIdle(this->m_dispatch));
}

auto Device::operator=(Device&& mv) -> Device& {
  this->allocate_cb = mv.allocate_cb;
  this->gpu = mv.gpu;
  this->physical_device = mv.physical_device;
  this->surface = mv.surface;
  this->queue_props = mv.queue_props;
  this->properties = mv.properties;
  this->mem_prop = mv.mem_prop;
  this->mem_heaps = mv.mem_heaps;
  this->features = mv.features;
  this->m_dispatch = mv.m_dispatch;
  this->queues = mv.queues;
  this->id = mv.id;
  this->extensions = mv.extensions;
  this->validation = mv.validation;
  this->m_score = mv.m_score;

  mv.allocate_cb = nullptr;
  mv.gpu = nullptr;
  mv.physical_device = nullptr;
  mv.surface = nullptr;
  mv.mem_prop = vk::PhysicalDeviceMemoryProperties();
  mv.mem_heaps = {};
  mv.properties = vk::PhysicalDeviceProperties();
  mv.features = vk::PhysicalDeviceFeatures();
  mv.id = 0;
  mv.m_score = 0.f;
  mv.queue_props.clear();
  mv.extensions.clear();
  mv.validation.clear();
  for (auto& q : mv.queues) q = Queue();

  return *this;
}

auto Device::score() -> float {
  return this->m_score;
}

auto Device::find_queue_families(vk::DispatchLoaderDynamic& dispatch) -> void {
  this->queue_props = this->physical_device.getQueueFamilyProperties(
      dispatch);

  for (unsigned id = 0; id < this->queue_props.size(); id++) {
    auto& family = this->queue_props[id];

    auto queue_flag = static_cast<VkQueueFlags>(family.queueFlags);

    if (queue_flag & VK_QUEUE_GRAPHICS_BIT && id != this->queues[COMPUTE].id &&
        this->queues[GRAPHICS].id == UINT_MAX) {
      this->queues[GRAPHICS].id = id;
    } else if (queue_flag & VK_QUEUE_COMPUTE_BIT &&
               id != this->queues[GRAPHICS].id &&
               this->queues[COMPUTE].id == UINT_MAX) {
      this->queues[COMPUTE].id = id;
    } else if (queue_flag & VK_QUEUE_TRANSFER_BIT &&
               id != this->queues[SPARSE].id &&
               this->queues[TRANSFER].id == UINT_MAX) {
      this->queues[TRANSFER].id = id;
    } else if (queue_flag & VK_QUEUE_SPARSE_BINDING_BIT &&
               this->queues[SPARSE].id != UINT_MAX) {
      this->queues[SPARSE].id = id;
    }
  }
}

auto Device::make_device(vk::DispatchLoaderDynamic& dispatch) -> void {
  using StringVec = std::vector<const char*>;
  using QueueInfos = std::vector<vk::DeviceQueueCreateInfo>;

  auto& info = this->m_device_info.get<vk::DeviceCreateInfo>();

  StringVec extensions;
  StringVec validation;
  QueueInfos queue_infos;

  extensions = this->make_extensions(dispatch);
  validation = this->make_layers(dispatch);

  for (const auto& queue : this->queues) {
    if (queue.id != UINT_MAX) {
      vk::DeviceQueueCreateInfo info;

      info.setQueueFamilyIndex(queue.id);
      info.setQueueCount(1);
      info.setPQueuePriorities(&queue.priority);

      queue_infos.push_back(info);
    }
  }

  vk::PhysicalDeviceProperties2 props;

  auto& features2 = this->m_device_info.get<vk::PhysicalDeviceFeatures2>();
  features2.setFeatures(this->features);
  this->features.setShaderInt64(true);
  this->features.setFragmentStoresAndAtomics(true);
  this->features.setVertexPipelineStoresAndAtomics(true);

  info.setQueueCreateInfos(queue_infos);
  info.setEnabledExtensionCount(extensions.size());
  info.setPpEnabledExtensionNames(extensions.data());
  info.setEnabledLayerCount(validation.size());
  info.setPpEnabledLayerNames(validation.data());
  info.pNext = nullptr;
  //      info.setPNext                  ( &this->pnext_chain ) ;
  error(this->physical_device.createDevice(&info, this->allocate_cb, &this->gpu,
                                           dispatch));
}

auto Device::make_extensions(vk::DispatchLoaderDynamic& dispatch) -> std::vector<const char*> {
  std::vector<const char*> list;

  auto available_extentions =
      error(this->physical_device.enumerateDeviceExtensionProperties(
          nullptr, dispatch));

  auto copy = this->extensions;

  this->extensions.clear();
  for (const auto& ext : available_extentions) {
    for (const auto& requested : copy) {
      if (std::string(&ext.extensionName[0]) == requested) {
        this->extensions.push_back(requested);
        list.push_back(this->extensions.back().data());
      }
    }
  }

  return list;
}

auto Device::check_limits() -> void {
  if(!this->gpu) return;

  auto all_required_features_supported = 
  this->properties.limits.timestampComputeAndGraphics;

  LunaAssert(all_required_features_supported, "Limited GPU functionality not found on this graphics card. Sorry dude :(");
}

auto Device::make_layers(vk::DispatchLoaderDynamic& dispatch) -> std::vector<const char*> {
  std::vector<const char*> list;
  auto available_layers =
      error(this->physical_device.enumerateDeviceLayerProperties(dispatch));
  auto copy = this->validation;

  this->validation.clear();
  for (const auto& ext : available_layers) {
    for (const auto& requested : copy) {
      if (std::string(&ext.layerName[0]) == requested) {
        this->validation.push_back(requested);
        list.push_back(this->validation.back().data());
      }
    }
  }

  return list;
}

auto Device::find_queues() -> void {
  /* Check and see if we found Queues. If not, set to the 'last available queue'
   * as to not break if people need to use say, a compute queue even though
   * teeeeeechnically their device doesn't support it.
   */
  if (this->queues[GRAPHICS].id != UINT_MAX) {
    this->queues[GRAPHICS].queue =
        this->gpu.getQueue(this->queues[GRAPHICS].id, 0, this->m_dispatch);
  }

  // Set graphics to the defacto default because this queue is guaranteed to be
  // able to to everything + graphics.
  auto last_queue = this->queues[GRAPHICS].queue;

  if (this->queues[COMPUTE].id != UINT_MAX) {
    this->queues[COMPUTE].queue =
        this->gpu.getQueue(this->queues[COMPUTE].id, 0, this->m_dispatch);
    last_queue = this->queues[COMPUTE].queue;
  } else {
    this->queues[COMPUTE].queue = last_queue;
  }

  if (this->queues[TRANSFER].id != UINT_MAX) {
    this->queues[TRANSFER].queue =
        this->gpu.getQueue(this->queues[TRANSFER].id, 0, this->m_dispatch);
    last_queue = this->queues[TRANSFER].queue;
  } else {
    this->queues[COMPUTE].queue = last_queue;
  }

  if (this->queues[SPARSE].id != UINT_MAX) {
    this->queues[SPARSE].queue =
        this->gpu.getQueue(this->queues[SPARSE].id, 0, this->m_dispatch);
    last_queue = this->queues[SPARSE].queue;
  } else {
    this->queues[SPARSE].queue = last_queue;
  }
}

auto Device::find_memory_info() -> void {
  this->mem_heaps.resize(mem_prop.memoryHeapCount);
  for (auto index = 0u; index < mem_prop.memoryTypeCount; index++) {
    auto& vk_type = this->mem_prop.memoryTypes[index];
    auto& vk_heap = this->mem_prop.memoryHeaps[vk_type.heapIndex];
    auto& heap = this->mem_heaps[vk_type.heapIndex];

    heap.size = vk_heap.size;
    auto host_visible =
        vk_type.propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible ||
        vk_type.propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent;

    auto device_capable =
        vk_type.propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal;

    if (host_visible) heap.type = HeapType::HostVisible | heap.type;
    if (device_capable) heap.type = HeapType::GpuOnly | heap.type;
  }
}

auto Device::check_support(vk::SurfaceKHR surface) const -> void {
  error(this->physical_device.getSurfaceSupportKHR(this->queues[GRAPHICS].id,
                                              surface, this->m_dispatch));
}
}  // namespace vulkan
}  // namespace luna
