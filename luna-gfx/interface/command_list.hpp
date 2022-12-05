#pragma once
#include <memory>
#include <cstddef>
#include <cstdint>
namespace luna {
namespace gfx {
class BindGroup;
class Buffer;
class Image;
enum class Queue {
  All,
  Graphics,
  Compute,
  Transfer
};
class CommandList {
  public:
    CommandList() {this->m_handle = -1;}
    CommandList(int gpu, Queue queue = Queue::All);
    CommandList(int gpu, CommandList& parent);
    CommandList(CommandList&& mv) {*this = std::move(mv);};
    CommandList(const CommandList& cpy) = delete;
    ~CommandList();

    auto begin() -> void;
    auto end() -> void;
    auto submit() -> void;
    auto synchronize() -> void;
    auto wait_on(const CommandList& cmd) -> void;
    auto copy(const Buffer& src, const Buffer& dst) -> void;
    auto copy(const Buffer& src, const Buffer& dst, std::size_t amt) -> void;
    auto copy(const Buffer& src, const Image& dst) -> void;
    auto copy(const Image& src, const Image& dst) -> void;
    auto copy(const Image& src, const Buffer& dst) -> void;
    auto bind(const BindGroup& bind_group) -> void;
    auto draw(const Buffer& vertices, const Buffer& indices, std::size_t instance_count = 1) -> void;
    auto draw(const Buffer& vertices, std::size_t instance_count = 1) -> void;

    [[nodiscard]] auto queue() const {return this->m_type;}
    [[nodiscard]] auto handle() const {return this->m_handle;}
    auto operator=(CommandList&& mv) -> CommandList& {this->m_handle = mv.handle(); mv.m_handle = -1; return *this;};
    auto operator=(const CommandList& cpy) -> CommandList& = delete;
  private:
    std::int32_t m_handle;
    Queue m_type;
};
}
}