#include "luna-gfx/interface/bind_group.hpp"
#include "luna-gfx/interface/buffer.hpp"
#include "luna-gfx/interface/image.hpp"
#include "luna-gfx/vulkan/descriptor.hpp"
#include "luna-gfx/vulkan/global_resources.hpp"
namespace luna {
namespace gfx {
BindGroup::~BindGroup() {
  auto& res = vulkan::global_resources();
  auto& desc = res.descriptors[this->m_handle];
  desc = std::move(vulkan::Descriptor());
}

auto BindGroup::set(const MemoryBuffer& buffer, std::string_view str) -> bool {
  auto& res = vulkan::global_resources();
  auto& buf = res.buffers[buffer.handle()];
  auto& desc = res.descriptors[this->m_handle];
  return desc.bind(str, buf);
}

auto BindGroup::set(const Image& image, std::string_view str) -> bool {
  auto& res = vulkan::global_resources();
  auto& img = res.images[image.handle()];
  auto& desc = res.descriptors[this->m_handle];
  return desc.bind(str, img);
}
}
}