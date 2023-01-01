#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
namespace luna {
namespace gfx {
class ImageView;
class Image;
enum class ImageFormat {
  RGBA8,
  RGB8,
  RG8,
  R8,
  BGRA8,
  RGBA32F,
  RGB32F,
  RG32F,
  R32F,
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
    Image(const Image& cpy) = delete;
    auto operator=(const Image& cpy) -> Image& = delete;

    Image() {this->m_handle = -1;}
    Image(ImageInfo info, const unsigned char* initial_data = nullptr);
    Image(Image&& mv) {*this = std::move(mv);};
    ~Image();

    auto operator=(Image&& mv) -> Image& {
      this->m_handle = mv.m_handle;
      this->m_info = mv.m_info;
      mv.m_handle = -1;
      return *this;
    }

    template<typename T>
    auto upload(const T* ptr) -> void {
      this->upload_raw(reinterpret_cast<const unsigned char*>(ptr));
    }

    [[nodiscard]] inline auto handle() const -> std::int32_t {return this->m_handle;}
    [[nodiscard]] inline auto info() const {return this->m_info;}
  private:
    auto upload_raw(const unsigned char* ptr) -> void;
    friend class Window;
    std::int32_t m_handle;
    ImageInfo m_info;
};

// A const view into an image.
class ImageView {
public:
  ImageView() {this->m_handle = -1;}
  ImageView(std::int32_t handle) {this->m_handle = handle;}
  ImageView(const Image& img) {this->m_handle = img.handle();}
  ~ImageView() = default;
  auto operator=(const Image& ref) -> ImageView& {this->m_handle = ref.handle(); return *this;}
  [[nodiscard]] auto handle() const -> std::int32_t {return this->m_handle;}
  [[nodiscard]] auto name() const -> std::string;
  [[nodiscard]] auto format() const -> ImageFormat;
  [[nodiscard]] auto width() const -> std::size_t;
  [[nodiscard]] auto height() const -> std::size_t;
  [[nodiscard]] auto layers() const -> std::size_t;
private:
  std::int32_t m_handle;
};
}
}