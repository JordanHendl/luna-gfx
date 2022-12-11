#pragma once
#include "luna-gfx/vulkan/vulkan_defines.hpp"
#include "luna-gfx/common/dlloader.hpp"
#include <array>
#include <climits>
#include <mutex>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>
namespace luna {
namespace vulkan {
class Instance;

enum class HeapType : int {
  GpuOnly = 1 << 1,
  HostVisible = 1 << 2,
  LazyAllocation = 1 << 3,
};

struct GpuMemoryHeap {
  HeapType type = HeapType::GpuOnly;
  size_t size = 0;
};

inline auto operator|(const HeapType& a, const HeapType& b) -> HeapType {
  return static_cast<HeapType>(static_cast<int>(a) | static_cast<int>(b));
}

inline auto operator|=(const HeapType& a, const HeapType& b) -> HeapType {
  return static_cast<HeapType>(static_cast<int>(a) | static_cast<int>(b));
}

inline auto operator&(const HeapType& a, const HeapType& b) -> bool {
  return static_cast<HeapType>(static_cast<int>(a) & static_cast<int>(b)) == b;
}

struct Queue {
  vk::Queue queue;
  float priority = 1.0f;
  unsigned id = UINT_MAX;
  std::mutex lock;

  auto operator=(const Queue& cpy) -> Queue& {
    this->queue = cpy.queue;
    this->priority = cpy.priority;
    this->id = cpy.id;
    return *this;
  }
};

enum QueueTypes {
  GRAPHICS = 0,
  COMPUTE = 1,
  TRANSFER = 2,
  SPARSE = 3,
  NUM_QUEUES = 4
};

struct Device {
  Device();
  Device(gfx::Dlloader& loader, vk::AllocationCallbacks* callback,
                  vk::PhysicalDevice device, Instance& instance, vk::DispatchLoaderDynamic& dispatch);
  Device(Device&& mv);
  Device(const Device& cpy) = delete;
  ~Device();
  auto operator=(const Device& cpy) -> Device& = delete;
  auto operator=(Device&& cpy) -> Device&;
  auto score() -> float;
  auto check_support(vk::SurfaceKHR surface) const -> void;
  auto wait_idle() -> void;
  [[nodiscard]] inline auto graphics() -> Queue& { return this->queues[GRAPHICS]; }
  [[nodiscard]] inline auto compute() -> Queue& { return this->queues[COMPUTE]; }
  [[nodiscard]] inline auto transfer() -> Queue& { return this->queues[TRANSFER]; }
  [[nodiscard]] inline auto sparse() -> Queue& { return this->queues[SPARSE]; }
  using FeaturesChain = vk::StructureChain<vk::DeviceCreateInfo, vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceShaderAtomicFloat2FeaturesEXT, vk::PhysicalDeviceShaderAtomicFloatFeaturesEXT>;
  vk::AllocationCallbacks* allocate_cb;
  vk::Device gpu;
  vk::PhysicalDevice physical_device;
  vk::SurfaceKHR surface;
  std::vector<vk::QueueFamilyProperties> queue_props;
  std::vector<GpuMemoryHeap> mem_heaps;
  vk::PhysicalDeviceProperties properties;
  vk::PhysicalDeviceFeatures features;
  FeaturesChain m_device_info;
  vk::PhysicalDeviceMemoryProperties mem_prop;
  vk::DispatchLoaderDynamic m_dispatch;
  std::array<Queue, 4> queues;
  unsigned id;
  std::vector<std::string> extensions;
  std::vector<std::string> validation;
  float m_score;

  private:
    inline auto check_limits() -> void;
    inline auto find_memory_info() -> void;
    inline auto find_queues() -> void;
    inline auto find_queue_families(vk::DispatchLoaderDynamic& dispatch) -> void;
    inline auto make_device(vk::DispatchLoaderDynamic& dispatch) -> void;
    inline auto make_extensions(vk::DispatchLoaderDynamic& dispatch) -> std::vector<const char*>;
    inline auto make_layers(vk::DispatchLoaderDynamic& dispatch) -> std::vector<const char*>;
};
}  // namespace vulkan
}  // namespace luna
