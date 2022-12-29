#include "luna-gfx/vulkan/render_pass.hpp"
#include "luna-gfx/vulkan/vulkan_defines.hpp"
#include "luna-gfx/vulkan/device.hpp"
#include "luna-gfx/error/error.hpp"
#include "luna-gfx/vulkan/swapchain.hpp"
#include "luna-gfx/vulkan/utils/helper_functions.hpp"
#include "luna-gfx/vulkan/global_resources.hpp"
#include <vulkan/vulkan.hpp>
#include <array>
#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <memory>

namespace luna {
namespace vulkan {
inline auto convert(const gfx::Attachment& attachment)
    -> vk::AttachmentDescription {
  using StoreOps = vk::AttachmentStoreOp;
  using LoadOps = vk::AttachmentLoadOp;

  auto is_depth = attachment.views[0].format() == gfx::ImageFormat::Depth;
  auto format = convert(attachment.views[0].format());
  auto layout = vk::ImageLayout::eColorAttachmentOptimal;
  auto stencil_store = is_depth ? StoreOps::eStore : StoreOps::eDontCare;
  auto stencil_load = is_depth ? LoadOps::eLoad : LoadOps::eDontCare;
  auto load_op = LoadOps::eClear;  /// TODO make configurable
  auto store_op = StoreOps::eStore;

  if(is_depth) layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

  auto desc = vk::AttachmentDescription();
  desc.setSamples(vk::SampleCountFlagBits::e1);
  desc.setLoadOp(load_op);
  desc.setStoreOp(store_op);
  desc.setFormat(format);
  desc.setInitialLayout(vk::ImageLayout::eUndefined);
  desc.setStencilLoadOp(stencil_load);
  desc.setStencilStoreOp(stencil_store);
  desc.setFinalLayout(layout);
  return desc;
}

RenderPass::RenderPass() {
  this->m_current_framebuffer = 0;
  this->m_num_binded_subpasses = 0;
  this->m_area.extent.setWidth(1280);
  this->m_area.extent.setHeight(1024);
}

RenderPass::RenderPass(Device& device, const gfx::RenderPassInfo& info, Swapchain* swap) {
  this->m_device = &device;
  this->m_area.extent.width = info.width;
  this->m_area.extent.height = info.height;
  this->m_current_framebuffer = 0;
  this->m_swap = swap;
  this->parse_info(info);
  this->make_render_pass();
  this->make_images();
  this->bind();
}

RenderPass::RenderPass(RenderPass&& mv) { *this = std::move(mv); }

RenderPass::~RenderPass() {
  for (auto& framebuffer : this->m_framebuffers) {
    this->m_device->gpu.destroy(framebuffer,
                                     this->m_device->allocate_cb,
                                     this->m_device->m_dispatch);
  }

  for(auto& subpass : this->m_subpasses) {
    subpass.luna_attachments.clear();
  }

  if (this->m_pass) {
    this->m_device->gpu.destroy(this->m_pass,
                                     this->m_device->allocate_cb,
                                     this->m_device->m_dispatch);
    this->m_pass = nullptr;
  }

  this->m_framebuffers.clear();
  this->m_attachments.clear();
  this->m_subpasses.clear();
}

auto RenderPass::operator=(RenderPass&& mv) -> RenderPass& {
  this->m_framebuffers = std::move(mv.m_framebuffers);
  this->m_subpasses = std::move(mv.m_subpasses);
  this->m_attachments = std::move(mv.m_attachments);
  this->m_device = mv.m_device;
  this->m_pass = mv.m_pass;
  this->m_area = mv.m_area;
  this->m_surface = mv.m_surface;
  this->m_current_framebuffer = mv.m_current_framebuffer;
  this->m_num_binded_subpasses = mv.m_num_binded_subpasses;

  mv.m_framebuffers.clear();
  mv.m_subpasses.clear();
  mv.m_attachments.clear();
  mv.m_device = nullptr;
  mv.m_pass = nullptr;
  mv.m_area = vk::Rect2D();
  mv.m_surface = nullptr;
  mv.m_current_framebuffer = 0;
  mv.m_num_binded_subpasses = 0;
  return *this;
}

auto RenderPass::make_images() -> void {
}

auto RenderPass::add_subpass(const gfx::Subpass& in_subpass, std::size_t index) -> void {
  auto& res = global_resources();
  auto& subpass = this->m_subpasses[index];
  subpass.desc.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
  
  for (auto index = 0u; index < in_subpass.attachments.size(); index++) {
    auto color = vk::ClearColorValue();
    auto reference = vk::AttachmentReference(); 
    auto clear = vk::ClearValue();
    auto& attachment = in_subpass.attachments[index];
    auto description = convert(attachment);
    
    auto& img = res.images[attachment.views[0].handle()];
    for(auto index = 0u; index < 4; ++index) color.float32[index] = attachment.clear_color[index];
    const auto is_depth = description.finalLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal;
    const auto is_color = !(is_depth);
    color = attachment.clear_color;
    clear.setColor(color);
    reference.setAttachment(this->m_attachments.size());
    if(is_depth) {
      reference.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
      subpass.depth = reference;  
      clear.depthStencil.depth = attachment.clear_color[0];
      clear.depthStencil.stencil = 0;
      subpass.desc.setPDepthStencilAttachment(std::addressof(*subpass.depth));
    } else if(is_color) {
      reference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
      if(img.imported) {
        description.finalLayout = vk::ImageLayout::ePresentSrcKHR;
      }
      subpass.color.push_back(reference);
    }

    subpass.name = in_subpass.name;
    subpass.clear_values.push_back(clear);
    subpass.luna_attachments.push_back(in_subpass.attachments[index]);
    this->m_attachments.push_back(description);
  }
  subpass.desc.setColorAttachments(subpass.color);
}

auto RenderPass::parse_info(const gfx::RenderPassInfo& info) -> void {
  this->m_subpasses.resize(info.subpasses.size());
  for(auto index = 0u; index < info.subpasses.size(); ++index) {
    this->add_subpass(info.subpasses[index], index);
  }
}

auto RenderPass::make_render_pass() -> void {
  auto info = vk::RenderPassCreateInfo();

  auto subpass_descriptions = std::vector<vk::SubpassDescription>();
  subpass_descriptions.reserve(this->m_subpasses.size());
  for(auto& subpass : this->m_subpasses) {
    subpass_descriptions.push_back(subpass.desc);
  }
  auto device = this->m_device->gpu;
  auto& dispatch = this->m_device->m_dispatch;
  auto* alloc_cb = this->m_device->allocate_cb;

  info.setAttachments(this->m_attachments);
  //info.setDependencies(this->m_dependencies);
  info.setSubpasses(subpass_descriptions);

  this->m_pass = error(device.createRenderPass(info, alloc_cb, dispatch));
}

auto RenderPass::bind() -> void {
  auto& res = global_resources();
  auto* gpu = this->m_device;

  auto num_buffers = this->m_subpasses[0].luna_attachments[0].views.size();

  for(auto index = 0u; index < num_buffers; index++) {
    auto info = vk::FramebufferCreateInfo();
    auto views = std::vector<vk::ImageView>();
    views.reserve(this->m_attachments.size());
    info.setWidth(this->m_area.extent.width);
    info.setHeight(this->m_area.extent.height);
    info.setAttachmentCount(this->m_attachments.size());
    info.setLayers(1);
    info.setRenderPass(this->m_pass);
    for(auto& subpass : this->m_subpasses) {
      for(auto& attach : subpass.luna_attachments) {
        LunaAssert(attach.views.size() == num_buffers, "All images in a render pass must have the same buffering (all must be single/double/triple buffered).");
        views.push_back(res.images[attach.views[index].handle()].view);
      };
    }
  
    info.setAttachments(views);
    auto fb = error(gpu->gpu.createFramebuffer(info, gpu->allocate_cb, gpu->m_dispatch));
    this->m_framebuffers.push_back(fb);
  }
}
}  // namespace vulkan
}  // namespace luna