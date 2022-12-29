#include "luna-gfx/interface/command_list.hpp"
#include "luna-gfx/interface/buffer.hpp"
#include "luna-gfx/interface/image.hpp"
#include "luna-gfx/interface/window.hpp"
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

  auto CommandList::viewport(const Viewport& view) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to begin an invalid command buffer.");
    auto& res = vulkan::global_resources();
    auto& cmd = res.cmds[this->m_handle];
    auto& gpu = res.devices[cmd.gpu];

    auto vp = vk::Viewport();
    auto sc = vk::Rect2D();

    vp.width = view.width;
    vp.height = view.height;
    vp.minDepth = 0;
    vp.maxDepth = view.max_depth;
    sc.extent.width = view.width;
    sc.extent.height = view.height;
    cmd.cmd.setViewport(0, vp, gpu.m_dispatch);
    cmd.cmd.setScissor(0, sc, gpu.m_dispatch);
  }

  auto CommandList::start_draw(const RenderPass& pass, int buffer_layer) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to begin an invalid command buffer.");
    vulkan::cmd_start_render_pass(this->m_handle, pass.handle(), buffer_layer);
  }

  auto CommandList::end_draw() -> void {
    LunaAssert(this->m_handle >= 0, "Unable to begin an invalid command buffer.");
    vulkan::cmd_end_render_pass(this->m_handle);
  }

  auto CommandList::begin() -> void {
    LunaAssert(this->m_handle >= 0, "Unable to begin an invalid command buffer.");
    vulkan::begin_command_buffer(this->m_handle);
  }

  auto CommandList::end() -> void {
    LunaAssert(this->m_handle >= 0, "Unable to end an invalid command buffer.");
    vulkan::end_command_buffer(this->m_handle);
  }

  auto CommandList::submit() -> std::future<bool> {
    LunaAssert(this->m_handle >= 0, "Unable to submit an invalid command buffer.");
    vulkan::submit_command_buffer(this->m_handle);
    auto wait_func = [](std::int32_t handle) {
      vulkan::synchronize_cmd(handle);
      return true;
    };

    return std::async(std::launch::deferred, wait_func, this->m_handle);
  }

  auto CommandList::combo_into(const Window& window) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to tell an invalid command buffer to wait on another.");
    auto& res = vulkan::global_resources();
    auto& this_cmd = res.cmds[this->handle()];
    auto& other_swap = res.swapchains[window.handle()];
    // Grab sem ownership.
    auto sem = vulkan::create_semaphores(this_cmd.gpu, 1);

    // Set it to us to signal.
    this_cmd.sems_to_signal.push_back(sem[0]);

    // Set it to the input cmd to wait on (and eventually release).
    other_swap.m_sems_to_wait_on.push_back(sem[0]);
  }

  auto CommandList::combo_into(const CommandList& cmd) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to tell an invalid command buffer to wait on another.");
    auto& res = vulkan::global_resources();
    auto& this_cmd = res.cmds[this->handle()];
    auto& other_cmd = res.cmds[cmd.handle()];
    // Grab sem ownership.
    auto sem = vulkan::create_semaphores(this_cmd.gpu, 1);

    // Set it to us to signal.
    this_cmd.sems_to_signal.push_back(sem[0]);

    // Set it to the input cmd to wait on (and eventually release).
    other_cmd.sems_to_wait_on.push_back(sem[0]);
  }

  auto CommandList::copy(const MemoryBuffer& src, const MemoryBuffer& dst) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to record a copy buffers operation as an invalid command buffer.");
    this->copy(src, dst, std::min(src.size(), dst.size()));
  }

  auto CommandList::copy(const MemoryBuffer& src, const MemoryBuffer& dst, std::size_t amt) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to record a copy buffers operation as an invalid command buffer.");
    vulkan::copy_buffer_to_buffer(this->m_handle, src.handle(), dst.handle(), amt);
  }

  auto CommandList::copy(const MemoryBuffer& src, const Image& dst) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to record a copy operation as an invalid command buffer.");
    vulkan::copy_buffer_to_image(this->m_handle, src.handle(), dst.handle());
  }

  auto CommandList::copy(const Image& src, const Image& dst) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to record a copy operation as an invalid command buffer.");
    LunaAssert(false, "Not yet implemented");
  }

  auto CommandList::copy(const Image& src, const MemoryBuffer& dst) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to record a copy operation as an invalid command buffer.");
    LunaAssert(false, "Not yet implemented");
  }

  auto CommandList::bind(const BindGroup& bind_group) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to record a draw operation as an invalid command buffer.");
    luna::vulkan::cmd_bind_descriptor(this->m_handle, bind_group.handle());
  }

  auto CommandList::draw(const MemoryBuffer& vertices, std::size_t num_verts, const MemoryBuffer& indices, std::size_t num_indices, std::size_t instance_count) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to record a draw operation as an invalid command buffer.");
    luna::vulkan::cmd_buffer_draw(this->m_handle, vertices.handle(), num_verts, indices.handle(), num_indices, instance_count);
  }

  auto CommandList::draw(const MemoryBuffer& vertices, std::size_t num_verts, std::size_t instance_count) -> void {
    LunaAssert(this->m_handle >= 0, "Unable to record a draw operation as an invalid command buffer.");
    luna::vulkan::cmd_buffer_draw(this->m_handle, vertices.handle(), num_verts, instance_count);
  }
 
  auto CommandList::start_time_stamp() -> void {
    LunaAssert(this->m_handle >= 0, "Unable to record a time stamp start operation as an invalid command buffer.");
    vulkan::start_timestamp(this->m_handle, vk::PipelineStageFlagBits::eTopOfPipe);
  }

  auto CommandList::end_time_stamp() -> std::future<std::chrono::duration<double, std::nano>> {
    LunaAssert(this->m_handle >= 0, "Unable to end a time stamp operation as an invalid command buffer.");
    vulkan::end_timestamp(this->m_handle, vk::PipelineStageFlagBits::eBottomOfPipe);
    auto read_func = [](std::int32_t handle) {
      return vulkan::read_timestamp(handle);
    };

    return std::async(std::launch::deferred, read_func, this->m_handle);
  }
}
}