#pragma once 
#include "luna-gfx/common/dlloader.hpp"
#include "luna-gfx/vulkan/vulkan_defines.hpp"
#include <vulkan/vulkan.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vector>
#include <iostream>
#include <string_view>
#include <memory>
#include <utility>

namespace luna {
namespace vulkan {
struct Instance {
  Instance() {}
  Instance(gfx::Dlloader& loader, vk::AllocationCallbacks* callback, std::string_view app_name);
  inline Instance(Instance&& mv) {*this = std::move(mv);};
  ~Instance();
  [[nodiscard]] inline auto valid() const -> bool {return this->m_instance;}
  auto operator=(Instance&& mv) -> Instance&;
  std::vector<vk::PhysicalDevice> m_devices;
  std::array<unsigned, 3> version;
  std::string app_name;
  vk::DispatchLoaderDynamic m_dispatch;
  vk::Instance m_instance;
  std::vector<std::string> extensions;
  std::vector<std::string> validation;

  private:
    inline vk::ApplicationInfo makeAppInfo();
    inline vk::DebugUtilsMessengerCreateInfoEXT makeDebugInfo();
    inline std::vector<const char*> makeExtensionList();
    inline std::vector<const char*> makeValidationList();
};
}
}