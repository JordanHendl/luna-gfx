#include "luna-gfx/interface/image.hpp"
#include "luna-gfx/vulkan/utils/helper_functions.hpp"
#include <cstddef>
#include <cstdint>
namespace luna {
namespace gfx {
Image::Image(ImageInfo info, const unsigned char* initial_data) {
  this->m_handle = luna::vulkan::create_image(info, vk::ImageLayout::eUndefined, vulkan::standard_image_usage(), initial_data);
  this->m_info = info;
}

Image::~Image() {
  if(this->m_handle < 0) return;
  vulkan::destroy_image(this->m_handle);
  this->m_handle = -1;
  this->m_info = {};
}
}
}