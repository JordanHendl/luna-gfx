#pragma once
#include "luna-gfx/vulkan/vulkan_defines.hpp"
#include "luna-gfx/interface/window.hpp"
#include <vulkan/vulkan.hpp>
#include <string>
#include <memory>
namespace luna {
namespace vulkan {
  class Window {
    public:
      Window();
      explicit Window(gfx::WindowInfo info);
      Window(Window&& mv);
      Window(const Window& cpy) = delete;
      ~Window();
      auto operator=(Window&& mv) -> Window&;
      auto operator=(const Window& cpy) -> Window& = delete;
      auto update(gfx::WindowInfo info) -> void;
      auto info() const -> const gfx::WindowInfo&;
      auto set_title(std::string_view title) -> void;
      auto set_borderless(bool value) -> void;
      auto set_fullscreen(bool value) -> void;
      auto set_width(size_t value) -> void;
      auto set_height(size_t value) -> void;
      auto set_xpos(size_t value) -> void;
      auto set_ypos(size_t value) -> void;
      auto set_resizable(bool value) -> void;
      auto set_monitor(size_t value) -> void;
      auto set_minimized(bool value) -> void;
      auto set_maximized(bool value) -> void;
      auto set_shown(bool value) -> void;
      auto set_mouse_capture(bool value) -> void;
      auto title() -> std::string_view;
      auto borderless() -> bool;
      auto fullscreen() -> bool;
      auto has_focus() -> bool;
      auto width() -> size_t;
      auto height() -> size_t;
      auto xpos() -> size_t;
      auto ypos() -> size_t;
      auto monitor() -> size_t;
      auto resizable() -> bool;
      auto minimized() -> bool;
      auto maximized() -> bool;
      auto surface() -> vk::SurfaceKHR;
      auto valid() const -> bool;
    private:
      struct WindowData;
      std::unique_ptr<WindowData> m_data;
  };
}
}