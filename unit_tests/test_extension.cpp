#include <gtest/gtest.h>
#include "luna-gfx/gfx.hpp"
#include "luna-gfx/ext/ext.hpp"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "draw_vert.hpp"
#include "draw_frag.hpp"

namespace luna::extension_lib_test {
TEST(MathTest, GLMVectorCompatability)
{
  constexpr auto cX1 = 0.0f;
  constexpr auto cY1 = 1.0f;
  constexpr auto cZ1 = 2.5f;
  constexpr auto cW1 = 6.2f;

  const auto luna_vec2 = vec2(cX1, cY1);
  const auto luna_vec3 = vec3(cX1, cY1, cZ1);
  const auto luna_vec4 = vec4(cX1, cY1, cZ1, cW1);

  const auto glm_vec2 = glm::vec2(cX1, cY1);
  const auto glm_vec3 = glm::vec3(cX1, cY1, cZ1);
  const auto glm_vec4 = glm::vec4(cX1, cY1, cZ1, cW1);

  const auto empty_vec2 = glm::vec2{0.f, 0.f};  
  const auto empty_vec3 = glm::vec3{0.f, 0.f, 0.f};
  const auto empty_vec4 = glm::vec4{0.f, 0.f, 0.f, 0.f};

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
  const auto a = vec4(0.2f, 0.3f, 0.4f, 0.5f);
  const auto b = vec4(1.8f, 2.2f, 2.6f, 3.0f);
  const auto truth = 3.56f;

  EXPECT_FLOAT_EQ(dot(a, b), truth);
}

TEST(MathTest, MatrixMultiplication)
{
  const auto luna_mat4 = mat4(1.0f);
  auto identity = luna_mat4 * luna_mat4;
  EXPECT_EQ(identity, identity);

  auto init_test_mat = [](auto& a, auto& b) {
    a[0] = {0.20f, 0.60f, 1.00f, 1.40f};
    a[1] = {0.30f, 0.70f, 1.10f, 1.50f};
    a[2] = {0.40f, 0.80f, 1.20f, 1.60f};
    a[3] = {0.50f, 0.90f, 1.30f, 1.70f};

    b[0] = {1.80f, 2.20f, 2.60f, 3.00f};
    b[1] = {1.90f, 2.30f, 2.70f, 3.10f};
    b[2] = {2.00f, 2.40f, 2.80f, 3.20f};
    b[3] = {2.10f, 2.50f, 2.90f, 3.30f};
  };

  auto truth = mat4(1.0f);
  truth[0] = {3.56f, 7.40f, 11.24f, 15.08f};
  truth[1] = {3.70f, 7.70f, 11.70f, 15.70f};
  truth[2] = {3.84f, 8.00f, 12.16f, 16.32f};
  truth[3] = {3.98f, 8.30f, 12.62f, 16.94f};

  auto test_a = mat4(1.0f);
  auto test_b = mat4(1.0f);

  init_test_mat(test_a, test_b);

  auto output_luna = test_a * test_b;
  auto identity_test = test_a * identity;

  EXPECT_EQ(identity_test, test_a);
  EXPECT_EQ(output_luna, truth);
}

TEST(MathTest, Scale) {
  constexpr auto cScaleVec = luna::vec3(0.5f, 2.0f, 99.f);
  constexpr auto cGLMScaleVec = glm::vec3(0.5f, 2.0f, 99.f);
  auto t = luna::mat4();
  auto g = glm::mat4();

  g = glm::scale(glm::mat4(1.0f), cGLMScaleVec);
  t = luna::scale(luna::mat4(1.0f), cScaleVec);

  EXPECT_EQ(t, g);
}

TEST(MathTest, Translation) {
  constexpr auto cTranslationVec = luna::vec3(0.5f, 2.0f, 99.f);
  constexpr auto cGLMTranslationVec = glm::vec3(0.5f, 2.0f, 99.f);
  auto t = luna::mat4();
  auto g = glm::mat4();

  g = glm::translate(glm::mat4(1.0f), cGLMTranslationVec);
  t = luna::translate(luna::mat4(1.0f), cTranslationVec);

  EXPECT_EQ(t, g);
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

TEST(ExtensionLib, Renderer) {
  constexpr auto cGPU = 0;
  constexpr auto cWidth = 1280u;
  constexpr auto cHeight = 1024u;
  auto vert_shader = std::vector<uint32_t>(draw_vert, std::end(draw_vert));
  auto frag_shader = std::vector<uint32_t>(draw_frag, std::end(draw_frag));

  auto info = gfx::RendererInfo();
  auto framebuffers = gfx::FramebufferCreator(cGPU, cWidth, cHeight, {{"SomeImage", gfx::ImageFormat::RGBA8}});
  info.render_pass_info = {cGPU, cWidth, cHeight, {{"Default", {{"Default", framebuffers.views()["SomeImage"]}}}}};
  info.pipeline_infos["DefaultPipeline"] = {cGPU, {{"vertex", luna::gfx::ShaderType::Vertex, vert_shader}, {"fragment", luna::gfx::ShaderType::Fragment, frag_shader}}};
  auto renderer = gfx::Renderer(info);
}
}
  
int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}