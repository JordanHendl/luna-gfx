#include "luna-gfx/vulkan/swapchain.hpp"
#include "luna-gfx/interface/image.hpp"
#include "luna-gfx/error/error.hpp"
#include "luna-gfx/vulkan/global_resources.hpp"
#include "luna-gfx/vulkan/data_types.hpp"
#include "luna-gfx/vulkan/utils/helper_functions.hpp"
#include <utility>
#include <algorithm>
namespace luna {
namespace vulkan {
Swapchain::Swapchain(int32_t device, vk::SurfaceKHR surface, bool vsync) {
  this->m_gpu = device;
  this->m_surface = surface;
  this->m_vsync = vsync;
  this->m_current_frame = 0;
  this->recreate();
  this->m_was_recreated = false; // Reset the flag set by ::recreate() because this is initialization.
}

Swapchain::Swapchain(Swapchain&& mv) { *this = std::move(mv); }

Swapchain::~Swapchain() {
  this->reset();
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
  this->m_gpu = mv.m_gpu;
  this->m_swapchain = mv.m_swapchain;
  this->m_capabilities = mv.m_capabilities;
  this->m_surface_format = mv.m_surface_format;
  this->m_surface = mv.m_surface;
  this->m_extent = mv.m_extent;
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
  mv.m_gpu = -1;
  mv.m_swapchain = nullptr;
  mv.m_surface = nullptr;
  mv.m_skip_frame = false;
  mv.m_vsync = false;
  mv.m_format = {};

  return *this;
}

auto Swapchain::reset() -> void {
  if (this->m_swapchain) {
    auto& gpu = global_resources().devices[this->m_gpu];
    auto& dispatch = gpu.m_dispatch;
    auto alloc_cb = gpu.allocate_cb;

    for (auto& img : this->m_images)
      destroy_image(img);

    gpu.gpu.destroy(this->m_swapchain, alloc_cb, dispatch);
    this->m_swapchain = nullptr;

    for (auto& fence : this->m_fences) {
      error(gpu.gpu.waitForFences(1, &fence, true, UINT64_MAX, dispatch));
      gpu.gpu.destroy(fence, alloc_cb, dispatch);
    }
    for (auto& sem : this->m_image_available)
      gpu.gpu.destroy(sem, alloc_cb, dispatch);
    for (auto& sem : this->m_present_done) gpu.gpu.destroy(sem, alloc_cb, dispatch);

    this->m_images.clear();
    this->m_fences.clear();
    this->m_image_available.clear();
    this->m_present_done.clear();
  }
}
auto Swapchain::make_swapchain() -> void {
  auto& gpu = global_resources().devices[this->m_gpu];
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

  this->m_swapchain = error(gpu.gpu.createSwapchainKHR(info, gpu.allocate_cb, gpu.m_dispatch));
}

auto Swapchain::gen_images() -> void {
  auto info = gfx::ImageInfo();
  auto& gpu = global_resources().devices[this->m_gpu];

  info.width = this->m_extent.width;
  info.height = this->m_extent.height;
  info.format = convert(this->m_surface_format.format);
  info.num_mips = 1;
  info.layers = 1;
  auto images = error(gpu.gpu.getSwapchainImagesKHR(this->m_swapchain, gpu.m_dispatch));
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
  auto& gpu = global_resources().devices[this->m_gpu];
  const auto device = gpu.physical_device;
  this->m_formats = error(
      device.getSurfaceFormatsKHR(this->m_surface, gpu.m_dispatch));
  this->m_capabilities =
      error(device.getSurfaceCapabilitiesKHR(this->m_surface, gpu.m_dispatch));
  this->m_modes = error(device.getSurfacePresentModesKHR(
      this->m_surface, gpu.m_dispatch));
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

auto Swapchain::acquire() -> std::tuple<vk::Result, size_t> {
  auto& gpu = global_resources().devices[this->m_gpu];
  auto img = 0u;
  auto sem = vk::Semaphore();

  if(!this->m_sems_to_signal.empty()) {
    auto vk_sems = vulkan::vk_sems_from_ids(this->m_gpu, this->m_sems_to_signal);
    sem = vk_sems[0];
  }

  auto result = gpu.gpu.acquireNextImageKHR(this->m_swapchain, UINT64_MAX, sem,
                                        nullptr, &img, gpu.m_dispatch);

  this->m_sems_to_signal.clear();
  this->m_current_frame = img;
  return {result, img};
}

auto Swapchain::recreate() -> void {
  this->reset();

  auto& gpu = global_resources().devices[this->m_gpu];
  auto fence_info = vk::FenceCreateInfo();
  fence_info.setFlags(vk::FenceCreateFlagBits::eSignaled);

  this->m_current_frame = 0;
  this->m_skip_frame = false;
  this->m_was_recreated = true;
  this->m_fences.clear();
  this->find_properties();
  this->choose_extent();
  this->make_swapchain();
  this->gen_images();

  this->m_fences.resize(this->m_images.size());
  this->m_image_available.resize(this->m_images.size());
  this->m_present_done.resize(this->m_images.size());
  this->m_fences_in_flight.resize(this->m_images.size());

  for (auto& fence : this->m_fences)
    fence = error(gpu.gpu.createFence(fence_info, gpu.allocate_cb, gpu.m_dispatch));
  for (auto& fence : this->m_fences_in_flight) fence = nullptr;
}

auto Swapchain::present() -> void {
  auto& res = vulkan::global_resources();
  auto& gpu = res.devices[this->m_gpu];

  // If this object has been recreated and it is now acquiring, we reset the flag because if anyone was to remake their assets they needed to have done so before now.
  if(m_was_recreated) this->m_was_recreated = false; 
  auto info = vk::PresentInfoKHR();
  auto queue = gpu.graphics().queue;
  auto indices = this->m_current_frame;
  auto chain = this->m_swapchain;
  auto wait_sems = vk_sems_from_ids(this->m_gpu, this->m_sems_to_wait_on);
  info.setPImageIndices(&indices);
  info.setSwapchainCount(1);
  info.setPSwapchains(&chain);
  info.setWaitSemaphores(wait_sems);
  auto result = queue.presentKHR(&info, gpu.m_dispatch);
  if(result != vk::Result::eSuccess) {
    gpu.wait_idle();
    this->recreate();
  }

  // Release ownership of the sems we just consumed.
  vulkan::release_semaphores(this->m_gpu, this->m_sems_to_wait_on);
  this->m_sems_to_wait_on.clear();
}
}  // namespace vulkan
}  // namespace luna
