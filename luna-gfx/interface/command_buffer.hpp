#pragma once
#include "luna/graphics/impl/backend.hpp"
#include "luna/graphics/impl/descriptor.hpp"
#include "luna/graphics/impl/render_pass.hpp"

#include <memory>
#include "cstddef"
namespace luna {
namespace gfx {
  class CommandBuffer {
    public:
      CommandBuffer() {this->m_handle = -1;}
      CommandBuffer(int gpu, int32_t parent = -1) {
        this->m_handle = impl().cmd.make(gpu, parent);
      };

      ~CommandBuffer() {
        if(this->m_handle >= 0) {
          impl().cmd.destroy(this->m_handle);
        }
      }

      CommandBuffer(CommandBuffer&& mv) {*this = std::move(mv);};
      CommandBuffer(const CommandBuffer& cpy) = delete;

      auto set_descriptor(const Descriptor& desc) -> void {
        impl().cmd.bind_descriptor(this->m_handle, desc.handle());
      }

      auto handle() const {return this->m_handle;}
      auto operator=(CommandBuffer&& mv) -> CommandBuffer& {this->m_handle = mv.handle(); mv.m_handle = -1; return *this;};
      auto operator=(const CommandBuffer& cpy) -> CommandBuffer& = delete;
    private:
      std::int32_t m_handle;
  };
}
}