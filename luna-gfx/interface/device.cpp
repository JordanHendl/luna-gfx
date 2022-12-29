#include "luna-gfx/interface/device.hpp"
#include "luna-gfx/vulkan/utils/helper_functions.hpp"
#include "luna-gfx/vulkan/global_resources.hpp"
#include "luna-gfx/error/error.hpp"

namespace luna {
namespace gfx {
auto gpu_info() -> std::vector<GPUInfo> {
  auto& res = luna::vulkan::global_resources();
  auto vec = std::vector<GPUInfo>(res.devices.size());

  for(auto i = 0u; i < vec.size(); ++i) {
    vec[i].name = res.devices[i].properties.deviceName.data();
    vec[i].dedicated_card = res.devices[i].properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
  }

  return vec;
}

auto synchronize_gpu(int gpu) -> void {
  auto& res = luna::vulkan::global_resources();
  auto& device = res.devices[gpu];
  vulkan::error(device.gpu.waitIdle(device.m_dispatch));
}
}
}