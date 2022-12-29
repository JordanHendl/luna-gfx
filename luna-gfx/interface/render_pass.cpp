#include "luna-gfx/interface/render_pass.hpp"
#include "luna-gfx/interface/window.hpp"
#include "luna-gfx/vulkan/global_resources.hpp"
#include "luna-gfx/vulkan/render_pass.hpp"
#include <utility>
namespace luna {
namespace gfx {

  FramebufferCreator::FramebufferCreator(int gpu, std::size_t width, std::size_t height, std::unordered_map<std::string, gfx::ImageFormat> requested_framebuffers) {
    auto info = gfx::ImageInfo();
    info.width = width;
    info.height = height;

    for(auto& attachment : requested_framebuffers) {
      info.name = attachment.first;
      info.format = attachment.second;

      for(auto i = 0; i < 3; i++) {
        this->m_images.push_back(gfx::Image(info));
      }
    }
  }

  auto FramebufferCreator::views() -> std::unordered_map<std::string, std::vector<gfx::ImageView>> {
    auto map = std::unordered_map<std::string, std::vector<gfx::ImageView>>();
    for(auto& img : this->m_images) {
      map[img.info().name].push_back(img);
    }
    return map;
  }

  RenderPass::RenderPass(RenderPassInfo info) {
    LunaAssert(info.gpu >= 0, "Cannot accept a negative gpu value.");
    auto& res = vulkan::global_resources();
    auto& gpu = res.devices[info.gpu];
    auto index = vulkan::find_valid_entry(res.render_passes);
    res.render_passes[index] = std::move(luna::vulkan::RenderPass(gpu, info));
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