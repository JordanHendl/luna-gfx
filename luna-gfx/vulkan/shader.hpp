#pragma once
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <vulkan/vulkan.hpp>
#include "luna-gfx/interface/pipeline.hpp"
#include "luna-gfx/common/shader.hpp"
#include "luna-gfx/vulkan/device.hpp"
#include <utility>

namespace luna {
namespace vulkan {
struct Device;
class Shader {
 public:
  enum class InputRate {
    Vertex,
    Instanced,
  };

  Shader();
  Shader(const Shader& shader);
  Shader(Device& device, gfx::GraphicsPipelineInfo info);
  Shader(Device& device, gfx::ComputePipelineInfo info);
  Shader(Shader&& mv);
  ~Shader();
  Shader& operator=(Shader&& mv);
  inline auto file() const -> const gfx::Shader& { return *this->m_file; }
  inline auto device() const -> const Device& { return *this->m_device; }
  inline auto layout() const -> const vk::DescriptorSetLayout& {
    return this->m_layout;
  }
  inline auto inputs() const
      -> const std::vector<vk::VertexInputAttributeDescription>& {
    return this->m_inputs;
  }
  inline auto bindings() const
      -> const std::vector<vk::VertexInputBindingDescription>& {
    return this->m_bindings;
  }
  inline auto shaderInfos() const
      -> const std::vector<vk::PipelineShaderStageCreateInfo>& {
    return this->m_infos;
  }
  inline auto descriptorLayouts() const
      -> const std::vector<vk::DescriptorSetLayoutBinding>& {
    return this->m_descriptors;
  }
 private:
  using SPIRVMap =
      std::map<vk::ShaderStageFlagBits, vk::ShaderModuleCreateInfo>;
  using ShaderModules = std::map<gfx::Shader::Type, vk::ShaderModule>;
  using Attributes = std::vector<vk::VertexInputAttributeDescription>;
  using Bindings = std::vector<vk::VertexInputBindingDescription>;
  using Infos = std::vector<vk::PipelineShaderStageCreateInfo>;
  using Descriptors = std::vector<vk::DescriptorSetLayoutBinding>;

  ShaderModules m_modules;
  Descriptors m_descriptors;
  SPIRVMap m_spirv_map;
  Attributes m_inputs;
  Bindings m_bindings;
  Infos m_infos;
  std::unique_ptr<gfx::Shader> m_file;
  Device* m_device;
  vk::DescriptorSetLayout m_layout;
  vk::PipelineVertexInputStateCreateInfo m_info;
  vk::VertexInputRate m_rate;

  inline auto parse() -> void;
  inline auto makeDescriptorLayout() -> void;
  inline auto makeShaderModules() -> void;
  inline auto makePipelineShaderInfos() -> void;
};
}  // namespace vulkan
}  // namespace luna