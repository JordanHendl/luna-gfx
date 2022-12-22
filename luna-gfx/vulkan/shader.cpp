#include "luna-gfx/vulkan/vulkan_defines.hpp"
#include "luna-gfx/vulkan/shader.hpp"
#include <fstream>
#include <istream>
#include <map>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <iostream>
#include "luna-gfx/error/error.hpp"
#include "luna-gfx/common/shader.hpp"
#include "luna-gfx/vulkan/device.hpp"
#include "luna-gfx/vulkan/global_resources.hpp"

namespace luna {
namespace vulkan {
static inline auto convert(gfx::VariableType type) -> vk::DescriptorType;
static inline auto convert(gfx::ShaderType stage) -> vk::ShaderStageFlagBits;
static inline auto convert(gfx::AttributeType type) -> vk::Format;
static inline auto convert(vk::ShaderStageFlagBits flag) -> gfx::ShaderType;
static inline auto byteSize(gfx::AttributeType type) -> size_t;

auto byteSize(gfx::AttributeType type) -> size_t {
  switch (type) {
    case gfx::AttributeType::eMat4:
      return 4 * 4 * sizeof(float);
    case gfx::AttributeType::eMat3:
      return 3 * 3 * sizeof(float);
    case gfx::AttributeType::eMat2:
      return 2 * 2 * sizeof(float);
    case gfx::AttributeType::eVec4:
      return 4 * sizeof(float);
    case gfx::AttributeType::eVec3:
      return 3 * sizeof(float);
    case gfx::AttributeType::eVec2:
      return 2 * sizeof(float);
    case gfx::AttributeType::eIVec4:
      return 4 * sizeof(int);
    case gfx::AttributeType::eIVec3:
      return 3 * sizeof(int);
    case gfx::AttributeType::eIVec2:
      return 2 * sizeof(int);
    case gfx::AttributeType::eFloat:
      return sizeof(float);
    case gfx::AttributeType::eInt:
      return sizeof(int);
    default:
      return 4;
  }
}

auto convert(gfx::AttributeType type) -> vk::Format {
  switch (type) {
    case gfx::AttributeType::eMat4:
      return vk::Format::eR32G32B32A32Sfloat;
    case gfx::AttributeType::eVec4:
      return vk::Format::eR32G32B32A32Sfloat;
    case gfx::AttributeType::eIVec4:
      return vk::Format::eR32G32B32A32Sint;
    case gfx::AttributeType::eMat3:
      return vk::Format::eR32G32B32Sfloat;
    case gfx::AttributeType::eVec3:
      return vk::Format::eR32G32B32Sfloat;
    case gfx::AttributeType::eIVec3:
      return vk::Format::eR32G32B32Sint;
    case gfx::AttributeType::eMat2:
      return vk::Format::eR32G32Sfloat;
    case gfx::AttributeType::eVec2:
      return vk::Format::eR32G32Sfloat;
    case gfx::AttributeType::eIVec2:
      return vk::Format::eR32G32Sint;
    case gfx::AttributeType::eFloat:
      return vk::Format::eR32Sfloat;
    case gfx::AttributeType::eInt:
      return vk::Format::eR32Sint;
    default:
      return vk::Format::eR32Sfloat;
  }
}

auto convert(gfx::ShaderType stage) -> vk::ShaderStageFlagBits {
  switch (stage) {
    case gfx::ShaderType::Fragment:
      return vk::ShaderStageFlagBits::eFragment;
    case gfx::ShaderType::Geometry:
      return vk::ShaderStageFlagBits::eGeometry;
    case gfx::ShaderType::TesselationControl:
      return vk::ShaderStageFlagBits::eTessellationControl;
    case gfx::ShaderType::TesselationEval:
      return vk::ShaderStageFlagBits::eTessellationEvaluation;
    case gfx::ShaderType::Compute:
      return vk::ShaderStageFlagBits::eCompute;
    case gfx::ShaderType::Vertex:
      return vk::ShaderStageFlagBits::eVertex;
    default:
      return vk::ShaderStageFlagBits::eFragment;
  }
}

auto convert(gfx::VariableType type) -> vk::DescriptorType {
  switch (type) {
    case gfx::VariableType::Uniform:
      return vk::DescriptorType::eUniformBuffer;
      break;
    case gfx::VariableType::Sampler:
      return vk::DescriptorType::eCombinedImageSampler;
      break;
    case gfx::VariableType::Image:
      return vk::DescriptorType::eStorageImage;
      break;
    case gfx::VariableType::Storage:
      return vk::DescriptorType::eStorageBuffer;
      break;
    case gfx::VariableType::None:
      return vk::DescriptorType::eUniformBuffer;
      break;
    default:
      return vk::DescriptorType::eUniformBuffer;
  }
}

auto convert(vk::ShaderStageFlagBits flag) -> gfx::ShaderType {
  switch (flag) {
    case vk::ShaderStageFlagBits::eVertex:
      return gfx::ShaderType::Vertex;
    case vk::ShaderStageFlagBits::eFragment:
      return gfx::ShaderType::Fragment;
    case vk::ShaderStageFlagBits::eCompute:
      return gfx::ShaderType::Compute;
    case vk::ShaderStageFlagBits::eGeometry:
      return gfx::ShaderType::Geometry;
    default:
      return gfx::ShaderType::Vertex;
  }
}

Shader::Shader() { this->m_rate = vk::VertexInputRate::eVertex; }

Shader::Shader(Device& device, gfx::GraphicsPipelineInfo info) {
  this->m_rate = vk::VertexInputRate::eVertex;
  this->m_device = &device;
  this->m_file = std::make_unique<gfx::Shader>(info);

  this->parse();
  this->makeDescriptorLayout();   
  this->makeShaderModules();
  this->makePipelineShaderInfos();
}

Shader::Shader(Device& device, gfx::ComputePipelineInfo info) {
  this->m_rate = vk::VertexInputRate::eVertex;
  this->m_device = &device;
  this->m_file = std::make_unique<gfx::Shader>(info);

  this->parse();
  this->makeDescriptorLayout();
  this->makeShaderModules();
  this->makePipelineShaderInfos();
}

Shader::~Shader() {
  auto device = this->m_device->gpu;
  auto* alloc_cb = this->m_device->allocate_cb;
  auto& dispatch = this->m_device->m_dispatch;

  for (auto module : this->m_modules) {
    device.destroy(module.second, alloc_cb, dispatch);
  }

  if (this->m_layout) device.destroy(this->m_layout, alloc_cb, dispatch);

  this->m_modules.clear();
  this->m_inputs.clear();
  this->m_descriptors.clear();
  this->m_bindings.clear();
  this->m_spirv_map.clear();
  this->m_infos.clear();
}

auto Shader::parse() -> void {
  auto binding_map = std::map<std::string, vk::DescriptorSetLayoutBinding>();
  auto binding = vk::DescriptorSetLayoutBinding();
  auto module_info = vk::ShaderModuleCreateInfo();
  auto attr = vk::VertexInputAttributeDescription();
  auto bind = vk::VertexInputBindingDescription();
  auto offset = 0u;
  for (auto& stage : this->m_file->stages()) {
    if(stage.type == gfx::ShaderType::Vertex) 
    for (auto& attribute : stage.in_attributes) {
      attr.setLocation(attribute.location);
      attr.setBinding(0); //JH TODO fix this
      attr.setFormat(convert(attribute.type));
      attr.setOffset(offset);

      this->m_inputs.push_back(attr);
      offset += byteSize(attribute.type);
    }

    for (auto& variable : stage.variables) {
      auto iter = binding_map.find(variable.first);
      if (iter != binding_map.end()) {
        auto& flags = iter->second.stageFlags;
        flags |= convert(stage.type);
      } else {
        auto& var = variable.second;
        binding.setBinding(var.binding);
        binding.setDescriptorCount(var.size);
        binding.setStageFlags(binding.stageFlags | convert(stage.type));
        binding.setDescriptorType(convert(var.type));
        binding_map.insert(iter, {variable.first, binding});
      }
    }
    module_info.setCodeSize(stage.spirv.size() * sizeof(unsigned));
    module_info.setPCode(stage.spirv.data());
    this->m_spirv_map[convert(stage.type)] = module_info;
  }

  bind.setBinding(0); //JH TODO fix this
  bind.setInputRate(this->m_rate);
  bind.setStride(offset);

  this->m_bindings.push_back(bind);
  this->m_descriptors.reserve(binding_map.size());
  for (auto bind : binding_map) {
    this->m_descriptors.push_back(bind.second);
  }
}

auto Shader::makeDescriptorLayout() -> void {
  auto info = vk::DescriptorSetLayoutCreateInfo();

  info.setBindings(this->m_descriptors);
  this->m_layout = error(this->m_device->gpu.createDescriptorSetLayout(
      info, this->m_device->allocate_cb, this->m_device->m_dispatch));
}

auto Shader::makeShaderModules() -> void {
  auto device = this->m_device->gpu;
  auto* alloc_cb = this->m_device->allocate_cb;
  auto& dispatch = this->m_device->m_dispatch;
  auto mod = vk::ShaderModule();
  this->m_modules.clear();

  for (auto shader : this->m_spirv_map) {
    mod = error(device.createShaderModule(shader.second, alloc_cb, dispatch));
    this->m_modules[convert(shader.first)] = mod;
  }
}

auto Shader::makePipelineShaderInfos() -> void {
  auto info = vk::PipelineShaderStageCreateInfo();
  auto iter = 0u;

  this->m_infos.clear();
  this->m_infos.resize(this->m_modules.size());

  for (auto& it : this->m_modules) {
    info.setStage(convert(it.first));
    info.setModule(it.second);
    info.setPName("main");
    this->m_infos[iter++] = info;
  }
}
}  // namespace vulkan
}  // namespace luna