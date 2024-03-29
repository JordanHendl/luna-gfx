#pragma once
#include "luna-gfx/interface/buffer.hpp"
#include <memory>
#include <cstddef>
#include <cstdint>
#include <chrono>
#include <future>

namespace luna {
namespace gfx {
class BindGroup;
class MemoryBuffer;
class Image;
class RenderPass;
class Window;
struct Viewport;
class Window;
enum class Queue {
  All,
  Graphics,
  Compute,
  Transfer
};
class CommandList {
  public:
    CommandList(const CommandList& cpy) = delete;
    auto operator=(const CommandList& cpy) -> CommandList& = delete;

    CommandList() {this->m_handle = -1;}
    CommandList(int gpu, Queue queue = Queue::All);
    CommandList(int gpu, CommandList& parent);
    CommandList(CommandList&& mv) {*this = std::move(mv);};
    ~CommandList();

    auto begin() -> void;
    auto end() -> void;
    auto start_draw(const RenderPass& pass, int buffer_layer = 0) -> void;
    auto end_draw() -> void; 

    // Returns a future that is ready when the submit is finished executing on the gpu.
    [[nodiscard]] auto submit() -> std::future<bool>;
    auto combo_into(const Window& window) -> void;
    auto combo_into(const CommandList& cmd) -> void;
    auto start_time_stamp() -> void;

    // Returns a future that returns the time it took for the GPU to perform any of the in-between actions.
    [[nodiscard]] auto end_time_stamp() -> std::future<std::chrono::duration<double, std::nano>>;

    auto barrier() -> void;
    auto flush() -> void;
    auto viewport(const Viewport& view) -> void;

    template<typename T>
    auto copy(const Vector<T>& src, const Vector<T>& dst) -> void {this->copy(src.buffer(), dst.buffer());}

    template<typename T>
    auto copy(const Vector<T>& src, const Vector<T>& dst, std::size_t amt) -> void {this->copy(src.buffer(), dst.buffer(), amt);}
    auto copy(const MemoryBuffer& src, const MemoryBuffer& dst) -> void;
    auto copy(const MemoryBuffer& src, const MemoryBuffer& dst, std::size_t amt) -> void;

    template<typename T>
    auto copy(const Vector<T>& src, const Image& dst) -> void {this->copy(src.buffer(), dst);}

    auto copy(const MemoryBuffer& src, const Image& dst) -> void;
    auto copy(const Image& src, const Image& dst) -> void;

    template<typename T>
    auto copy(const Image& src, const Vector<T>& dst) -> void {this->copy(src, dst.buffer());}

    auto copy(const Image& src, const MemoryBuffer& dst) -> void;
    auto bind(const BindGroup& bind_group) -> void;

    template<typename V, typename T>
    auto draw(const Vector<V>& vertices, const Vector<T>& indices, std::size_t instance_count = 1) -> void {
      this->draw(vertices.buffer(), vertices.buffer().size()/sizeof(V), indices.buffer(), indices.buffer().size()/sizeof(T), instance_count); 
    }

    template<typename V>
    auto draw(const Vector<V>& vertices, std::size_t instance_count = 1) -> void {
      this->draw(vertices.buffer(), vertices.buffer().size()/sizeof(V), instance_count); 
    }

    auto draw(const MemoryBuffer& vertices, std::size_t num_verts, const MemoryBuffer& indices, std::size_t num_indices, std::size_t instance_count = 1) -> void;
    auto draw(const MemoryBuffer& vertices, std::size_t num_verts, std::size_t instance_count = 1) -> void;

    auto dispatch(std::size_t group_amt_x, std::size_t group_amt_y = 1, std::size_t group_amt_z = 1) -> void;
    auto next_subpass() -> void;
    
    [[nodiscard]] auto queue() const {return this->m_type;}
    [[nodiscard]] auto handle() const {return this->m_handle;}
    auto operator=(CommandList&& mv) -> CommandList& {this->m_handle = mv.handle(); mv.m_handle = -1; return *this;};
  private:
    std::int32_t m_handle;
    Queue m_type;
};
}
}