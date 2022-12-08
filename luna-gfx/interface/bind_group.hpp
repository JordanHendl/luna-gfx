#pragma once
#include "luna-gfx/interface/buffer.hpp"
#include <memory>
#include <cstddef>
#include <cstdint>
#include <string_view>
namespace luna {
namespace gfx {
class GraphicsPipeline;
class ComputePipeline;
class MemoryBuffer;
class Image;
class BindGroup {
  public:
    BindGroup() {this->m_handle = -1;}
    ~BindGroup();
    BindGroup(BindGroup&& mv) {*this = std::move(mv);};
    BindGroup(const BindGroup& cpy) = delete;
    template<typename T>
    auto set(const Vector<T>& buffer, std::string_view str) -> bool {return this->set(buffer.memory(), str);}
    auto set(const MemoryBuffer& buffer, std::string_view str) -> bool;
    auto set(const Image& image, std::string_view str) -> bool;
    [[nodiscard]] inline auto handle() const -> std::int32_t {return this->m_handle;}
    auto operator=(BindGroup&& mv) -> BindGroup& {this->m_handle = mv.m_handle; mv.m_handle = -1; return *this;};
    auto operator=(const BindGroup& cpy) -> BindGroup& = delete;
  private:
    friend class GraphicsPipeline;
    friend class ComputePipeline;
    std::int32_t m_handle;
};
}
}