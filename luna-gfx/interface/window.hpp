#pragma once
#include "luna-gfx/interface/render_pass.hpp"
#include "luna-gfx/interface/image.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <future>
#include <functional>
namespace luna {
namespace gfx {
class CommandList;
struct WindowInfo {
  std::string title = "LunaWindow";
  size_t width = 1280;
  size_t height = 1024;
  int gpu = 0;
  std::function<void()> resize_callback;
  bool borderless = false;
  bool fullscreen = false;
  bool resizable = false;
  bool shown = true;
  bool capture_mouse = false;
  bool vsync = false;
};

class Window {
  public:
    Window(const Window& cpy) = delete;

    Window() {this->m_handle = -1;}
    auto operator=(const Window& cpy) -> Window& = delete;

    Window(WindowInfo info);
    Window(Window&& mv) {*this = std::move(mv);};
    ~Window();
    auto width() -> std::size_t;
    auto height() -> std::size_t;
    auto acquire() -> std::size_t;
    auto combo_into(CommandList& cmd) -> void;
    auto present() -> void;
    [[nodiscard]] auto current_frame() -> std::size_t;
    [[nodiscard]] auto image_views() -> std::vector<ImageView>;
    [[nodiscard]] inline auto handle() const -> std::int32_t {return this->m_handle;}
    [[nodiscard]] inline auto info() const -> const WindowInfo& {return this->m_info;}
    auto operator=(Window&& mv) -> Window& {this->m_handle = mv.m_handle; mv.m_handle = -1; this->m_info = mv.m_info; return *this;};
  private:
    std::vector<Image> m_images;
    std::int32_t m_handle;
    WindowInfo m_info;
};
}
}