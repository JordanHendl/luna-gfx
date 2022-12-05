#include "luna-gfx/interface/pipeline.hpp"
#include "luna-gfx/vulkan/utils/helper_functions.hpp"
namespace luna {
namespace gfx {
  GraphicsPipeline::GraphicsPipeline(const RenderPass& pass, GraphicsPipelineInfo info) {
    this->m_handle = luna::vulkan::create_graphics_pipeline(pass.handle(), info);
    this->m_info = info;
  }

  GraphicsPipeline::~GraphicsPipeline() {
    luna::vulkan::destroy_pipeline(this->m_handle);
    this->m_info = {};
  }

  auto GraphicsPipeline::create_bind_group() -> BindGroup {
    auto tmp = BindGroup();
    tmp.m_handle = luna::vulkan::create_bind_group(this->m_handle);
    return tmp;
  }

  auto GraphicsPipeline::update_viewport(Viewport view) -> void {
    luna::vulkan::update_view_port(this->m_handle, view);
  }
}
}