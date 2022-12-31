#include "luna-gfx/vulkan/window.hpp"
#include "luna-gfx/vulkan/global_resources.hpp"
#include "luna-gfx/vulkan/instance.hpp"
#include "luna-gfx/error/error.hpp"
#include <SDL.h>
#include <SDL_vulkan.h>
#include <string>
#include <utility>
namespace luna {
namespace vulkan {

struct Window::WindowData {
  SDL_Window* m_window = nullptr;
  vk::SurfaceKHR m_surface = {};
};

Window::Window() {
  this->m_data = std::make_unique<Window::WindowData>();
}

Window::Window(gfx::WindowInfo info) {
  this->m_data = std::make_unique<Window::WindowData>();
  const auto flags = SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN;
  this->m_data->m_window =
      (SDL_CreateWindow(info.title.c_str(), 0, 256, static_cast<int>(info.width), static_cast<int>(info.height),
                        SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN));

  auto raw_surface = VkSurfaceKHR{};
  auto error = SDL_Vulkan_CreateSurface(this->m_data->m_window, global_resources().instance->m_instance, &raw_surface);
  LunaAssert(error, "Failed to create surface for window!");
  (void)error;
  this->m_data->m_surface = raw_surface;

  //this->update(info);
}

Window::Window(Window&& mv) { 
  this->m_data = std::make_unique<Window::WindowData>();
  *this = std::move(mv); 
}

Window::~Window() {
  if (this->m_data->m_window) SDL_DestroyWindow(this->m_data->m_window);
  this->m_data->m_window = nullptr;
  this->m_data->m_surface = nullptr;
}

auto Window::operator=(Window&& mv) -> Window& {
  this->m_data->m_window = mv.m_data->m_window;
  this->m_data->m_surface = mv.m_data->m_surface;
  mv.m_data->m_window = nullptr;
  mv.m_data->m_surface = nullptr;
  return *this;
}

auto Window::update(gfx::WindowInfo info) -> void {
  this->set_mouse_capture(info.capture_mouse);
  this->set_borderless(info.borderless);
  if (info.width != this->width()) this->set_width(info.width);
  if (info.height != this->height()) this->set_height(info.height);
  if (info.fullscreen != this->fullscreen())
    this->set_fullscreen(info.fullscreen);
  if (info.resizable != this->resizable()) this->set_resizable(info.resizable);
  if (info.title != this->title()) this->set_title(info.title);
}

auto Window::set_title(std::string_view title) -> void {
  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");

  SDL_SetWindowTitle(this->m_data->m_window, title.data());
}

auto Window::set_borderless(bool value) -> void {
  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  SDL_SetWindowBordered(this->m_data->m_window, static_cast<SDL_bool>(value));
}

auto Window::set_fullscreen(bool value) -> void {
  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  SDL_SetWindowFullscreen(this->m_data->m_window, value);
}

auto Window::set_width(size_t value) -> void {
  int x;
  int y;

  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  SDL_GetWindowSize(this->m_data->m_window, &x, &y);
  x = value;
  SDL_SetWindowSize(this->m_data->m_window, x, y);
}

auto Window::set_height(size_t value) -> void {
  int x;
  int y;

  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  SDL_GetWindowSize(this->m_data->m_window, &x, &y);
  y = value;
  SDL_SetWindowSize(this->m_data->m_window, x, y);
}

auto Window::set_xpos(size_t value) -> void {
  int x;
  int y;

  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  SDL_GetWindowPosition(this->m_data->m_window, &x, &y);
  x = value;
  SDL_SetWindowPosition(this->m_data->m_window, x, y);
}

auto Window::set_ypos(size_t value) -> void {
  int x;
  int y;

  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  SDL_GetWindowPosition(this->m_data->m_window, &x, &y);
  y = value;
  SDL_SetWindowPosition(this->m_data->m_window, x, y);
}

auto Window::set_resizable(bool value) -> void {
  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  SDL_SetWindowResizable(this->m_data->m_window, static_cast<SDL_bool>(value));
}

auto Window::set_monitor(size_t value) -> void {
  SDL_DisplayMode mode;
  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  SDL_GetDesktopDisplayMode(value, &mode);
  SDL_SetWindowDisplayMode(this->m_data->m_window, &mode);
}

auto Window::set_minimized(bool value) -> void {
  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  if (value)
    SDL_MinimizeWindow(this->m_data->m_window);
  else
    SDL_RestoreWindow(this->m_data->m_window);
}

auto Window::set_maximized(bool value) -> void {
  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  if (value)
    SDL_MaximizeWindow(this->m_data->m_window);
  else
    SDL_RestoreWindow(this->m_data->m_window);
}

auto Window::set_shown(bool value) -> void {
  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  if (value)
    SDL_ShowWindow(this->m_data->m_window);
  else
    SDL_HideWindow(this->m_data->m_window);
}

auto Window::set_mouse_capture(bool value) -> void {
  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  SDL_CaptureMouse(static_cast<SDL_bool>(value));
  SDL_SetWindowGrab(this->m_data->m_window, static_cast<SDL_bool>(value));

  // If we capture the mouse, we want to disable the cursor at the same time.
  auto enabled = value ? SDL_DISABLE : SDL_ENABLE;
  SDL_SetRelativeMouseMode(static_cast<SDL_bool>(value));
  SDL_ShowCursor(enabled);
}

auto Window::valid() const -> bool {
  return this->m_data->m_window;
}

auto Window::title() -> std::string_view {
  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  return SDL_GetWindowTitle(this->m_data->m_window);
}

auto Window::borderless() -> bool {
  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  auto flags = SDL_GetWindowFlags(this->m_data->m_window);

  return flags & SDL_WINDOW_BORDERLESS;
}

auto Window::fullscreen() -> bool {
  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  auto flags = SDL_GetWindowFlags(this->m_data->m_window);

  return flags & SDL_WINDOW_FULLSCREEN;
}

auto Window::has_focus() -> bool {
  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  auto flags = SDL_GetWindowFlags(this->m_data->m_window);

  return flags & SDL_WINDOW_INPUT_FOCUS;
}

auto Window::width() -> size_t {
  int w = 0;
  int h = 0;

  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  SDL_GetWindowSize(this->m_data->m_window, &w, &h);

  return w;
}

auto Window::height() -> size_t {
  int w = 0;
  int h = 0;

  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  SDL_GetWindowSize(this->m_data->m_window, &w, &h);

  return h;
}

auto Window::xpos() -> size_t {
  int x = 0;
  int y = 0;

  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  SDL_GetWindowPosition(this->m_data->m_window, &x, &y);

  return x;
}

auto Window::ypos() -> size_t {
  int x = 0;
  int y = 0;

  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  SDL_GetWindowPosition(this->m_data->m_window, &x, &y);

  return y;
}

auto Window::monitor() -> size_t {
  int x = 0;
  int y = 0;

  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  SDL_GetWindowPosition(this->m_data->m_window, &x, &y);

  return y;
}

auto Window::resizable() -> bool {
  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  auto flags = SDL_GetWindowFlags(this->m_data->m_window);

  return flags & SDL_WINDOW_RESIZABLE;
}

auto Window::minimized() -> bool {
  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  auto flags = SDL_GetWindowFlags(this->m_data->m_window);

  return flags & SDL_WINDOW_MINIMIZED;
}

auto Window::surface() -> vk::SurfaceKHR {
  return this->m_data->m_surface;
}

auto Window::maximized() -> bool {
  LunaAssert(
      this->m_data->m_window,
      "Attempting to access athis->m_data->m_window that has not been initialized.");
  auto flags = SDL_GetWindowFlags(this->m_data->m_window);

  return flags & SDL_WINDOW_MAXIMIZED;
}
}  // namespace gfx
}  // namespace luna