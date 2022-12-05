#pragma once
#include "luna-gfx/vulkan/vulkan_defines.hpp"
#include "luna-gfx/vulkan/data_types.hpp"
#include "luna-gfx/vulkan/render_pass.hpp"
#include "luna-gfx/vulkan/pipeline.hpp"
#include "luna-gfx/error/error.hpp"
#include "luna-gfx/vulkan/device.hpp"
#include "luna-gfx/interface/image.hpp"
#include "luna-gfx/interface/render_pass.hpp"
#include "luna-gfx/interface/pipeline.hpp"
#include "luna-gfx/vulkan/global_resources.hpp"

namespace luna {
namespace vulkan {
inline auto convert(luna::gfx::ImageFormat fmt) -> vk::Format {
  switch(fmt) {
    case gfx::ImageFormat::RGBA8: return vk::Format::eR8G8B8A8Srgb;
    case gfx::ImageFormat::BGRA8: return vk::Format::eB8G8R8A8Srgb;
    case gfx::ImageFormat::RGBA32F: return vk::Format::eR32G32B32A32Sfloat;
    case gfx::ImageFormat::Depth: return vk::Format::eD24UnormS8Uint;
    default: return vk::Format::eR8G8B8A8Srgb;
  }
}

inline auto convert(vk::Format fmt) -> gfx::ImageFormat {
  switch(fmt) {
    case vk::Format::eR8G8B8A8Srgb: return gfx::ImageFormat::RGBA8;
    case vk::Format::eB8G8R8A8Srgb: return gfx::ImageFormat::BGRA8;
    case vk::Format::eR32G32B32A32Sfloat: return gfx::ImageFormat::RGBA32F;
    case vk::Format::eD24UnormS8Uint: return gfx::ImageFormat::Depth;
    default: return gfx::ImageFormat::RGBA8;
  }
}

inline auto begin_command_buffer(int32_t handle) -> void {
  LunaAssert(handle >= 0, "Attempting to use an invalid command buffer.");
  auto& cmd = luna::vulkan::global_resources().cmds[handle];
  auto& gpu = luna::vulkan::global_resources().devices[cmd.gpu];
  luna::vulkan::error(cmd.cmd.begin(cmd.begin_info, gpu.m_dispatch));
}

inline auto end_command_buffer(int32_t handle) -> void {
  LunaAssert(handle >= 0, "Attempting to use an invalid command buffer.");
  auto& cmd = luna::vulkan::global_resources().cmds[handle];
  auto& gpu = luna::vulkan::global_resources().devices[cmd.gpu];
  luna::vulkan::error(cmd.cmd.end(gpu.m_dispatch));
}

inline auto create_cmd(int gpu, CommandBuffer* parent = nullptr) -> int32_t {
  auto& res = global_resources();
  auto index = find_valid_entry(res.cmds);
  auto& cmd = res.cmds[index];
  auto& device = res.devices[gpu];
  auto pool = create_pool(device, device.graphics().id);
  auto info = vk::CommandBufferAllocateInfo();
  auto fence_info = vk::FenceCreateInfo();
  info.setCommandBufferCount(1);
  info.setCommandPool(pool);
  info.setLevel(parent ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary);
  cmd.cmd = error(device.gpu.allocateCommandBuffers(info, device.m_dispatch)).data()[0];
  cmd.fence = error(device.gpu.createFence(fence_info, device.allocate_cb, device.m_dispatch));

  cmd.signal_sems.resize(1);
  cmd.signal_sems[0] = error(device.gpu.createSemaphore({}, device.allocate_cb, device.m_dispatch));
  cmd.gpu = gpu;
  return index;
}

inline auto destroy_cmd(int32_t handle) {
  auto& res = global_resources();
  auto& cmd = res.cmds[handle];
  cmd.cmd = nullptr;
}

inline auto submit_command_buffer(int32_t handle) -> void {
  auto& cmd = luna::vulkan::global_resources().cmds[handle];
  auto& gpu = luna::vulkan::global_resources().devices[cmd.gpu];
  auto& info = cmd.submit_info;
  auto& queue = gpu.graphics();

  auto vector = std::vector<vk::Semaphore>();
  auto signal_sems = std::vector<vk::Semaphore>();
  auto masks = std::vector<vk::PipelineStageFlags>();
  
  if(cmd.fence && cmd.signaled) {
    error(gpu.gpu.waitForFences(1, &cmd.fence, true, UINT64_MAX, gpu.m_dispatch));
    error(gpu.gpu.resetFences(1, &cmd.fence, gpu.m_dispatch));
  }

  info.setCommandBufferCount(1);
  info.setPCommandBuffers(&cmd.cmd);
  info.setWaitSemaphores(cmd.wait_sems);
  info.setSignalSemaphores(cmd.signal_sems);
  info.setWaitDstStageMask(masks);

  error(queue.queue.submit(1, &info, cmd.fence, gpu.m_dispatch));
  cmd.signaled = true;
}

inline auto transition_image(int32_t cmd_id, int32_t image_id, vk::ImageLayout layout) -> void {
  auto& res = global_resources();
  auto& image = res.images[image_id];
  auto& cmd = res.cmds[cmd_id];
  auto& gpu = res.devices[image.info.gpu];

  auto range = vk::ImageSubresourceRange();
  auto src = vk::PipelineStageFlags();
  auto dst = vk::PipelineStageFlags();
  auto dep_flags = vk::DependencyFlags();
  auto new_layout = vk::ImageLayout();
  auto old_layout = vk::ImageLayout();

  new_layout = layout;
  old_layout = image.layout;

  range.setBaseArrayLayer(0);
  range.setBaseMipLevel(0);
  range.setLevelCount(1);
  range.setLayerCount(image.info.layers);
  if (image.format == vk::Format::eD24UnormS8Uint)
    range.setAspectMask(vk::ImageAspectFlagBits::eDepth |
                        vk::ImageAspectFlagBits::eStencil);
  else
    range.setAspectMask(vk::ImageAspectFlagBits::eColor);

  image.barrier.setOldLayout(old_layout);
  image.barrier.setNewLayout(new_layout);
  image.barrier.setImage(image.image);
  image.barrier.setSubresourceRange(range);
  image.barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
  image.barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);

