#include "luna-gfx/interface/image.hpp"
#include "luna-gfx/interface/buffer.hpp"
#include "luna-gfx/vulkan/utils/helper_functions.hpp"
#include <cstddef>
#include <cstdint>
namespace luna {
namespace gfx {
Image::Image(ImageInfo info, const unsigned char* initial_data) {
  auto usage = vulkan::usage_from_format(info.format);
  this->m_handle = luna::vulkan::create_image(info, vk::ImageLayout::eUndefined, usage, nullptr);
  this->info() = info;
  if(initial_data != nullptr) {
    this->upload_raw(initial_data);
  }
}

Image::~Image() {
  if(this->m_handle < 0) return;
  auto& res = vulkan::global_resources();
  auto& img = res.images[this->m_handle];
  // If image is imported, we leave whoever originally created it to destroy it.
  // In other words, if its imported, this is just a view to that image. Maybe should be a separate type? Idk.
  if(!img.imported) vulkan::destroy_image(this->m_handle);
  this->m_handle = -1;
}

[[nodiscard]] inline auto Image::info() const -> ImageInfo {
  return vulkan::global_resources().images[this->m_handle].info;
}

auto Image::upload_raw(const unsigned char* ptr) -> void {
    auto amt = this->info().width * this->info().height * luna::vulkan::size_from_format(this->info().format);
    auto tmp_buffer = MemoryBuffer(this->info().gpu, amt, MemoryType::CPUVisible);
    tmp_buffer.upload(ptr);

    auto cmd = luna::vulkan::create_cmd(this->info().gpu);
    luna::vulkan::begin_command_buffer(cmd);
    luna::vulkan::copy_buffer_to_image(cmd, tmp_buffer.handle(), this->handle());
    luna::vulkan::end_command_buffer(cmd);
    luna::vulkan::submit_command_buffer(cmd);
    luna::vulkan::synchronize_cmd(cmd);
    luna::vulkan::destroy_cmd(cmd);
}

auto ImageView::name() const -> std::string {
  LunaAssert(this-m_handle >= 0, "Cannot access an invalid image view!");
  auto& res = vulkan::global_resources();
  auto& img = res.images[this->m_handle];
  return img.info.name;
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