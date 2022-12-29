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
#include "luna-gfx/interface/command_list.hpp"
#include "luna-gfx/vulkan/global_resources.hpp"
#include <chrono>
#include <array>
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

inline auto size_from_format(gfx::ImageFormat fmt) -> size_t {
  switch(fmt) {
    case gfx::ImageFormat::Depth : return sizeof(float);
    case gfx::ImageFormat::RGBA32F : return sizeof(float) * 4;
    case gfx::ImageFormat::RGBA8 : [[fallthrough]];
    case gfx::ImageFormat::BGRA8 : [[fallthrough]];
    default : return 4;
  }
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

inline auto begin_command_buffer(int32_t handle) -> void {
  LunaAssert(handle >= 0, "Attempting to use an invalid command buffer.");
  auto& cmd = luna::vulkan::global_resources().cmds[handle];
  auto& gpu = luna::vulkan::global_resources().devices[cmd.gpu];
  luna::vulkan::synchronize_cmd(handle);
  luna::vulkan::error(cmd.cmd.begin(cmd.begin_info, gpu.m_dispatch));
}

inline auto end_command_buffer(int32_t handle) -> void {
  LunaAssert(handle >= 0, "Attempting to use an invalid command buffer.");
  auto& cmd = luna::vulkan::global_resources().cmds[handle];
  auto& gpu = luna::vulkan::global_resources().devices[cmd.gpu];
  luna::vulkan::error(cmd.cmd.end(gpu.m_dispatch));
}

inline auto create_semaphores(int gpu_id, size_t amount) {
  auto& res = global_resources();
  auto& sems = res.semaphores[gpu_id];

  auto tmp = std::vector<int32_t>(amount);

  for(auto& sem : tmp) {
    sem = find_valid_entry(sems);
    sems[sem].in_use = true;
  }

  return tmp;
}

inline auto vk_sems_from_ids(int gpu_id, const std::vector<int32_t>& in_sems) -> std::vector<vk::Semaphore> {
  auto& res = global_resources();
  auto& sems = res.semaphores[gpu_id];

  auto ret = std::vector<vk::Semaphore>();
  ret.reserve(in_sems.size());
  for(auto sem : in_sems) {
    ret.push_back(sems[sem].sem);
  }

  return ret;
}

inline auto release_semaphores(int gpu_id, const std::vector<int32_t>& in_sems) -> std::vector<int32_t>{
  auto& res = global_resources();
  auto& sems = res.semaphores[gpu_id];

  for(auto sem : in_sems) {
    sems[sem].in_use = false;
  }
  return {};
}

inline auto create_cmd(int gpu, gfx::Queue type = gfx::Queue::All, CommandBuffer* parent = nullptr) -> int32_t {
  auto& res = global_resources();
  auto index = find_valid_entry(res.cmds);
  auto& cmd = res.cmds[index];
  auto& device = res.devices[gpu];
  auto info = vk::CommandBufferAllocateInfo();
  auto fence_info = vk::FenceCreateInfo();
  
  auto queue = device.graphics().queue;
  auto queue_id = device.graphics().id;
  switch(type) {
    case gfx::Queue::Compute : queue_id = device.compute().id; queue = device.compute().queue; break;
    case gfx::Queue::Transfer : queue_id = device.transfer().id; queue = device.transfer().queue; break;
    case gfx::Queue::All :  [[fallthrough]];
    case gfx::Queue::Graphics : [[fallthrough]];
    default: queue_id = device.graphics().id; queue = device.graphics().queue; break;
  }

  auto pool = create_pool(device, queue_id);
  info.setCommandBufferCount(1);
  info.setCommandPool(pool);
  info.setLevel(parent ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary);
  cmd.cmd = error(device.gpu.allocateCommandBuffers(info, device.m_dispatch)).data()[0];
  cmd.fence = error(device.gpu.createFence(fence_info, device.allocate_cb, device.m_dispatch));
  cmd.queue = queue;
  cmd.gpu = gpu;
  cmd.pool = pool;
  return index;
}

inline auto destroy_cmd(int32_t handle) -> void {
  auto& res = global_resources();
  auto& cmd = res.cmds[handle];
  auto& gpu = res.devices[cmd.gpu];
  synchronize_cmd(handle);
  if(cmd.timestamp_pool) {
    gpu.gpu.destroy(cmd.timestamp_pool, gpu.allocate_cb, gpu.m_dispatch);
    cmd.timestamp_pool = nullptr;
  } 
  release_semaphores(cmd.gpu, cmd.sems_to_wait_on);
  cmd.sems_to_signal.clear();
  cmd.sems_to_wait_on.clear();
  gpu.gpu.destroy(cmd.fence, gpu.allocate_cb, gpu.m_dispatch);
  gpu.gpu.freeCommandBuffers(cmd.pool, 1, &cmd.cmd, gpu.m_dispatch);
  cmd.cmd = nullptr;
}

inline auto cmd_start_render_pass(int32_t cmd_handle, int32_t rp_handle, size_t framebuffer_id) -> void {
  auto& res = global_resources();
  auto& cmd = res.cmds[cmd_handle];
  auto& gpu = res.devices[cmd.gpu];
  auto& rp = res.render_passes[rp_handle];

  const auto& curr_subpass = rp.current_subpass();
  auto info = vk::RenderPassBeginInfo();
  auto subpass = vk::SubpassContents::eInline;
  info.setRenderArea(rp.area());
  info.setRenderPass(rp.pass());
  info.setClearValues(curr_subpass.clear_values);
  info.setFramebuffer(rp.framebuffers()[framebuffer_id]);
  cmd.cmd.beginRenderPass(info, subpass, gpu.m_dispatch);
}

inline auto cmd_end_render_pass(int32_t cmd_handle) -> void {
  auto& res = global_resources();
  auto& cmd = res.cmds[cmd_handle];
  auto& gpu = res.devices[cmd.gpu];
  cmd.cmd.endRenderPass(gpu.m_dispatch);
}

inline auto cmd_bind_descriptor(int32_t cmd_handle, int32_t desc_handle) -> void {
  auto& res = global_resources();
  auto& cmd = res.cmds[cmd_handle];
  auto& gpu = res.devices[cmd.gpu];
  auto& desc = res.descriptors[desc_handle];

  auto& pipeline = desc.pipeline();
  auto vk_pipe = pipeline.pipeline();
  auto layout = pipeline.layout();
  const auto bind_point = pipeline.graphics() ? vk::PipelineBindPoint::eGraphics
                                              : vk::PipelineBindPoint::eCompute;

  cmd.cmd.bindPipeline(bind_point, vk_pipe, gpu.m_dispatch);
  if (desc.set())
    cmd.cmd.bindDescriptorSets(bind_point, layout, 0, 1, &desc.set(), 0, nullptr, gpu.m_dispatch);
}

inline auto cmd_buffer_draw(int32_t cmd_handle, int32_t vertices_id, size_t vertex_count, size_t instance_count) -> void {
  auto& res = global_resources();
  auto& cmd = res.cmds[cmd_handle];
  auto& gpu = res.devices[cmd.gpu];
  auto& vertices = res.buffers[vertices_id];

  auto offset = vk::DeviceSize(0);
  cmd.cmd.bindVertexBuffers(0, 1, &vertices.buffer, &offset, gpu.m_dispatch);
  cmd.cmd.draw(vertex_count, instance_count, 0, 0, gpu.m_dispatch);
}

inline auto cmd_buffer_draw(int32_t cmd_handle, int32_t vertices, size_t vert_count, int32_t indices,  size_t idx_count, size_t instance_count) -> void {
}

inline auto start_timestamp(int32_t cmd_handle, vk::PipelineStageFlagBits stage) -> void {
  auto& cmd = luna::vulkan::global_resources().cmds[cmd_handle];
  auto& gpu = luna::vulkan::global_resources().devices[cmd.gpu];
  constexpr auto cNumQueries = 2u;
  auto info = vk::QueryPoolCreateInfo();
  info.setQueryType(vk::QueryType::eTimestamp);
  info.setQueryCount(cNumQueries);
  if(!cmd.timestamp_pool) 
    cmd.timestamp_pool = error(gpu.gpu.createQueryPool(info, gpu.allocate_cb, gpu.m_dispatch));
  cmd.cmd.resetQueryPool(cmd.timestamp_pool, 0, cNumQueries, gpu.m_dispatch);
  cmd.cmd.writeTimestamp(stage, cmd.timestamp_pool, 0, gpu.m_dispatch);
}

inline auto end_timestamp(int32_t cmd_handle, vk::PipelineStageFlagBits stage) -> void {
  auto& cmd = luna::vulkan::global_resources().cmds[cmd_handle];
  auto& gpu = luna::vulkan::global_resources().devices[cmd.gpu];
  cmd.cmd.writeTimestamp(stage, cmd.timestamp_pool, 1, gpu.m_dispatch);
}

inline auto read_timestamp(int32_t cmd_handle) -> std::chrono::duration<double, std::nano> {
  static_assert(sizeof(double) == 8);
  constexpr auto flags = vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait;
  auto& cmd = luna::vulkan::global_resources().cmds[cmd_handle];
  auto& gpu = luna::vulkan::global_resources().devices[cmd.gpu];
  auto tmp_storage = std::array<int64_t, 2> {0, 0};
  error(gpu.gpu.getQueryPoolResults(cmd.timestamp_pool, 0, 2, sizeof(double) * 2, &tmp_storage[0], sizeof(double), flags, gpu.m_dispatch));

  auto duration = static_cast<double>(tmp_storage[1] - tmp_storage[0]) * gpu.properties.limits.timestampPeriod;
  return std::chrono::duration<double, std::nano>(duration);
}

inline auto submit_command_buffer(int32_t handle) -> void {
  auto& cmd = luna::vulkan::global_resources().cmds[handle];
  auto& gpu = luna::vulkan::global_resources().devices[cmd.gpu];
  auto& info = cmd.submit_info;
  auto& queue = cmd.queue;

  auto vector = std::vector<vk::Semaphore>();
  auto masks = std::vector<vk::PipelineStageFlags>();
  
  if(cmd.fence && cmd.signaled) {
    error(gpu.gpu.waitForFences(1, &cmd.fence, true, UINT64_MAX, gpu.m_dispatch));
    error(gpu.gpu.resetFences(1, &cmd.fence, gpu.m_dispatch));
  }

  auto wait_sems = vk_sems_from_ids(cmd.gpu, cmd.sems_to_wait_on);
  auto signal_sems = vk_sems_from_ids(cmd.gpu, cmd.sems_to_signal);

  masks.resize(wait_sems.size());
  for(auto& mask : masks) mask = vk::PipelineStageFlagBits::eTopOfPipe;

  info.setCommandBufferCount(1);
  info.setPCommandBuffers(&cmd.cmd);
  info.setWaitSemaphores(wait_sems);
  info.setSignalSemaphores(signal_sems);
  info.setWaitDstStageMask(masks);

  if(wait_sems.size() > 0)
    LunaAssert(info.waitSemaphoreCount > 0, "WTF");
  error(queue.submit(1, &info, cmd.fence, gpu.m_dispatch));

  // Release the sems that were just consumed.
  release_semaphores(cmd.gpu, cmd.sems_to_wait_on); 
  cmd.sems_to_wait_on.clear();
  cmd.sems_to_signal.clear();
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
  src = vk::PipelineStageFlagBits::eBottomOfPipe;
  dst = vk::PipelineStageFlagBits::eTopOfPipe;

  LunaAssert(new_layout != vk::ImageLayout::eUndefined, "Attempting to transition an image to an undefined layout, which is not possible");
  cmd.cmd.pipelineBarrier(src, dst, dep_flags, 0, nullptr, 0, nullptr, 1, &image.barrier, gpu.m_dispatch);
  image.layout = new_layout;
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

inline auto usage_from_format(gfx::ImageFormat format) {
  using Usage = vk::ImageUsageFlagBits;
  switch(format) {
    case gfx::ImageFormat::Depth: return Usage::eDepthStencilAttachment | Usage::eTransferSrc | Usage::eTransferDst;
    default : return standard_image_usage();
  }
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

  img.sampler = luna::vulkan::error(device.gpu.createSampler(info, device.allocate_cb, device.m_dispatch));
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
  
  // Transition image to general format.
  auto cmd = luna::vulkan::create_cmd(in_info.gpu);
  luna::vulkan::begin_command_buffer(cmd);
  luna::vulkan::transition_image(cmd, index, vk::ImageLayout::eGeneral);
  luna::vulkan::end_command_buffer(cmd);
  luna::vulkan::submit_command_buffer(cmd);
  luna::vulkan::synchronize_cmd(cmd);
  luna::vulkan::destroy_cmd(cmd);
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
  auto& res = global_resources();
  auto& pipeline = res.pipelines[pipe_handle];
  return pipeline.descriptor();
} 
}
}