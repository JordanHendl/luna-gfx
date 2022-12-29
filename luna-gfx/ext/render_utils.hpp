#pragma once
#include "luna-gfx/gfx.hpp"
#include "unordered_map"
#include <optional>
#include <memory>
namespace luna {
namespace gfx {

/**
struct QuickPSO {
  enum class FramebufferType {
    GBuffer,
    Position,
    Depth,
  };

  gfx::RenderPass render_pass;
  std::unordered_map<FramebufferType, std::vector<std::unique_ptr<gfx::Image>>> framebuffers;
  std::optional<gfx::Window> window;
};

auto quick_forward_render_pass_with_window(int cGPU = 0, std::size_t width = 1280, std::size_t height = 1024) -> QuickPSO {
  constexpr auto cClearColors = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f};
  auto pso = QuickPSO();
  auto& depth_images = pso.framebuffers[QuickPSO::FramebufferType::Depth];

  auto win_info = gfx::WindowInfo();
  win_info.width = width;
  win_info.height = height;
  pso.window = gfx::Window(win_info);
  auto info = gfx::RenderPassInfo();
  auto subpass = gfx::Subpass();
  auto attachment = gfx::Attachment();

  // Set up the color attachment (writing to the window's image buffers)
  attachment.clear_color = cClearColors;
  attachment.views = pso.window.image_views();
  subpass.attachments.push_back(attachment);


  // Now, set up the depth attachments.
  attachment.views.clear();
  auto depth_img_info = gfx::ImageInfo();
  depth_images.resize(pso.window.image_views().size());
  depth_img_info.format = luna::gfx::ImageFormat::Depth;
  depth_img_info.width = pso.window.info().width;
  depth_img_info.height = pso.window.info().height;
  depth_img_info.gpu = cGPU;
  attachment.clear_color = {1.0f, 1.0f, 1.0f, 1.0f};

  // They need to be the same amount of buffers as the image attachments (probably triple-buffered)
  for(auto& img : depth_images) {
    img = std::move(std::make_unique<luna::gfx::Image>(depth_img_info));
    attachment.views.push_back(img);
  }

  subpass.attachments.push_back(attachment);
  info.subpasses.push_back(subpass);
  info.gpu = cGPU;
  info.width = cWidth;
  info.height = cHeight;

  pso.render_pass = gfx::RenderPass(info);
  return pso;
}

auto quick_forward_render_pass(int cGPU = 0, std::size_t width = 1280, std::size_t height = 1024) -> QuickPSO {
  constexpr auto cClearColors = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f};
  constexpr auto cNumBuffers = 3;
  auto pso = QuickPSO();
  auto& color_images = pso.framebuffers[QuickPSO::FramebufferType::GBuffer];
  auto& depth_images = pso.framebuffers[QuickPSO::FramebufferType::Depth];

  auto info = gfx::RenderPassInfo();
  auto subpass = gfx::Subpass();
  auto attachment = gfx::Attachment();

  // Set up the color attachment (writing to the window's image buffers)
  attachment.clear_color = cClearColors;
  attachment.views = window.image_views();
  subpass.attachments.push_back(attachment);


  // Now, set up the depth attachments.
  attachment.views.clear();
  auto img_info = gfx::ImageInfo();
  img_info.width = width;
  img_info.height = height;
  img_info.gpu = cGPU;

  // Create GBuffer images.
  color_images.resize(cNumBuffers);
  img_info.format = luna::gfx::ImageFormat::RGBA8;
  attachment.clear_color = {1.0f, 0.0f, 0.0f, 1.0f};

  for(auto& img : color_images) {
    img = std::move(std::make_unique<luna::gfx::Image>(img_info));
    attachment.views.push_back(img);
  }

  // Create Depth buffer images
  depth_images.resize(cNumBuffers);
  img_info.format = luna::gfx::ImageFormat::Depth;
  attachment.clear_color = {1.0f, 1.0f, 1.0f, 1.0f};

  for(auto& img : depth_images) {
    img = std::move(std::make_unique<luna::gfx::Image>(img_info));
    attachment.views.push_back(img);
  }

  subpass.attachments.push_back(attachment);
  info.subpasses.push_back(subpass);
  info.gpu = cGPU;
  info.width = cWidth;
  info.height = cHeight;

  pso.render_pass = gfx::RenderPass(info);
  return pso;
}

auto quick_deferred_render_pass() -> gfx::RenderPass {
  static_assert(false);
  auto rp = gfx::RenderPass();
  return rp;
}
*/
}
}