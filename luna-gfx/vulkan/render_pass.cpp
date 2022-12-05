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

  auto is_depth = attachment.info.format == gfx::ImageFormat::Depth;
  auto format = convert(attachment.info.format);
  auto layout = vk::ImageLayout::eColorAttachmentOptimal;
  auto stencil_store = is_depth ? StoreOps::eStore : StoreOps::eDontCare;
  auto stencil_load = is_depth ? LoadOps::eLoad : LoadOps::eDontCare;
  auto load_op = LoadOps::eClear;  /// TODO make configurable
  auto store_op = StoreOps::eStore;

  if(is_depth) layout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;

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
    for(auto& img : subpass.images) {
      destroy_image(img[0]);
      destroy_image(img[1]);
      destroy_image(img[2]);
    }
    subpass.images.clear();
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

  for(auto& subpass : this->m_subpasses) {
    subpass.images.resize(subpass.luna_attachments.size());
    auto create_imgs = [&] (auto& attachment) {
      auto transfer_bits = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;
      auto depth_usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | transfer_bits;
      auto color_usage = vk::ImageUsageFlagBits::eColorAttachment | transfer_bits;
      auto usage = attachment.info.format == gfx::ImageFormat::Depth ?  depth_usage : color_usage;

      auto img1 = create_image(attachment.info, vk::ImageLayout::eUndefined, usage, nullptr);
      auto img2 = create_image(attachment.info, vk::ImageLayout::eUndefined, usage, nullptr);
      auto img3 = create_image(attachment.info, vk::ImageLayout::eUndefined, usage, nullptr);
      auto tmp = std::array<std::int32_t, 3>{img1, img2, img3}; 
      return tmp;
    };

    // oops prob should do this to the last subpass only? @TODO 
    if(this->m_swap == nullptr) {
      std::transform(subpass.luna_attachments.begin(), subpass.luna_attachments.end(), subpass.images.begin(), create_imgs); 
    } else {
      std::transform(subpass.luna_attachments.begin(), subpass.luna_attachments.end() - 1, subpass.images.begin(), create_imgs);
      LunaAssert(convert(subpass.luna_attachments.back().info.format) == this->m_swap->format(), "Trying to attach a window to a render pass when the format of the last attachment of the subpass is the same as the window's.");
      subpass.images.back() = {this->m_swap->image(0), this->m_swap->image(1), this->m_swap->image(2)};
    }
    }
}

auto RenderPass::add_subpass(const gfx::Subpass& in_subpass) -> void {
  auto subpass = RenderPass::Subpass();
  subpass.desc.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

  for (auto index = 0u; index < in_subpass.attachments.size(); index++) {
    auto color = vk::ClearColorValue();
    auto reference = vk::AttachmentReference(); 
    auto clear = vk::ClearValue();
    auto& attachment = in_subpass.attachments[index];
    auto description = convert(attachment);

    for(auto index = 0u; index < 4; ++index) color.float32[index] = attachment.clear_color[index];
    const auto is_depth = description.finalLayout == vk::ImageLayout::eDepthStencilReadOnlyOptimal;
    const auto is_color = !(is_depth);

    clear.setColor(color);
    reference.setAttachment(this->m_attachments.size());

    if(is_depth) {
      reference.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
      subpass.depth = reference;  
      subpass.desc.setPDepthStencilAttachment(std::addressof(*subpass.depth));
    } else if(is_color) {
      reference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
      subpass.color.push_back(reference);
    }

    subpass.luna_attachments.push_back(in_subpass.attachments[index]);
    this->m_attachments.push_back(description);
  }

  this->m_subpasses.push_back(subpass);
}

auto RenderPass::parse_info(const gfx::RenderPassInfo& info) -> void {
  for (auto& subpass : info.subpasses) {
    this->add_subpass(subpass);
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

  for(auto index = 0u; index < 3; index++) {
    for(auto& subpass : this->m_subpasses) {
      auto info = vk::FramebufferCreateInfo();
      auto views = std::vector<vk::ImageView>();

      info.setWidth(this->m_area.extent.width);
      info.setHeight(this->m_area.extent.height);

      info.setAttachmentCount(subpass.images.size());
      info.setLayers(1);
      info.setRenderPass(this->m_pass);
  
      views.reserve(subpass.images.size());
      for(auto& imgs : subpass.images) {
        views.push_back(res.images[imgs[index]].view);
      }
  
      info.setAttachments(views);
      auto fb = error(gpu->gpu.createFramebuffer(info, gpu->allocate_cb, gpu->m_dispatch));
      this->m_framebuffers.push_back(fb);
    }
  }
}
}  // namespace vulkan
}  // namespace luna