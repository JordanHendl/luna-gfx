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

TEST(MathTest, DotProduct) 
{
  const auto a = vec4(0.2, 0.3, 0.4, 0.5);
  const auto b = vec4(1.8, 2.2, 2.6, 3.0);
  const auto truth = 3.56;

  EXPECT_FLOAT_EQ(dot(a, b), truth);
}

TEST(MathTest, MatrixMultiplication)
{
  const auto luna_mat4 = mat4(1.0f);
  auto identity = luna_mat4 * luna_mat4;
  EXPECT_EQ(identity, identity);

  auto init_test_mat = [](auto& a, auto& b) {
    a[0] = {0.20, 0.60, 1.00, 1.40};
    a[1] = {0.30, 0.70, 1.10, 1.50};
    a[2] = {0.40, 0.80, 1.20, 1.60};
    a[3] = {0.50, 0.90, 1.30, 1.70};

    b[0] = {1.80, 2.20, 2.60, 3.00};
    b[1] = {1.90, 2.30, 2.70, 3.10};
    b[2] = {2.00, 2.40, 2.80, 3.20};
    b[3] = {2.10, 2.50, 2.90, 3.30};
  };

  auto truth = mat4(1.0f);
  truth[0] = {3.56, 7.40, 11.24, 15.08};
  truth[1] = {3.70, 7.70, 11.70, 15.70};
  truth[2] = {3.84, 8.00, 12.16, 16.32};
  truth[3] = {3.98, 8.30, 12.62, 16.94};

  auto test_a = mat4(1.0f);
  auto test_b = mat4(1.0f);

  init_test_mat(test_a, test_b);

  auto output_luna = test_a * test_b;
  auto identity_test = test_a * identity;

  EXPECT_EQ(identity_test, test_a);
  EXPECT_EQ(output_luna, truth);
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