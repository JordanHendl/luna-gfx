#pragma once
#include "luna-gfx/interface/render_pass.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
namespace luna {
namespace gfx {

struct WindowInfo {
  std::string title = "LunaWindow";
  size_t width = 1280;
  size_t height = 1024;
  int gpu = 0;
  bool borderless = false;
  bool fullscreen = false;
  bool resizable = true;
  bool shown = true;
  bool capture_mouse = false;
  bool vsync = false;
};

class Window {
  public:
    Window() {this->m_handle = -1;}
    Window(WindowInfo info);
    Window(Window&& mv) {*this = std::move(mv);};
    Window(const Window& cpy) = delete;
    ~Window();
    
    // This function returns the type of attachment for this window's framebuffer format. Used primarily for attaching this window to a render pass.
    [[nodiscard]] auto attachment() -> gfx::Attachment;
    [[nodiscard]] inline auto handle() const -> std::int32_t {return this->m_handle;}
    [[nodiscard]] inline auto info() const -> const WindowInfo& {return this->m_info;}
    auto operator=(Window&& mv) -> Window& {this->m_handle = mv.m_handle; mv.m_handle = -1; this->m_info = mv.m_info; return *this;};
    auto operator=(const Window& cpy) -> Window& = delete;
  private:
    std::int32_t m_handle;
    WindowInfo m_info;
};
}
}