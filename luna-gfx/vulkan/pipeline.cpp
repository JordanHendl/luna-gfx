#include "luna-gfx/vulkan/vulkan_defines.hpp"
#include "luna-gfx/vulkan/pipeline.hpp"
#include <cstdio>
#include <utility>
#include <vulkan/vulkan.hpp>
#include "luna-gfx/vulkan/device.hpp"
#include "luna-gfx/vulkan/render_pass.hpp"
#include "luna-gfx/vulkan/shader.hpp"
#include "luna-gfx/error/error.hpp"
namespace luna {
namespace vulkan {

inline auto convert(gfx::Topology topology) -> vk::PrimitiveTopology {
  switch (topology) {
    case gfx::Topology::Triangle:
      return vk::PrimitiveTopology::eTriangleList;
    case gfx::Topology::TriangleStrip:
      return vk::PrimitiveTopology::eTriangleStrip;
    default:
      return vk::PrimitiveTopology::eTriangleList;
  }
}

inline auto create_dynamic_state() {
  return std::vector<vk::DynamicState> {
    vk::DynamicState::eViewport,
    vk::DynamicState::eScissor
  };
}
Pipeline::Pipeline() {}

Pipeline::~Pipeline() {
  if (this->m_pipeline)
    this->m_device->gpu.destroy(this->m_pipeline,
                                     this->m_device->allocate_cb,
                                     this->m_device->m_dispatch);
  if (this->m_layout)
    this->m_device->gpu.destroy(this->m_layout,
                                     this->m_device->allocate_cb,
                                     this->m_device->m_dispatch);
}

Pipeline::Pipeline(Device& device, const gfx::ComputePipelineInfo& info) {
  this->m_device = &device;

  this->m_shader = std::make_unique<Shader>(device, info);
  this->m_pool.initialize(*this);
  this->init_params();
  this->createLayout();
  this->createPipeline();
}

Pipeline::Pipeline(RenderPass& pass, const gfx::GraphicsPipelineInfo& info)
{
  auto num_framebuffers = 0;
  auto subpass_index = 0;
  for(auto& subpass : pass.subpasses()) {
    if(subpass.name == info.subpass) {
      num_framebuffers = subpass.color.size();
      this->m_subpass_id = subpass_index;
    }
    subpass_index++;
  }

  this->m_color_blend_attachments.resize(num_framebuffers);
  this->init_params();
  this->m_device      = pass.device() ;
  this->m_render_pass = &pass          ;
  this->m_shader = std::make_unique<Shader>(*this->m_device, info);

  this->m_pool.initialize( *this ) ;

  this->parse(info) ;
  this->createLayout();
  this->createPipeline();
}

Pipeline::Pipeline(Pipeline&& mv) { *this = std::move(mv); }

auto Pipeline::operator=(Pipeline&& mv) -> Pipeline& {
  this->m_render_pass = mv.m_render_pass;
  this->m_scissors = mv.m_scissors;
  this->m_viewports = mv.m_viewports;
  this->m_device = mv.m_device;
  this->m_pipeline = mv.m_pipeline;
  this->m_layout = mv.m_layout;
  this->m_cache = mv.m_cache;
  this->m_push_constant_flags = mv.m_push_constant_flags;
  this->m_push_constant_size = mv.m_push_constant_size;
  this->m_viewport_info = mv.m_viewport_info;
  this->m_color_blend_info = mv.m_color_blend_info;
  this->m_rasterization_info = mv.m_rasterization_info;
  this->m_assembly_info = mv.m_assembly_info;
  this->m_multisample_info = mv.m_multisample_info;
  this->m_depth_stencil_info = mv.m_depth_stencil_info;
  this->m_sample_mask = mv.m_sample_mask;
  this->m_color_blend_attachments = mv.m_color_blend_attachments;

  mv.m_render_pass = nullptr;
  mv.m_device = nullptr;
  mv.m_pipeline = nullptr;
  mv.m_layout = nullptr;

  this->m_pool = std::move(mv.m_pool);
  this->m_pool.update_reference(this);
  this->m_shader = std::move(mv.m_shader);
  return *this;
}

auto Pipeline::init_params() -> void {
  this->m_render_pass = nullptr;
  this->m_push_constant_size = 128;
  this->m_push_constant_flags = vk::ShaderStageFlagBits::eVertex |
                                vk::ShaderStageFlagBits::eFragment |
                                vk::ShaderStageFlagBits::eCompute;

  vk::ColorComponentFlags color_blend_mask;

  color_blend_mask =
      (vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
       vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
  this->m_sample_mask = 0xFFFFFFFF;
  this->m_rasterization_info.setDepthClampEnable(false);
  this->m_rasterization_info.setRasterizerDiscardEnable(false);
  this->m_rasterization_info.setPolygonMode(vk::PolygonMode::eFill);
  this->m_rasterization_info.setLineWidth(1.0f);
  this->m_rasterization_info.setCullMode(vk::CullModeFlagBits::eNone);
  this->m_rasterization_info.setFrontFace(vk::FrontFace::eCounterClockwise);
  this->m_rasterization_info.setDepthBiasEnable(false);
  this->m_rasterization_info.setDepthBiasConstantFactor(0.0f);
  this->m_rasterization_info.setDepthBiasClamp(0.0f);
  this->m_rasterization_info.setDepthBiasSlopeFactor(0.0f);

  this->m_multisample_info.setSampleShadingEnable(false);
  this->m_multisample_info.setMinSampleShading(1.0f);
  this->m_multisample_info.setAlphaToOneEnable(false);
  this->m_multisample_info.setAlphaToCoverageEnable(false);
  this->m_multisample_info.setPSampleMask(&this->m_sample_mask);
  this->m_multisample_info.setRasterizationSamples(vk::SampleCountFlagBits::e1);

  for(auto& color : this->m_color_blend_attachments) {
    color.setColorWriteMask(color_blend_mask);
    color.setBlendEnable(false);
    color.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
    color.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
    color.setColorBlendOp(vk::BlendOp::eAdd);
    color.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
    color.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
    color.setAlphaBlendOp(vk::BlendOp::eAdd);
  }
  
  this->m_color_blend_info.setLogicOpEnable(false);
  this->m_color_blend_info.setLogicOp(vk::LogicOp::eCopy);

  this->m_viewport_info.setViewportCount(1);
  this->m_viewport_info.setScissorCount(1);

  this->m_assembly_info.setTopology(vk::PrimitiveTopology::eTriangleList);
  this->m_assembly_info.setPrimitiveRestartEnable(false);
}

auto Pipeline::createLayout() -> void {
  auto info = vk::PipelineLayoutCreateInfo();
  auto range = vk::PushConstantRange();
  auto desc_layout = vk::DescriptorSetLayout();

  desc_layout = this->m_shader->layout();

  this->m_color_blend_info.setAttachments(this->m_color_blend_attachments);

  range.setOffset(0);
  range.setSize(this->m_push_constant_size);
  range.setStageFlags(this->m_push_constant_flags);

  info.setSetLayoutCount(1);
  info.setPSetLayouts(&desc_layout);
  info.setPushConstantRangeCount(1);
  info.setPPushConstantRanges(&range);

  this->m_layout = error(this->m_device->gpu.createPipelineLayout(
      info, this->m_device->allocate_cb, this->m_device->m_dispatch));
}

auto Pipeline::createPipeline() -> void {
  auto graphics_info = vk::GraphicsPipelineCreateInfo();
  auto compute_info = vk::ComputePipelineCreateInfo();
  auto vertex_input = vk::PipelineVertexInputStateCreateInfo();

  auto device = this->m_device->gpu;
  auto* alloc_cb = this->m_device->allocate_cb;
  auto& dispatch = this->m_device->m_dispatch;
  auto dynamic_state = vk::PipelineDynamicStateCreateInfo();
  auto tmp = create_dynamic_state();
  dynamic_state.setDynamicStates(tmp);

  if (this->graphics()) {
    vertex_input.setVertexAttributeDescriptions(this->m_shader->inputs());
    vertex_input.setVertexBindingDescriptions(this->m_shader->bindings());
    this->m_viewport_info.setViewports(this->m_viewports);
    this->m_viewport_info.setScissors(this->m_scissors);
    graphics_info.setPDynamicState(&dynamic_state);
    graphics_info.setStages(this->m_shader->shaderInfos());
    graphics_info.setLayout(this->m_layout);
    graphics_info.setPVertexInputState(&vertex_input);
    graphics_info.setPInputAssemblyState(&this->m_assembly_info);
    graphics_info.setPViewportState(&this->m_viewport_info);
    graphics_info.setPRasterizationState(&this->m_rasterization_info);
    graphics_info.setPMultisampleState(&this->m_multisample_info);
    graphics_info.setPColorBlendState(&this->m_color_blend_info);
    graphics_info.setPDepthStencilState(&this->m_depth_stencil_info);
    graphics_info.setRenderPass(this->m_render_pass->pass());
    graphics_info.setSubpass(this->m_subpass_id);
    this->m_pipeline = error(device.createGraphicsPipeline(
        this->m_cache, graphics_info, alloc_cb, dispatch));
  } else {
    compute_info.setLayout(this->m_layout);
    compute_info.setStage(this->m_shader->shaderInfos()[0]);

    this->m_pipeline = error(device.createComputePipeline(
        this->m_cache, compute_info, alloc_cb, dispatch));
  }
}

auto Pipeline::addViewport(const gfx::Viewport& viewport) -> void {
  auto view = vk::Viewport();
  auto scissor = vk::Rect2D();

  view.setWidth(viewport.width);
  view.setHeight(viewport.height);
  view.setMinDepth(0.f);
  view.setMaxDepth(viewport.max_depth);
  view.setX(0);
  view.setY(0);

  scissor.setExtent({static_cast<uint32_t>(viewport.width),
                     static_cast<uint32_t>(viewport.height)});
  scissor.setOffset({0, 0});

  this->m_viewports.push_back(view);
  this->m_scissors.push_back(scissor);
}

auto Pipeline::parse(const gfx::GraphicsPipelineInfo& info) -> void {
  if (info.details.depth_test) {
    this->m_depth_stencil_info.setDepthTestEnable(true);
    this->m_depth_stencil_info.setDepthWriteEnable(true);
    this->m_depth_stencil_info.setDepthCompareOp(vk::CompareOp::eLess);
    this->m_depth_stencil_info.setDepthBoundsTestEnable(false);
    this->m_depth_stencil_info.setMinDepthBounds(0.0f);
    this->m_depth_stencil_info.setMaxDepthBounds(1.0f);
  }

  this->m_assembly_info.setTopology(convert(info.details.topology));
  this->addViewport(info.initial_viewport);
  
  // @JH TODO add more config to parse.
}

auto Pipeline::descriptor() -> int32_t { return this->m_pool.make(); }
}  // namespace vulkan
}  // namespace luna