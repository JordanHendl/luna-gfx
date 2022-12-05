#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "luna-gfx/common/shader.hpp"
#include "luna-gfx/vulkan/data_types.hpp"
#include "luna-gfx/vulkan/device.hpp"
namespace luna {
namespace vulkan {
constexpr auto MAX_DESCRIPTORS = 4096;
class Pipeline;
class DescriptorPool;
class Descriptor;
class DescriptorPool {
 public:
  DescriptorPool();
  DescriptorPool(DescriptorPool&& mv);
  ~DescriptorPool();
  auto operator=(DescriptorPool&& mv) -> DescriptorPool&;
  auto initialize(const Pipeline& shader, size_t amount = MAX_DESCRIPTORS)
      -> void;
  auto make() -> int32_t;
  auto update_reference(const Pipeline* ref) -> void { this->m_pipeline = ref; }

 private:
  using UniformMap = std::unordered_map<std::string, gfx::ShaderVariable>;
  friend class Descriptor;
  std::shared_ptr<UniformMap> m_map;
  const Device* m_device;
  const Pipeline* m_pipeline;
  size_t m_device_id;
  size_t m_amount;
  vk::DescriptorPool m_pool;
  vk::DescriptorSetLayout m_layout;
};

class Descriptor {
 public:
  Descriptor();
  Descriptor(Descriptor&& desc);
  Descriptor(DescriptorPool* pool);
  ~Descriptor();
  auto operator=(Descriptor&& desc) -> Descriptor&;
  auto initialize(const DescriptorPool& pool) -> void;
  auto reset() -> void;
  auto bind(std::string_view name, const Image& image) -> void;
  auto bind(std::string_view name, const Image** images, unsigned count)
      -> void;
  auto bind(std::string_view name, const Buffer& buffer) -> void;
  auto initialized() const -> bool { return this->m_set; }
  auto pipeline() const -> const Pipeline& { return *this->m_pipeline; }
  auto set() -> vk::DescriptorSet& { return this->m_set; }
  auto valid() const -> bool {return this->m_set;}
 private:
  using UniformMap = DescriptorPool::UniformMap;
  friend class DescriptorPool;
  vk::DescriptorSet m_set;
  const Device* m_device;
  std::shared_ptr<UniformMap> m_parent_map;
  const Pipeline* m_pipeline;
};
}  // namespace vulkan
}  // namespace luna