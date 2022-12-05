#pragma once
#include "luna-gfx/vulkan/vulkan_defines.hpp"
#include "luna-gfx/vulkan/device.hpp"
#include "luna-gfx/interface/render_pass.hpp"
#include "luna-gfx/vulkan/data_types.hpp"
#include "luna-gfx/vulkan/swapchain.hpp"
#include <vulkan/vulkan.hpp>
#include <vector>
#include <optional>
#include <cstdint>
#pragma once
namespace luna {
namespace vulkan {
class Device;
class CommandBuffer;
class Image;
class Swapchain;

class RenderPass {
  public:
    struct Subpass {
      std::string name;
      vk::SubpassDescription desc;
      std::vector<gfx::Attachment> luna_attachments;
      std::vector<vk::AttachmentReference> color;
      std::optional<vk::AttachmentReference> depth;
      std::optional<vk::AttachmentReference> stencil;
      std::vector<std::array<std::int32_t, 3>> images;
    };
    
    RenderPass();
    RenderPass(Device& device, const gfx::RenderPassInfo& info, Swapchain* swap = nullptr);
    RenderPass(RenderPass&& mv);
    ~RenderPass();
    auto operator=(RenderPass&& mv) -> RenderPass&;
    auto bind() -> void;
    auto offset(size_t subpass) const -> size_t;
    inline auto initialized() const -> bool { return this->m_pass; }
    inline auto count() const -> size_t { return this->m_subpasses.size(); }
    inline auto device() -> Device* { return this->m_device; }
    inline auto area() const -> const vk::Rect2D& { return this->m_area; }
    inline auto subpasses() const -> const std::vector<Subpass>& {
      return this->m_subpasses;
    }

    inline auto framebuffers() const -> const std::vector<vk::Framebuffer>& {
      return this->m_framebuffers;
    }
    inline auto pass() const -> const vk::RenderPass& { return this->m_pass; }
    inline auto current() const -> size_t { return this->m_current_framebuffer; }
    inline auto advance() -> void {
      this->m_current_framebuffer++;
      if(this->m_current_framebuffer >= this->m_framebuffers.size()) this->m_current_framebuffer = 0;
    }
    inline auto setCurrent(size_t val) -> void {
      this->m_current_framebuffer = val;
    }

    inline auto valid() const -> bool {return this->m_pass;}
  private:

  
    std::vector<Subpass> m_subpasses;
    std::vector<vk::Framebuffer> m_framebuffers;
    std::vector<vk::AttachmentDescription> m_attachments;
    Device* m_device;
    vk::RenderPass m_pass;
    vk::Rect2D m_area;
    vk::SurfaceKHR m_surface;
    vulkan::Swapchain* m_swap;
    size_t m_current_framebuffer;
    size_t m_num_binded_subpasses;

    auto add_subpass(const gfx::Subpass& subpass) -> void;
    auto parse_info(const gfx::RenderPassInfo& info) -> void;
    auto make_render_pass() -> void;
    auto make_images() -> void;
};
}  // namespace vulkan
}  // namespace luna