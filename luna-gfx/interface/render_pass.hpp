#pragma once
#include "luna-gfx/interface/image.hpp"
#include <array>
#include <cstdint>
#include <vector>
#include <utility>
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
  std::vector<Attachment> attachments = {};
  std::vector<size_t> dependancies = {};
  float depth_clear = 0.0f;
  bool enable_depth = true;
};

struct RenderPassInfo {
  int gpu = 0;
  size_t width = 1280;
  size_t height = 1024;
  std::vector<Subpass> subpasses = {};
};

class RenderPass {
  public:
    RenderPass() {this->m_handle = -1;}
    RenderPass(RenderPassInfo info);
    RenderPass(RenderPass&& mv) {*this = std::move(mv);}
    RenderPass(const RenderPass& cpy) = delete;
    ~RenderPass();
    [[nodiscard]] inline auto info() const {return this->m_info;}
    [[nodiscard]] inline auto handle() const {return this->m_handle;}
    auto operator=(RenderPass&& mv) -> RenderPass& {this->m_handle = mv.m_handle; mv.m_handle = -1; this->m_info = mv.m_info; return *this;};
    auto operator=(const RenderPass& cpy) -> RenderPass& = delete;
private:
  std::int32_t m_handle;
  RenderPassInfo m_info;
};
}
}