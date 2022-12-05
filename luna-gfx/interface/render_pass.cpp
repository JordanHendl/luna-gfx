#include "luna-gfx/interface/render_pass.hpp"
#include "luna-gfx/interface/window.hpp"
#include "luna-gfx/vulkan/global_resources.hpp"
#include "luna-gfx/vulkan/render_pass.hpp"
#include <utility>
namespace luna {
namespace gfx {
  RenderPass::RenderPass(RenderPassInfo info) {
    LunaAssert(info.gpu >= 0, "Cannot accept a negative gpu value.");
    auto& res = vulkan::global_resources();
    auto& gpu = res.devices[info.gpu];
    auto index = vulkan::find_valid_entry(res.render_passes);
    res.render_passes[index] = std::move(luna::vulkan::RenderPass(gpu, info));
    this->m_handle = index;
    this->m_info = info;
  }
  
  RenderPass::RenderPass(RenderPassInfo info, Window& window) {
    LunaAssert(info.gpu >= 0, "Cannot accept a negative gpu value.");
    LunaAssert(window.handle() >= 0, "Cannot use an invalid window for render pass.");
    auto& res = vulkan::global_resources();
    auto& gpu = res.devices[info.gpu];
    auto index = vulkan::find_valid_entry(res.render_passes);
    res.render_passes[index] = std::move(luna::vulkan::RenderPass(gpu, info, &res.swapchains[window.handle()]));
    this->m_handle = index;
    this->m_info = info;
  }

  RenderPass::~RenderPass() {
    if(this->m_handle < 0) return;

    auto& res = vulkan::global_resources();
    auto rp = std::move(res.render_passes[this->m_handle]);
    this->m_handle = -1;
    this->m_info = {};
  }
}
}