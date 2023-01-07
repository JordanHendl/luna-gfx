#include "luna-gfx/ext/camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace luna {
namespace gfx {
namespace camera_impl {

const auto world_up = vec3{0.0f, -1.0f, 0.0f};

Perspective::Perspective() {
  this->m_position = {0.0f, 0.0f, 0.0f};
  this->m_up = world_up;
  this->m_right = {1.0f, 0.0f, 0.0f};
  this->m_front = {0.0f, 0.0f, 1.0f};
  this->m_transform = {};
}

auto Perspective::translate(vec3 offsets) -> void {
  this->m_position = this->m_position + offsets;
  this->update();
}

auto Perspective::rotate_euler(float x, float y, float) -> void {
  const auto glm_world_up = glm::vec3{world_up.x, world_up.y, world_up.z};
  const auto pitch = x;
  const auto yaw = y;
  //const auto roll = z;

  this->m_front.x = std::cos(to_radians(yaw)) * std::cos(to_radians(pitch));
  this->m_front.y = std::sin(to_radians(pitch));
  this->m_front.z = std::sin(to_radians(yaw)) * std::cos(to_radians(pitch));

  auto front = glm::normalize(glm::vec3(m_front.x, m_front.y, m_front.z));
  auto right = glm::normalize(glm::cross(front, glm_world_up));
  auto up    = glm::normalize(glm::cross(right, front));  

  this->m_front = front;
  this->m_right = right;
  this->m_up = up;
  this->update();
}

auto Perspective::look_at(vec3 position) -> void {
  auto target_pos = glm::vec3{position.x, position.y, position.z};
  auto pos = glm::vec3{this->m_position.x, this->m_position.y, this->m_position.z};
  auto up = glm::vec3{this->m_up.x, this->m_up.y, this->m_up.z};
  
  this->m_transform = glm::lookAtRH(pos, pos + target_pos, up);  
}

auto Perspective::update() -> void {
  auto pos = glm::vec3{this->m_position.x, this->m_position.y, this->m_position.z};
  auto front = glm::vec3{this->m_front.x, this->m_front.y, this->m_front.z};
  auto up = glm::vec3{this->m_up.x, this->m_up.y, this->m_up.z};

  this->m_transform = glm::lookAtRH(pos, pos + front, up);  
}
}
}
}