#include "luna-gfx/interface/window.hpp"
#include "luna-gfx/vulkan/window.hpp"
#include "luna-gfx/vulkan/swapchain.hpp"
#include "luna-gfx/vulkan/utils/helper_functions.hpp"
#include "luna-gfx/vulkan/global_resources.hpp"
#include "luna-gfx/error/error.hpp"
#include <utility>

namespace luna {
namespace gfx {
Window::Window(WindowInfo info) {
  auto& res = vulkan::global_resources();
  auto& gpu = res.devices[info.gpu];
  this->m_handle = vulkan::find_valid_entry(res.windows);
  res.windows[this->m_handle] = std::move(vulkan::Window(info));
  res.swapchains[this->m_handle] = std::move(vulkan::Swapchain(gpu, res.windows[this->m_handle].surface(), false));

  this->m_info = info;
}

Window::~Window() {
  if(this->m_handle < 0) return;

  auto& res = vulkan::global_resources();
  {
    auto tmp = std::move(res.swapchains[this->m_handle]);
  }

  {
    auto tmp = std::move(res.windows[this->m_handle]);
  }
}

auto Window::attachment() -> gfx::Attachment {
  LunaAssert(this->m_handle >= 0, "Asking for an attachment of a invalid window.");
  auto& res = vulkan::global_resources();
  auto& swap = res.swapchains[this->m_handle];
  auto tmp = gfx::Attachment();

  tmp.info.name = this->info().title;
  tmp.info.gpu = this->m_info.gpu;
  tmp.info.format = vulkan::convert(swap.format());
  tmp.info.width = swap.width();
  tmp.info.height = swap.height();
  return tmp;
}
}
}