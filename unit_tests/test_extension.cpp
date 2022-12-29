#include <gtest/gtest.h>
#include "luna-gfx/gfx.hpp"
#include "luna-gfx/ext/ext.hpp"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
namespace luna::extension_lib_test {
TEST(MathTest, GLMVectorCompatability)
{
  constexpr auto cX1 = 0.0;
  constexpr auto cY1 = 1.0;
  constexpr auto cZ1 = 2.5;
  constexpr auto cW1 = 6.2;

  const auto luna_vec2 = vec2(cX1, cY1);
  const auto luna_vec3 = vec3(cX1, cY1, cZ1);
  const auto luna_vec4 = vec4(cX1, cY1, cZ1, cW1);

  const auto glm_vec2 = glm::vec2(cX1, cY1);
  const auto glm_vec3 = glm::vec3(cX1, cY1, cZ1);
  const auto glm_vec4 = glm::vec4(cX1, cY1, cZ1, cW1);

  const auto empty_vec2 = glm::vec2{0, 0};  
  const auto empty_vec3 = glm::vec3{0, 0, 0};
  const auto empty_vec4 = glm::vec4{0, 0, 0, 0};

  EXPECT_EQ(luna_vec2, glm_vec2);
  EXPECT_EQ(luna_vec3, glm_vec3);
  EXPECT_EQ(luna_vec4, glm_vec4);
  EXPECT_NE(luna_vec2, empty_vec2);
  EXPECT_NE(luna_vec3, empty_vec3);
  EXPECT_NE(luna_vec4, empty_vec4);
}

TEST(MathTest, GLMMatrixCompatability)
{
  constexpr auto fov = 90.0f;
  constexpr auto width = 1280.f;
  constexpr auto height = 1024.f;
  constexpr auto ratio = width / height;
  constexpr auto near = 0.1f;
  constexpr auto far = 100.0f;

  const auto luna_mat4 = perspective(to_radians(fov), ratio, near, far);
  const auto glm_mat4 = glm::perspective(glm::radians(fov), ratio, near, far);
  const auto empty_mat4 = glm::mat4();
  EXPECT_EQ(luna_mat4, glm_mat4);
  EXPECT_NE(luna_mat4, empty_mat4);
}

TEST(ExtensionLib, MultiBufferedCreation)
{
  constexpr auto cGPU = 0;
  constexpr auto cBufferSize = 1024u;
  auto buffer = gfx::MultiBuffered<gfx::MemoryBuffer>(cGPU, cBufferSize);
  for(auto index = 0u; index < buffer.layers(); ++index) {
    EXPECT_GE(buffer->size(), cBufferSize);
    buffer.advance();
  }
}
}
  
int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}