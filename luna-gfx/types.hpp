#pragma once
#include <cstddef>
#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "luna-gfx/common/shader.hpp"
namespace luna {
namespace gfx {

struct WindowInfo {
  std::string title;
  size_t width;
  size_t height;
  bool borderless;
  bool fullscreen;
  bool resizable;
  bool shown;
  bool capture_mouse;
  bool vsync;

  WindowInfo(std::string_view name, size_t width = 1280, size_t height = 1024,
             bool resizable = false) {
    this->title = name;
    this->width = width;
    this->height = height;
    this->resizable = resizable;
    this->borderless = false;
    this->fullscreen = false;
    this->shown = true;
    this->capture_mouse = false;
    this->vsync = false;
  }

  WindowInfo(size_t width, size_t height) {
    this->title = "Window";
    this->width = width;
    this->height = height;
    this->borderless = false;
    this->fullscreen = false;
    this->resizable = false;
    this->shown = true;
    this->capture_mouse = false;
    this->vsync = false;
  }

  WindowInfo() {
    this->title = "Window";
    this->width = 1280;
    this->height = 1024;
    this->borderless = false;
    this->fullscreen = false;
    this->resizable = false;
    this->shown = true;
    this->capture_mouse = false;
    this->vsync = false;
  }
};

enum class ImageFormat {
  RGBA8,
  BGRA8,
  RGBA32F,
  Depth,
};

enum class AttachmentType {
  Color,
  Depth,
};



struct Attachment {
  ImageInfo info;
  std::array<float, 4> clear_color;

  Attachment() {
    this->info = {};
    this->clear_color = {0, 0, 0, 0};
  }
};

struct Subpass {
  std::vector<Attachment> attachments;
  std::vector<size_t> dependancies;
  float depth_clear;
  bool enable_depth;

  Subpass() {
    this->depth_clear = 0.0f;
    this->enable_depth = false;
  }

  Subpass(Attachment info, bool depth = false) {
    this->attachments.push_back(info);
    this->depth_clear = 0.0f;
    this->enable_depth = depth;
  }
};
struct RenderPassInfo {
  int gpu = 0;
  size_t width = 1280;
  size_t height = 1024;
  std::vector<Subpass> subpasses;

  RenderPassInfo() {
    this->width = 1280;
    this->height = 1024;
  }

  RenderPassInfo(size_t width, size_t height) {
    this->width = width;
    this->height = height;
  }
};
}
}