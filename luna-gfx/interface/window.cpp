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
  auto img_info = gfx::ImageInfo();

  this->m_handle = vulkan::find_valid_entry(res.windows);
  auto& swap = res.swapchains[this->m_handle];

  res.windows[this->m_handle] = std::move(vulkan::Window(info));
  swap = std::move(vulkan::Swapchain(info.gpu, res.windows[this->m_handle].surface(), false));
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

auto Window::acquire() -> std::size_t {
  auto& res = vulkan::global_resources();
  auto& swap = res.swapchains[this->m_handle];

  auto [result, img] = swap.acquire();
  // If result was bad, keep recreating until its better!
  while(result == vk::Result::eErrorSurfaceLostKHR || result == vk::Result::eSuboptimalKHR) {
    swap.recreate();
    auto [res2, img2] = swap.acquire();
    result = res2;
    img = img2;
  }

  return img;
}

auto Window::present() -> void {
  auto& res = vulkan::global_resources();
  auto& swap = res.swapchains[this->m_handle];
  swap.present();
}

auto Window::current_frame() -> std::size_t {
  auto& res = vulkan::global_resources();
  auto& swap = res.swapchains[this->m_handle];
  return swap.front();
}

auto Window::combo_into(CommandList& in_cmd) -> void {
  auto& res = vulkan::global_resources();
  auto& swap = res.swapchains[this->m_handle];
  auto& cmd = res.cmds[in_cmd.handle()];

  // Grab sem ownership.
  auto sem = vulkan::create_semaphores(swap.m_gpu, 1);

  // Set it to us to signal.
  swap.m_sems_to_signal.push_back(sem[0]);

  // Set it to the input cmd to wait on (and eventually release).
  cmd.sems_to_wait_on.push_back(sem[0]);
}

auto Window::image_views() -> std::vector<ImageView> {
  auto& res = vulkan::global_resources();
  auto& swap = res.swapchains[this->m_handle];
  auto images = swap.images();

  
  auto ret = std::vector<ImageView>();
  ret.reserve(images.size());
  for(auto& swap_img : images) {
    ret.push_back(swap_img);
  }

  return ret;
}
}
}