#pragma once
#include <cstdint>
#include <array>
namespace luna {
namespace gfx {
/** Helper class for multi-buffered data.
 * Detail:
 * The main purpose of a class like this is that when doing work on the GPU,
 * there are many times you have 'frames in flight' and need multi-layered data
 * that you only operate on the current 'valid' frame at a time. 
 */
template<typename T, std::size_t amt = 3>
class MultiBuffered {
  public:
    MultiBuffered() = default;

    template<typename ... Args>
    MultiBuffered(Args ... args) {for(auto& t : this->m_data) t = std::move(T(args...)); this->m_curr = 0;}

    MultiBuffered(MultiBuffered&& o) : m_curr(o.m_curr), m_data(std::move(o.m_data)) {};

    ~MultiBuffered() = default;

    auto advance() -> std::size_t {this->m_curr += 1; if(this->m_curr >= amt) this->m_curr = 0; return this->m_curr;}
    [[nodiscard]] constexpr auto layers() const {return amt;};
    [[nodiscard]] auto operator=(MultiBuffered&& o) -> MultiBuffered& {this->m_data = std::move(o.m_data); this->m_curr = o.m_curr; return *this;}
    [[nodiscard]] auto operator->() const -> const T* {return &this->m_data[this->m_curr];}
    [[nodiscard]] auto operator->() -> T* {return &this->m_data[this->m_curr];}
    [[nodiscard]] auto operator*() const -> const T& {return this->m_data[this->m_curr];}
    [[nodiscard]] auto operator*() -> T& {return this->m_data[this->m_curr];}
  private:
    std::size_t m_curr;
    std::array<T, amt> m_data;
};
}
}