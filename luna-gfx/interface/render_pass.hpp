#pragma once
#include "luna-gfx/interface/image.hpp"
#include <array>
#include <cstdint>
#include <vector>
#include <utility>
#include <unordered_map>
namespace luna {
namespace gfx {
class Window;
enum class AttachmentType {
  Color,
  Depth,
};

struct Attachment {
  // For N-style buffering.
  std::vector<ImageView> views;
  std::array<float, 4> clear_color = {0, 0, 0, 0};
};

struct Subpass {
  std::string name = "Default";
  std::vector<Attachment> attachments = {};
  //                 subpass      attachment
  std::unordered_map<std::string, std::string> dependencies;
  float depth_clear = 0.0f;
  bool enable_depth = true;
};

struct RenderPassInfo {
  int gpu = 0;
  size_t width = 1280;
  size_t height = 1024;
  std::vector<Subpass> subpasses = {};
};

// Helper class to create and own framebuffers for render pass usage.
class FramebufferCreator {
public:
  FramebufferCreator() = default;
  FramebufferCreator(int gpu, std::size_t width, std::size_t height, std::unordered_map<std::string, gfx::ImageFormat> requested_framebuffers);
  FramebufferCreator(FramebufferCreator&& mv) = default;
  inline ~FramebufferCreator() = default;
  auto operator[](std::string name) -> std::vector<gfx::ImageView> {return this->views()[name];}
  auto views() -> std::unordered_map<std::string, std::vector<gfx::ImageView>>;
  auto operator=(FramebufferCreator&& mv) -> FramebufferCreator& = default;
private:
  std::vector<gfx::Image> m_images;
};

class RenderPass {
  public:
    RenderPass(const RenderPass& cpy) = delete;
    auto operator=(const RenderPass& cpy) -> RenderPass& = delete;

    RenderPass() {this->m_handle = -1;}
    RenderPass(RenderPassInfo info);
    RenderPass(RenderPass&& mv) {*this = std::move(mv);}
    ~RenderPass();
    [[nodiscard]] inline auto info() const {return this->m_info;}
    [[nodiscard]] inline auto handle() const {return this->m_handle;}
    auto operator=(RenderPass&& mv) -> RenderPass& {this->m_handle = mv.m_handle; mv.m_handle = -1; this->m_info = mv.m_info; return *this;};
private:
  std::int32_t m_handle;
  RenderPassInfo m_info;
};
}
}