  dep_flags = vk::DependencyFlags();
  src = vk::PipelineStageFlagBits::eTopOfPipe;
  dst = vk::PipelineStageFlagBits::eBottomOfPipe;

  LunaAssert(new_layout != vk::ImageLayout::eUndefined, "Attempting to transition an image to an undefined layout, which is not possible");
  cmd.cmd.pipelineBarrier(src, dst, dep_flags, 0, nullptr, 0, nullptr, 1, &image.barrier, gpu.m_dispatch);
  image.layout = new_layout;
}

inline auto present_swapchain(int32_t /*cmd_id*/, int32_t /*swap_id*/) -> void {
//  auto& res = global_resources();
//  auto& cmd = res.cmds[cmd_id];
//  auto& swap = res.swapchains[swap_id];
//  auto& gpu = res.devices[cmd.gpu];
//
//  auto info = vk::PresentInfoKHR();
//  auto queue = gpu.graphics().queue;
//  auto indices = swap.front();
//  auto chain = swap.swapchain();
//  auto wait_sems = std::vector<vk::Semaphore>();
//
//  wait_sems.push_back(cmd.signal_sems[0]);
//  wait_sems.push_back(swap.wait_sem());
//  info.setPImageIndices(&indices);
//  info.setSwapchainCount(1);
//  info.setPSwapchains(&chain);
//  info.setWaitSemaphores(wait_sems);
//  
//  swap.advance();
//
//  // Don't assert here because we may just need to handle window resizes.
//  // @JH TODO: This technically should assert on all errors that aren't window
//  // resizes.
//
//  auto recreate_swap = [&] () {
//    error(gpu.gpu.waitIdle(gpu.m_dispatch));
//    { auto tmp = std::move(swap); }
//    swap = std::move(luna::vulkan::Swapchain(gpu, res.windows[swap_id].surface(), false));
//  };
//
//  auto result = queue.presentKHR(&info, gpu.m_dispatch);
//  if(result != vk::Result::eSuccess) {
//    recreate_swap();
//  } else {
//     result = swap.acquire();
//     if(result != vk::Result::eSuccess) {
//       recreate_swap();
//     }
//  }
}

inline auto copy_buffer_to_buffer(int32_t cmd_id, int32_t from, int32_t to, std::size_t amt = 0) -> void {
  auto& res = global_resources();
  auto& src = res.buffers[from];
  auto& dst = res.buffers[to];
  auto& cmd = res.cmds[cmd_id];
  auto& gpu = res.devices[src.gpu];

  auto region = vk::BufferCopy();

  auto size = amt == 0 ? dst.info.size : amt;
  region.setSize(size);
  region.setSrcOffset(0);
  region.setDstOffset(0);

  cmd.cmd.copyBuffer(src.buffer, dst.buffer, 1, &region, gpu.m_dispatch);
}

inline auto copy_buffer_to_image(int32_t cmd_id, int32_t buffer_id, int32_t image_id) -> void {
  auto& res = global_resources();
  auto& src = res.buffers[buffer_id];
  auto& dst = res.images[image_id];
  auto& cmd = res.cmds[cmd_id];
  auto& gpu = res.devices[src.gpu];

  auto info = vk::BufferImageCopy();
  auto extent = vk::Extent3D();

  extent.setWidth(dst.info.width);
  extent.setHeight(dst.info.height);
  extent.setDepth(1);

  info.setImageExtent(extent);
  info.setBufferImageHeight(0);
  info.setBufferRowLength(0);
  info.setImageOffset(0);
  info.setImageSubresource(dst.subresource);

  auto dst_old_layout = dst.layout;

  // Need to handle layout transitions because we're copying
  if (dst.layout != vk::ImageLayout::eTransferDstOptimal)
    transition_image(cmd_id, image_id, vk::ImageLayout::eTransferDstOptimal);

  cmd.cmd.copyBufferToImage(src.buffer, dst.image, vk::ImageLayout::eTransferDstOptimal, 1, &info, gpu.m_dispatch);

  if (dst_old_layout != vk::ImageLayout::eUndefined)
    transition_image(cmd_id, image_id, dst_old_layout);
}

inline auto synchronize_cmd(int32_t cmd_id) -> void {
  auto& cmd = luna::vulkan::global_resources().cmds[cmd_id];
  auto& gpu = luna::vulkan::global_resources().devices[cmd.gpu];
  
  if(cmd.fence && cmd.signaled) {
    error(gpu.gpu.waitForFences(1, &cmd.fence, true, UINT64_MAX, gpu.m_dispatch));
    error(gpu.gpu.resetFences(1, &cmd.fence, gpu.m_dispatch));
    cmd.signaled = false;
  }
}

inline auto map_buffer(int32_t buffer_id, void** ptr) -> void {
  auto& res = global_resources();
  auto& buffer = res.buffers[buffer_id];
  auto alloc = res.allocators[buffer.gpu];
  vmaMapMemory(alloc, buffer.alloc, ptr);
}

inline auto unmap_buffer(int32_t buffer_id) -> void {
  auto& res = global_resources();
  auto& buffer = res.buffers[buffer_id];
  auto alloc = res.allocators[buffer.gpu];
  vmaUnmapMemory(alloc, buffer.alloc);
}

inline auto create_buffer(int gpu, std::size_t size, vk::BufferUsageFlags usage, bool mappable) -> std::int32_t {
  auto& res  = vulkan::global_resources();
  auto info = vk::BufferCreateInfo();
  auto alloc_info = VmaAllocationCreateInfo{};
  auto index = luna::vulkan::find_valid_entry(res.buffers);
  auto& buffer = res.buffers[index];
  
  if(mappable) {
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
  } else {
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
  }

  info.size = size;
  info.usage = usage;
  buffer.gpu = gpu;
  auto& c_info = static_cast<VkBufferCreateInfo&>(info);
  auto c_buffer = static_cast<VkBuffer>(buffer.buffer);
  vmaCreateBuffer(res.allocators[gpu], &c_info, &alloc_info, &c_buffer, &buffer.alloc, nullptr);
  vmaGetAllocationInfo(res.allocators[gpu], buffer.alloc, &buffer.info);
  buffer.buffer = c_buffer;
  buffer.size = size;
  LunaAssert(c_buffer, "Could not create buffer given input parameters.");
  return index;
}

inline auto destroy_buffer(std::int32_t handle) -> void {
  auto& res  = luna::vulkan::global_resources();
  auto& buffer = res.buffers[handle];
  auto c_buffer = static_cast<VkBuffer>(buffer.buffer);
  
  vmaDestroyBuffer(res.allocators[buffer.gpu], c_buffer, buffer.alloc);
  buffer.info = {};
  buffer.buffer = nullptr;
  buffer.size = 0;
  buffer.alloc = nullptr;
}
inline auto standard_image_usage() {
  return vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc |
  vk::ImageUsageFlagBits::eSampled;
}
inline auto create_sampler(luna::vulkan::Device& device, luna::vulkan::Image& img) {
  const auto max_anisotropy = 16.0f;
  auto info = vk::SamplerCreateInfo();
  info.setMagFilter(vk::Filter::eNearest);
  info.setMinFilter(vk::Filter::eNearest);
  info.setAddressModeU(vk::SamplerAddressMode::eRepeat);
  info.setAddressModeV(vk::SamplerAddressMode::eRepeat);
  info.setAddressModeW(vk::SamplerAddressMode::eRepeat);
  info.setBorderColor(vk::BorderColor::eIntTransparentBlack);
  info.setCompareOp(vk::CompareOp::eNever);
  info.setMipmapMode(vk::SamplerMipmapMode::eLinear);
  info.setAnisotropyEnable(vk::Bool32(false));
  info.setUnnormalizedCoordinates(vk::Bool32(false));
  info.setCompareEnable(vk::Bool32(false));
  info.setMaxAnisotropy(max_anisotropy);
  info.setMipLodBias(0.0f);
  info.setMinLod(0.0f);
  info.setMaxLod(0.0f);

  img.sampler = luna::vulkan::error(device.gpu.createSampler(info, device.allocation_cb(), device.m_dispatch));
}

inline auto create_image_view(luna::vulkan::Device& device, luna::vulkan::Image& img) -> void {
  auto info = vk::ImageViewCreateInfo();
  auto range = vk::ImageSubresourceRange();

  if (img.info.format == luna::gfx::ImageFormat::Depth) {
    range.setAspectMask(vk::ImageAspectFlagBits::eDepth |
                        vk::ImageAspectFlagBits::eStencil);
  } else {
    range.setAspectMask(vk::ImageAspectFlagBits::eColor);
  }

  range.setBaseArrayLayer(0);
  range.setBaseMipLevel(0);
  range.setLayerCount(img.info.layers);
  range.setLevelCount(1);

  info.setImage(img.image);
  info.setViewType(img.view_type);  //@JH TODO Make configurable.
  info.setFormat(img.format);
  info.setSubresourceRange(range);

  img.view = luna::vulkan::error(device.gpu.createImageView(info, device.allocate_cb, device.m_dispatch));
}

//inline auto upload_data_to_image(int32_t image_id, gfx::ImageInfo& info, const unsigned char* initial_data) -> void {
//    auto& res = global_resources();
//    auto num_channels = 4;
//    auto element_size = sizeof(unsigned char);
//    switch(info.format) {
//      case gfx::ImageFormat::RGBA32F:
//        element_size = sizeof(float);
//      case gfx::ImageFormat::RGBA8:
//      case gfx::ImageFormat::BGRA8:
//      default: break;
//    }
//
//    auto size = info.width * info.height * num_channels * element_size;
//    auto tmp_buffer = make_mappable_buffer(info.gpu, size);
//    auto tmp_cmd = create_cmd(info.gpu, nullptr);
//    auto cmd = res.cmds[tmp_cmd];
//
//    void* ptr = nullptr;
//    map_buffer(tmp_buffer, &ptr);
//    std::memcpy(ptr, initial_data, size);
//    unmap_buffer(tmp_buffer);
//
//    begin_command_buffer(tmp_cmd);
//    copy_buffer_to_image(tmp_cmd, tmp_buffer, image_id);
//    end_command_buffer(tmp_cmd);
//    submit_command_buffer(tmp_cmd);
//    synchronize(tmp_cmd);
//}


inline auto create_image(gfx::ImageInfo& in_info, vk::ImageLayout layout, vk::ImageUsageFlags usage, vk::Image import) -> int32_t {
  auto& res = luna::vulkan::global_resources();
  auto index = luna::vulkan::find_valid_entry(res.images);
  auto& gpu = res.devices[in_info.gpu];
  auto& image = res.images[index];
  image.info = in_info;
  image.image = import;
  image.imported = true;
  image.layout = layout;
  image.format = convert(in_info.format);
  image.usage = usage;

  if (in_info.is_cubemap) {
    image.view_type = vk::ImageViewType::eCube;
  } else {
    switch (in_info.layers) {
      case 0:
        image.view_type = vk::ImageViewType::e1D;
        break;
      case 1:
        image.view_type = vk::ImageViewType::e2D;
        break;
      default:
        image.view_type = vk::ImageViewType::e2DArray;
        break;
    }
  }

  image.subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
  image.subresource.setBaseArrayLayer(0);
  image.subresource.setLayerCount(in_info.layers);
  image.subresource.setMipLevel(0);

  image.layout = layout;
  image.usage = usage;
  image.alloc = nullptr;
  create_sampler(gpu, image);
  create_image_view(gpu, image);
  return index;
}

inline auto create_image(gfx::ImageInfo& in_info, vk::ImageLayout layout, vk::ImageUsageFlags usage, const unsigned char* initial_data) -> int32_t {
  auto& res = luna::vulkan::global_resources();
  auto& allocator = res.allocators[in_info.gpu];
  auto& gpu = res.devices[in_info.gpu];
  auto info = vk::ImageCreateInfo();
  auto alloc_info = VmaAllocationCreateInfo{};
  auto index = luna::vulkan::find_valid_entry(res.images);
  auto& image = res.images[index];
  
  
  if (in_info.is_cubemap) {
    image.view_type = vk::ImageViewType::eCube;
    info.imageType = vk::ImageType::e2D;
  } else {
    switch (in_info.layers) {
      case 0:
        image.view_type = vk::ImageViewType::e1D;
        info.imageType = vk::ImageType::e1D;
        break;
      case 1:
        image.view_type = vk::ImageViewType::e2D;
        info.imageType = vk::ImageType::e2D;
        break;
      default:
        image.view_type = vk::ImageViewType::e2DArray;
        info.imageType = vk::ImageType::e2D;
        break;
    }
  }

  info.extent = vk::Extent3D{static_cast<unsigned>(in_info.width), static_cast<unsigned>(in_info.height), 1};
  info.arrayLayers = in_info.layers;
  info.format = luna::vulkan::convert(in_info.format);
  info.initialLayout = vk::ImageLayout::eUndefined;
  info.mipLevels = in_info.num_mips;
  info.usage = usage;
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO;

  image.subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
  image.subresource.setBaseArrayLayer(0);
  image.subresource.setLayerCount(in_info.layers);
  image.subresource.setMipLevel(0);

  auto& c_info = static_cast<VkImageCreateInfo&>(info); 
  auto c_image = static_cast<VkImage>(image.image);
  vmaCreateImage(allocator, &c_info, &alloc_info, &c_image, &image.alloc, nullptr);
  LunaAssert(c_image, "Could not create image given input parameters.");
  image.info = in_info;
  image.image = c_image;
  image.layout = layout;
  image.format = info.format;
  image.usage = usage;
  create_sampler(gpu, image);
  create_image_view(gpu, image);
  
  if(initial_data != nullptr) {
    LunaAssert(false, "Input parameters to images not yet supported.");
    //upload_data_to_image(index, in_info, initial_data);
  }
  return index;
}

inline auto destroy_image(int32_t handle) -> void {
  auto& res  = luna::vulkan::global_resources();
  auto& img = res.images[handle];
  auto& gpu = res.devices[img.info.gpu];
  if(!img.valid()) return;
  if(img.imported) {
    img.imported = false;
    gpu.gpu.destroy(img.view, gpu.allocate_cb, gpu.m_dispatch);
    gpu.gpu.destroy(img.sampler, gpu.allocate_cb, gpu.m_dispatch);
  } else {
    auto c_img = static_cast<VkImage>(img.image);
    gpu.gpu.destroy(img.view, gpu.allocate_cb, gpu.m_dispatch);
    gpu.gpu.destroy(img.sampler, gpu.allocate_cb, gpu.m_dispatch);
    vmaDestroyImage(res.allocators[img.info.gpu], c_img, img.alloc);
    img.alloc = nullptr;
  }
  img.view = nullptr;
  img.sampler = nullptr;
  img.image = nullptr;
}

inline auto create_graphics_pipeline(int32_t rp_handle, gfx::GraphicsPipelineInfo info) -> int32_t {
  auto& res = global_resources();
  auto& rp = res.render_passes[rp_handle];
  auto index = find_valid_entry(res.pipelines);
  auto& pipe = res.pipelines[index];
  
  pipe = std::move(Pipeline(rp, info));
  return index;
}

inline auto destroy_pipeline(int32_t handle) -> void {
  auto tmp = std::move(global_resources().pipelines[handle]);
}

inline auto create_bind_group(int32_t pipe_handle) -> int32_t {
  return -1;
}

inline auto update_view_port(int32_t pipeline_handle, gfx::Viewport viewport) -> void {

}
}
}