#include "luna-gfx/vulkan/swapchain.hpp"
#include "luna-gfx/error/error.hpp"
#include "luna-gfx/interface/image.hpp"
#include "luna-gfx/vulkan/global_resources.hpp"
#include "luna-gfx/vulkan/utils/helper_functions.hpp"
#include <utility>
#include <algorithm>
namespace luna {
namespace vulkan {
Swapchain::Swapchain(Device& device, vk::SurfaceKHR surface, bool vsync) {
  auto fence_info = vk::FenceCreateInfo();
  fence_info.setFlags(vk::FenceCreateFlagBits::eSignaled);

  this->m_format = vk::Format::eB8G8R8A8Srgb;
  this->m_dependency = nullptr;
  this->m_current_frame = 0;
  this->m_skip_frame = false;
  this->m_surface = surface;
  this->m_device = &device;
  this->m_vsync = vsync;

  this->m_fences.clear();
  this->find_properties();
  this->choose_extent();
  this->make_swapchain();
  this->gen_images();

  this->m_fences.resize(this->m_images.size());
  this->m_image_available.resize(this->m_images.size());
  this->m_present_done.resize(this->m_images.size());
  this->m_fences_in_flight.resize(this->m_images.size());

  auto gpu = device.gpu;
  auto& dispatch = device.m_dispatch;
  auto alloc_cb = device.allocate_cb;
  for (auto& sem : this->m_image_available) {
    sem = error(gpu.createSemaphore({}, nullptr, dispatch));
  }

  for (auto& fence : this->m_fences)
    fence = error(gpu.createFence(fence_info, alloc_cb, dispatch));
  for (auto& fence : this->m_fences_in_flight) fence = nullptr;

  this->acquire();
}

Swapchain::Swapchain(Swapchain&& mv) { *this = std::move(mv); }

Swapchain::~Swapchain() {
  if (this->m_swapchain) {
    auto gpu = this->m_device->gpu;
    auto& dispatch = this->m_device->m_dispatch;
    auto alloc_cb = this->m_device->allocate_cb;

    for (auto& img : this->m_images)
      destroy_image(img);

    gpu.destroy(this->m_swapchain, alloc_cb, dispatch);
    this->m_swapchain = nullptr;

    for (auto& fence : this->m_fences) {
      error(gpu.waitForFences(1, &fence, true, UINT64_MAX, dispatch));
      gpu.destroy(fence, alloc_cb, dispatch);
    }
    for (auto& sem : this->m_image_available)
      gpu.destroy(sem, alloc_cb, dispatch);
    for (auto& sem : this->m_present_done) gpu.destroy(sem, alloc_cb, dispatch);

    

    this->m_images.clear();
    this->m_fences.clear();
    this->m_image_available.clear();
    this->m_present_done.clear();
  }
}

auto Swapchain::operator=(Swapchain&& mv) -> Swapchain& {
  this->m_fences = mv.m_fences;
  this->m_fences_in_flight = mv.m_fences_in_flight;
  this->m_formats = mv.m_formats;
  this->m_modes = mv.m_modes;
  this->m_images = mv.m_images;
  this->m_image_available = mv.m_image_available;
  this->m_present_done = mv.m_present_done;
  this->m_queue = mv.m_queue;
  this->m_device = mv.m_device;
  this->m_dependency = mv.m_dependency;
  this->m_swapchain = mv.m_swapchain;
  this->m_capabilities = mv.m_capabilities;
  this->m_surface_format = mv.m_surface_format;
  this->m_surface = mv.m_surface;
  this->m_extent = mv.m_extent;
  this->m_acquired = mv.m_acquired;
  this->m_current_frame = mv.m_current_frame;
  this->m_skip_frame = mv.m_skip_frame;
  this->m_vsync = mv.m_vsync;
  this->m_format = mv.m_format;

  mv.m_fences.clear();
  mv.m_fences_in_flight.clear();
  mv.m_formats.clear();
  mv.m_modes.clear();
  mv.m_images.clear();
  mv.m_image_available.clear();
  mv.m_present_done.clear();
  mv.m_current_frame = 0;
  mv.m_queue = nullptr;
  mv.m_device = nullptr;
  mv.m_dependency = nullptr;
  mv.m_swapchain = nullptr;
  mv.m_surface = nullptr;
  mv.m_skip_frame = false;
  mv.m_vsync = false;
  mv.m_format = {};

  while (!mv.m_acquired.empty()) mv.m_acquired.pop();
  return *this;
}

auto Swapchain::make_swapchain() -> void {
  auto info = vk::SwapchainCreateInfoKHR();

  auto usage = vk::ImageUsageFlagBits::eTransferDst |
               vk::ImageUsageFlagBits::eTransferSrc |
               vk::ImageUsageFlagBits::eColorAttachment;

  choose_format(vk::Format::eB8G8R8A8Srgb,
                vk::ColorSpaceKHR::eSrgbNonlinear);  // TODO make config

  auto present_mode = this->m_vsync ? vk::PresentModeKHR::eFifo
                                    : vk::PresentModeKHR::eImmediate;

  info.setSurface(this->m_surface);
  info.setMinImageCount(this->m_capabilities.minImageCount + 1);
  info.setImageFormat(this->m_surface_format.format);
  info.setImageColorSpace(this->m_surface_format.colorSpace);
  info.setImageExtent(this->m_extent);
  info.setImageArrayLayers(1);
  info.setImageUsage(usage);
  info.setPreTransform(this->m_capabilities.currentTransform);
  info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
  info.setPresentMode(this->select_mode(present_mode));

  info.setImageSharingMode(vk::SharingMode::eExclusive);
  info.setQueueFamilyIndexCount(0);
  info.setQueueFamilyIndices(nullptr);

  this->m_swapchain = error(this->m_device->gpu.createSwapchainKHR(info, this->m_device->allocate_cb, this->m_device->m_dispatch));
}

auto Swapchain::gen_images() -> void {
  auto info = gfx::ImageInfo();
  auto gpu = this->m_device->gpu;
  auto& dispatch = this->m_device->m_dispatch;

  info.width = this->m_extent.width;
  info.height = this->m_extent.height;
  info.format = convert(this->m_surface_format.format);
  info.num_mips = 1;
  info.layers = 1;
  auto images = error(gpu.getSwapchainImagesKHR(this->m_swapchain, dispatch));
  this->m_images.reserve(images.size());

  for(auto& img : images) {
    this->m_images.push_back(create_image(info, vk::ImageLayout::ePresentSrcKHR, vk::ImageUsageFlagBits::eColorAttachment, img));
    //TODO translate it to correct format;
  }
}

auto Swapchain::choose_format(vk::Format value, vk::ColorSpaceKHR color)
    -> void {
  for (const auto& format : this->m_formats) {
    if (format.format == value && format.colorSpace == color) {
      this->m_surface_format = format;
    }
  }
}

void Swapchain::find_properties() {
  const auto device = this->m_device->physical_device;
  auto& dispatch = this->m_device->m_dispatch;
  this->m_formats = error(
      device.getSurfaceFormatsKHR(this->m_surface, this->m_device->m_dispatch));
  this->m_capabilities =
      error(device.getSurfaceCapabilitiesKHR(this->m_surface, dispatch));
  this->m_modes = error(device.getSurfacePresentModesKHR(
      this->m_surface, this->m_device->m_dispatch));
}

void Swapchain::choose_extent() {
  if (this->m_capabilities.currentExtent.width != UINT32_MAX) {
    this->m_extent = this->m_capabilities.currentExtent;
  } else {
    this->m_extent.width =
        std::max(this->m_capabilities.minImageExtent.width,
                 std::min(this->m_capabilities.maxImageExtent.width,
                          this->m_extent.width));
    this->m_extent.height =
        std::max(this->m_capabilities.minImageExtent.height,
                 std::min(this->m_capabilities.maxImageExtent.height,
                          this->m_extent.height));
  }
}

vk::PresentModeKHR Swapchain::select_mode(vk::PresentModeKHR value) {
  for (auto& mode : this->m_modes) {
    if (mode == value) return mode;
  }

  return vk::PresentModeKHR::eFifo;
}

auto Swapchain::acquire() -> vk::Result {
  auto gpu = this->m_device->gpu;
  auto img = 0u;
  auto& dispatch = this->m_device->m_dispatch;
  auto& sem = this->m_image_available[this->m_current_frame];

  auto result = gpu.acquireNextImageKHR(this->m_swapchain, UINT64_MAX, sem,
                                        nullptr, &img, dispatch);
  if (result != vk::Result::eSuccess) {
    this->m_skip_frame = true;
    return result;
  }

  this->m_acquired.push(static_cast<unsigned>(img));
  this->m_skip_frame = false;
  return result;
}

auto Swapchain::present() -> bool {
//  if (!this->m_skip_frame) {
//    if(this->m_dependency) {
//      auto dep = this->m_dependency;
//      auto sem = this->m_image_available[this->m_current_frame];
//      this->m_fences_in_flight[this->m_current_frame] = dep->fence();
//      dep->submit();
//      if (!dep->present(sem, *this)) {
//        this->m_current_frame =
//            (this->m_current_frame + 1) % this->m_images.size();
//        return false;
//      }
//
//      this->m_current_frame =
//          (this->m_current_frame + 1) % this->m_images.size();
//      this->m_acquired.pop();
//      return this->acquire();
//    }
//  } else {
//    if (this->m_dependency) this->m_dependency->end();
//    this->m_current_frame = (this->m_current_frame + 1) % this->m_images.size();
//    this->m_acquired.pop();
//  }
//
//  return true;
return true;
}

void Swapchain::wait(CommandBuffer& cmd) {
  //cmd.add_signal(-1);
  //this->m_dependency = &cmd;
}
}  // namespace vulkan
}  // namespace luna
