#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
namespace luna {
namespace gfx {
enum class ImageFormat {
  RGBA8,
  BGRA8,
  RGBA32F,
  Depth,
};

struct ImageInfo {
  std::string name = "LunaImage";
  int gpu = 0;
  size_t width = 1280;
  size_t height = 1024;
  size_t layers = 1;
  ImageFormat format = ImageFormat::RGBA8;
  size_t num_mips = 1;
  bool is_cubemap = false;
};

class Image {
  public:
    Image() {this->m_handle = -1;}
    Image(ImageInfo info, const unsigned char* initial_data = nullptr);
    ~Image();
    Image(Image&& mv) = default;
    Image(const Image& cpy) = delete;

    auto operator=(Image&& mv) -> Image& = default;
    auto operator=(const Image& cpy) -> Image& = delete;
    [[nodiscard]] inline auto handle() const -> std::int32_t {return this->m_handle;}
    [[nodiscard]] inline auto info() const {return this->m_info;}
  private:
    std::int32_t m_handle;
    ImageInfo m_info;
};
}
}