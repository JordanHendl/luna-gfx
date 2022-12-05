#include "luna-gfx/vulkan/vulkan_defines.hpp"
#include "luna-gfx/vulkan/device.hpp"
#include <vulkan/vulkan.hpp>
#include <limits.h>
#include <iostream>
#include <queue>
#include <vector>
#pragma once
namespace luna {
namespace vulkan {
class CommandBuffer;
/** Class for managing a Vulkan Swapchain.
 */
class Swapchain {
 public:
  Swapchain() = default;
  Swapchain(Device& device, vk::SurfaceKHR window, bool vsync);
  Swapchain(Swapchain&& mv);
  ~Swapchain();
  auto operator=(Swapchain&& mv) -> Swapchain&;
  auto wait(CommandBuffer& chain) -> void;
  auto present() -> bool;
  auto reset() -> void;
  auto format() {return this->m_surface_format.format;}
  auto width() {return this->m_extent.width;}
  auto height() {return this->m_extent.height;}
  auto image(size_t index) {return this->m_images[index];}
  auto wait_sem() -> vk::Semaphore {return this->m_image_available[this->m_current_frame];}
  auto swapchain() {return this->m_swapchain;}
  auto front() {return this->m_acquired.front();}
  auto advance() -> void {
    this->m_current_frame += 1;
    if(this->m_current_frame > this->m_image_available.size() - 1) {
      this->m_current_frame = 0;
    }
    
    this->m_acquired.pop();
  }

  /** Method to acquire the next image from the swapchain.
   */
  auto acquire() -> vk::Result;

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
  Device* m_device;
  CommandBuffer* m_dependency;
  vk::SwapchainKHR m_swapchain;
  vk::SurfaceCapabilitiesKHR m_capabilities;
  vk::SurfaceFormatKHR m_surface_format;
  vk::SurfaceKHR m_surface;
  vk::Extent2D m_extent;
  std::queue<unsigned> m_acquired;
  unsigned m_current_frame;
  bool m_skip_frame;
  bool m_vsync;

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