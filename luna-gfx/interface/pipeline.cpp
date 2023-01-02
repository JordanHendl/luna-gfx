#include "luna-gfx/interface/pipeline.hpp"
#include "luna-gfx/vulkan/utils/helper_functions.hpp"
namespace luna {
namespace gfx {
  ComputePipeline::ComputePipeline(ComputePipelineInfo info) {
    this->m_handle = luna::vulkan::create_compute_pipeline(info);
    this->m_info = info;
  }

  ComputePipeline::~ComputePipeline() {
    if(this->m_handle < 0) return; // Don't deconstruct an invalid handle!
    luna::vulkan::destroy_pipeline(this->m_handle);
    this->m_info = {};
  }

  auto ComputePipeline::create_bind_group() -> BindGroup {
    auto tmp = BindGroup();
    tmp.m_handle = luna::vulkan::create_bind_group(this->m_handle);
    return tmp;
  }

  GraphicsPipeline::GraphicsPipeline(const RenderPass& pass, GraphicsPipelineInfo info) {
    this->m_handle = luna::vulkan::create_graphics_pipeline(pass.handle(), info);
    this->m_info = info;
  }

  GraphicsPipeline::~GraphicsPipeline() {
    if(this->m_handle < 0) return; // Don't deconstruct an invalid handle!
    luna::vulkan::destroy_pipeline(this->m_handle);
    this->m_info = {};
  }

  auto GraphicsPipeline::create_bind_group() -> BindGroup {
    auto tmp = BindGroup();
    tmp.m_handle = luna::vulkan::create_bind_group(this->m_handle);
    return tmp;
  }
}
}