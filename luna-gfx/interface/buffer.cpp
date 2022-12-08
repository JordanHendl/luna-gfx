#include "luna-gfx/interface/buffer.hpp"
#include "luna-gfx/vulkan/global_resources.hpp"
#include "luna-gfx/vulkan/utils/helper_functions.hpp"
#include "luna-gfx/vulkan/data_types.hpp"
#include "luna-gfx/error/error.hpp"
namespace luna {
namespace gfx {
inline auto usage_from_type(MemoryType type) {
  using bits = vk::BufferUsageFlagBits;
  auto usage_flags = vk::BufferUsageFlags();
  usage_flags |= bits::eTransferDst | bits::eTransferSrc;
  switch(type) {
    case MemoryType::Vertex : usage_flags |= bits::eVertexBuffer | bits::eStorageBuffer | bits::eUniformBuffer; break;
    case MemoryType::Index : usage_flags |= bits::eIndexBuffer | bits::eStorageBuffer | bits::eUniformBuffer; break;
    case MemoryType::General : 
    case MemoryType::CPUVisible : usage_flags |= bits::eVertexBuffer | bits::eIndexBuffer | bits::eStorageBuffer | bits::eUniformBuffer; break;
    default : break;
  }

  return usage_flags;
}

inline auto is_mappable(MemoryType type) -> bool {
  switch(type) {
    case MemoryType::CPUVisible : return true;
    case MemoryType::General : return true;
    default: return false;
  }
}

MemoryBuffer::MemoryBuffer(int gpu, std::size_t size, MemoryType type) {
  auto index = luna::vulkan::create_buffer(gpu, size, usage_from_type(type), is_mappable(type));
  this->m_handle = index;
  this->m_type = type;
  this->m_size = size;
}

MemoryBuffer::~MemoryBuffer() {
  if(this->m_handle < 0) return; // Don't deconstruct an invalid handle!
  luna::vulkan::destroy_buffer(this->m_handle);
  this->m_handle = -1;
  this->m_size = 0;
  this->m_type = MemoryType::Unknown;
}

auto MemoryBuffer::gpu() const -> int {
  auto& res = vulkan::global_resources();
  return res.buffers[this->m_handle].gpu;
}

auto MemoryBuffer::unmap() -> void {
  if(is_mappable(this->m_type)) luna::vulkan::unmap_buffer(this->m_handle);
}

auto MemoryBuffer::map_impl(void** ptr) -> void {
  if(is_mappable(this->m_type)) luna::vulkan::map_buffer(this->m_handle, ptr);
}

auto MemoryBuffer::upload_data_impl(const unsigned char* in_data, std::size_t num_bytes) -> void {
  auto& buf = vulkan::global_resources().buffers[this->m_handle];
  unsigned char* ptr = nullptr;

  if(is_mappable(this->m_type)) {
    this->map(&ptr);
    std::memcpy(ptr, in_data, num_bytes);
    this->unmap();
  } else {
    auto tmp_buffer = MemoryBuffer(buf.gpu, this->m_size, MemoryType::CPUVisible);
    tmp_buffer.upload(in_data);

    auto cmd = luna::vulkan::create_cmd(buf.gpu);
    luna::vulkan::begin_command_buffer(cmd);
    luna::vulkan::copy_buffer_to_buffer(cmd, tmp_buffer.handle(), this->handle(), num_bytes);
    luna::vulkan::end_command_buffer(cmd);
    luna::vulkan::submit_command_buffer(cmd);
    luna::vulkan::synchronize_cmd(cmd);
    luna::vulkan::destroy_cmd(cmd);
  }
}
}
}