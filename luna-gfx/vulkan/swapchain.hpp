#include "luna-gfx/vulkan/vulkan_defines.hpp"
#include "luna-gfx/vulkan/device.hpp"
#include <vulkan/vulkan.hpp>
#include <functional>
#include <limits.h>
#include <iostream>
#include <queue>
#include <vector>
#include <tuple>
#pragma once
namespace luna {
namespace vulkan {
struct CommandBuffer;
struct SwapchainCreateInfo {
  int32_t device;
  std::string name;
  vk::SurfaceKHR surface;
  std::function<void()> callback_func;
  bool vsync;
};

/** Class for managing a Vulkan Swapchain.
 */
class Swapchain {
 public:
  Swapchain() = default;
  Swapchain(SwapchainCreateInfo info);
  Swapchain(Swapchain&& mv);
  ~Swapchain();
  auto operator=(Swapchain&& mv) -> Swapchain&;
  auto wait(CommandBuffer& chain) -> void;
  auto present() -> void;
  auto format() {return this->m_surface_format.format;}
  auto width() {return this->m_extent.width;}
  auto height() {return this->m_extent.height;}
  auto image(size_t index) {return this->m_images[index];}
  auto swapchain() {return this->m_swapchain;}
  auto front() {return this->m_current_frame;}
  auto images() const -> const std::vector<int32_t>& {return this->m_images;}
  auto was_recreated() -> bool {return this->m_was_recreated;}
  auto recreate() -> void;
  
  /** Method to acquire the next image from the swapchain.
   */
  auto acquire() -> std::tuple<vk::Result, size_t>;
  std::function<void()> m_update_cb;
  std::vector<int32_t> m_sems_to_signal;
  std::vector<int32_t> m_sems_to_wait_on;
  int32_t m_gpu;
 private:
  using Formats = std::vector<vk::SurfaceFormatKHR>;
  using Modes = std::vector<vk::PresentModeKHR>;
  using Images = std::vector<int32_t>;
  using Fences = std::vector<vk::Fence>;
  using Semaphores = std::vector<vk::Semaphore>;


  Fences m_fences;
  Fences m_fences_in_flight;
  Formats m_formats;
  Modes m_modes;
  Images m_images;
  Semaphores m_image_available;
  Semaphores m_present_done;
  vk::Format m_format;
  vk::Queue m_queue;
  vk::SwapchainKHR m_swapchain;
  vk::SurfaceCapabilitiesKHR m_capabilities;
  vk::SurfaceFormatKHR m_surface_format;
  vk::SurfaceKHR m_surface;
  vk::Extent2D m_extent;
  std::string m_name;

  SwapchainCreateInfo m_info;
  unsigned m_current_frame;
  bool m_skip_frame;
  bool m_vsync;
  bool m_was_recreated = false;
  auto reset() -> void;

  auto check_support() -> void;

  /** Method to return the mode for this swapchain, if it's available.
   * @param value The requested mode.
   * @return The mode selected by what was available.
   */
  auto select_mode(vk::PresentModeKHR value) -> vk::PresentModeKHR;

  /** Helper method for finding a vulkan format from all formats supported.
   */
  auto choose_format(vk::Format value, vk::ColorSpaceKHR color) -> void;

  /** Helper method for generating the swapchain.
   */
  auto make_swapchain() -> void;

  /** Helper method to find the images of this swapchain.
   */
  auto gen_images() -> void;

  /** Helper method to find the swapchain properties from the device.
   */
  auto find_properties() -> void;

  /** Method to choose an extent from the surface capabilities.
   */
  auto choose_extent() -> void;
}; 
}
}