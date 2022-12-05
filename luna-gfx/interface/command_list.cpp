#include "luna-gfx/interface/command_list.hpp"
#include "luna-gfx/interface/buffer.hpp"
#include "luna-gfx/interface/image.hpp"
#include "luna-gfx/vulkan/utils/helper_functions.hpp"
#include "luna-gfx/error/error.hpp"
#include <algorithm>
namespace luna {
namespace gfx {
  CommandList::CommandList(int gpu, Queue queue) {
    this->m_handle = vulkan::create_cmd(gpu, queue);
  }

  CommandList::CommandList(int gpu, CommandList& in_parent) {
    auto& res = vulkan::global_resources();
    auto& parent = res.cmds[in_parent.handle()];
    this->m_handle = vulkan::create_cmd(gpu, in_parent.queue(), &parent);
  }

  CommandList::~CommandList() {
    if(this->m_handle < 0) return;
    vulkan::destroy_cmd(this->m_handle);
    this->m_handle = -1;
  }

  auto CommandList::begin() -> void {
    LunaAssert(this->m_handle >= 0, "Unable to begin an invalid command buffer.");
    vulkan::begin_command_buffer(this->m_handle);
  }

  auto CommandList::end() -> void {
    LunaAssert(this->m_handle >= 0, "Unable to end an invalid command buffer.");
    vulkan::end_command_buffer(this->m_handle);
  }

  auto CommandList::submit() -> void {
    LunaAssert(this->m_handle >= 0, "Unable to submit an invalid command buffer.");
    vulkan::submit_command_buffer(this->m_handle);
  }

  auto CommandList::synchronize() -> void {
    LunaAssert(this->m_handle >= 0, "Unable to synchronize an invalid command buffer.");
    vulkan::synchronize_cmd(this->m_handle);
  }

  auto CommandList::wait_on(const CommandList& cmd) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to tell an invalid command buffer to wait on another.");
    LunaAssert(false, "Not yet implemented");
  }

  auto CommandList::copy(const Buffer& src, const Buffer& dst) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to record a copy buffers operation as an invalid command buffer.");
    this->copy(src, dst, std::min(src.size(), dst.size()));
  }

  auto CommandList::copy(const Buffer& src, const Buffer& dst, std::size_t amt) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to record a copy buffers operation as an invalid command buffer.");
    vulkan::copy_buffer_to_buffer(this->m_handle, src.handle(), dst.handle(), amt);
  }

  auto CommandList::copy(const Buffer& src, const Image& dst) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to record a copy operation as an invalid command buffer.");
    vulkan::copy_buffer_to_image(this->m_handle, src.handle(), dst.handle());
  }

  auto CommandList::copy(const Image& src, const Image& dst) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to record a copy operation as an invalid command buffer.");
    LunaAssert(false, "Not yet implemented");
  }

  auto CommandList::copy(const Image& src, const Buffer& dst) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to record a copy operation as an invalid command buffer.");
    LunaAssert(false, "Not yet implemented");
  }

  auto CommandList::bind(const BindGroup& bind_group) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to record a draw operation as an invalid command buffer.");
    LunaAssert(false, "Not yet implemented");
  }

  auto CommandList::draw(const Buffer& vertices, const Buffer& indices, std::size_t instance_count) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to record a draw operation as an invalid command buffer.");
    LunaAssert(false, "Not yet implemented");
  }

  auto CommandList::draw(const Buffer& vertices, std::size_t instance_count) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to record a draw operation as an invalid command buffer.");
    LunaAssert(false, "Not yet implemented");
  }
}
}