#include "luna-gfx/interface/image.hpp"
#include "luna-gfx/vulkan/utils/helper_functions.hpp"
#include <cstddef>
#include <cstdint>
namespace luna {
namespace gfx {
Image::Image(ImageInfo info, const unsigned char* initial_data) {
  auto usage = vulkan::usage_from_format(info.format);
  this->m_handle = luna::vulkan::create_image(info, vk::ImageLayout::eUndefined, usage, initial_data);
  this->m_info = info;
}

Image::~Image() {
  if(this->m_handle < 0) return;
  auto& res = vulkan::global_resources();
  auto& img = res.images[this->m_handle];
  // If image is imported, we leave whoever originally created it to destroy it.
  // In other words, if its imported, this is just a view to that image. Maybe should be a separate type? Idk.
  if(!img.imported) vulkan::destroy_image(this->m_handle);
  this->m_handle = -1;
  this->m_info = {};
}


auto ImageView::format() const -> ImageFormat {
  LunaAssert(this-m_handle >= 0, "Cannot access an invalid image view!");
  auto& res = vulkan::global_resources();
  auto& img = res.images[this->m_handle];
  return vulkan::convert(img.format);
}

auto ImageView::width() const -> std::size_t {
  LunaAssert(this-m_handle >= 0, "Cannot access an invalid image view!");
  auto& res = vulkan::global_resources();
  auto& img = res.images[this->m_handle];
  return img.info.width;
}

auto ImageView::height() const -> std::size_t {
  LunaAssert(this-m_handle >= 0, "Cannot access an invalid image view!");
  auto& res = vulkan::global_resources();
  auto& img = res.images[this->m_handle];
  return img.info.height;
}

auto ImageView::layers() const -> std::size_t {
  LunaAssert(this-m_handle >= 0, "Cannot access an invalid image view!");
  auto& res = vulkan::global_resources();
  auto& img = res.images[this->m_handle];
  return img.info.layers;
}
}
}