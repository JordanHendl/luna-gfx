#pragma once
#include <cstdint>
#include "luna-gfx/ext/math.hpp" // Units from math.hpp
namespace luna {
namespace gfx {

template<typename Type>
class Camera {
  public:
    Camera() = default;
    auto translate(vec3 offsets) -> void {this->impl.translate(offsets);};
    auto rotate_euler(float x, float y = 0.0f, float z = 0.0f) -> void {this->impl.rotate_euler(x, y, z);}
    auto look_at(vec3 position) -> void {this->impl.look_at(position);}
    [[nodiscard]] auto up() const -> vec3 {return this->impl.up();}
    [[nodiscard]] auto right() const -> vec3 {return this->impl.right();}
    [[nodiscard]] auto view_matrix() const -> const mat4& {return this->impl.view_matrix();}
  private:
    Type impl;
};

namespace camera_impl {
class Orthographic {
  public:
    Orthographic();
    ~Orthographic() = default;
    auto translate(vec3 offsets) -> void;
    auto rotate_euler(float x, float y = 0.0f, float z = 0.0f) -> void;
    auto look_at(vec3 position) -> void;
    auto up() const -> vec3 {return this->m_up;}
    auto right() const -> vec3 {return this->m_right;}
    [[nodiscard]] auto view_matrix() const -> const mat4& {return this->m_transform;}
  private:
    auto update() -> void;
    mat4 m_transform;
    vec3 m_position;
    vec3 m_front;
    vec3 m_up;
    vec3 m_right;
};

class Perspective {
  public:
    Perspective();
    ~Perspective() = default;
    auto translate(vec3 offsets) -> void;
    auto rotate_euler(float x, float y = 0.0f, float z = 0.0f) -> void;
    auto look_at(vec3 position) -> void;
    auto up() const -> vec3 {return this->m_up;}
    auto right() const -> vec3 {return this->m_right;}
    auto front() const -> vec3 {return this->m_front;}
    [[nodiscard]] auto view_matrix() const -> const mat4& {return this->m_transform;}
  private:
    auto update() -> void;
    mat4 m_transform;
    vec3 m_position;
    vec3 m_front;
    vec3 m_up;
    vec3 m_right;
};
}

using PerspectiveCamera = Camera<camera_impl::Perspective>;
using OrthographicCamera = Camera<camera_impl::Orthographic>;
}
}