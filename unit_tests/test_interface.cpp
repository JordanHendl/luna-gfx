#include <gtest/gtest.h>
#include "luna-gfx/interface/buffer.hpp"
#include "luna-gfx/interface/image.hpp"
#include "luna-gfx/interface/render_pass.hpp"
#include "luna-gfx/interface/pipeline.hpp"
#include "luna-gfx/interface/window.hpp"

#include "unit_tests/vertex_shader.hpp"
#include "unit_tests/fragment_shader.hpp"
#include <array>
#include <vector>
#include <cstdint>
namespace luna::interface_test {
TEST(Interface, CreateBuffer) {
  constexpr auto cGPU = 0;
  constexpr auto cBufferSize = 1024u;
  auto buffer = luna::gfx::Buffer(cGPU, cBufferSize);
  EXPECT_GE(buffer.size(), cBufferSize);
}

TEST(Interface, MapBuffer)  {
  float* tmp = nullptr;
  constexpr auto cGPU = 0;
  constexpr auto cBufferSize = 1024u * sizeof(float);
  auto buffer = luna::gfx::Buffer(cGPU, cBufferSize);
  buffer.map(&tmp);
  EXPECT_TRUE(tmp != nullptr);
  buffer.unmap();
}

TEST(Interface, InitializeBufferWithData) {
  float* tmp = nullptr;
  constexpr auto cGPU = 0;
  constexpr auto cNumElements = 1024u;
  constexpr auto cBufferSize = cNumElements * sizeof(float);
  constexpr auto cConstantValue = 1337.f;
  const auto v = std::vector<float>(1024, cConstantValue);

  auto buffer = luna::gfx::Buffer(cGPU, cBufferSize);
  buffer.upload_data(v.data(), v.size());
  buffer.map(&tmp);
  EXPECT_TRUE(tmp != nullptr);
  for(auto index = 0u; index < cNumElements; index++) {
    EXPECT_FLOAT_EQ(tmp[index], cConstantValue);
  }
  buffer.unmap();
}

TEST(Interface, CreateImage) {
  constexpr auto cWidth = 1280u;
  constexpr auto cHeight = 1024u;
  constexpr auto cFormat = gfx::ImageFormat::RGBA8;
  constexpr auto cGPU = 0;
  auto info = gfx::ImageInfo();
  info.width = cWidth;
  info.height = cHeight;
  info.gpu = 0;
  info.format = cFormat;
  info.gpu = cGPU;
  auto image = gfx::Image(info);
  EXPECT_EQ(image.info().width, cWidth);
  EXPECT_EQ(image.info().height, cHeight);
  EXPECT_GE(image.handle(), 0);
}

TEST(Interface, CreateWindow) {
  auto window = gfx::Window(gfx::WindowInfo());
  EXPECT_GE(window.handle(), 0);
}

TEST(Interface, CreateRenderPass) {
  constexpr auto cWidth = 1280u;
  constexpr auto cHeight = 1024u;
  constexpr auto cFormat = gfx::ImageFormat::RGBA8;
  constexpr auto cGPU = 0;
  constexpr auto cClearColors = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f};

  auto info = gfx::RenderPassInfo();
  auto subpass = gfx::Subpass();
  auto attachment = gfx::Attachment();
  attachment.info.name = "ColorAttachment";
  attachment.info.width = cWidth;
  attachment.info.height = cHeight;
  attachment.info.gpu = 0;
  attachment.info.format = cFormat;
  attachment.info.gpu = cGPU;
  attachment.clear_color = cClearColors;

  subpass.attachments.push_back(attachment);
  info.subpasses.push_back(subpass);
  info.gpu = cGPU;
  info.width = cWidth;
  info.height = cHeight;

  auto rp = gfx::RenderPass(info);
  EXPECT_GE(rp.handle(), 0);

  {
    auto tmp = std::move(rp);
  }

  EXPECT_LT(rp.handle(), 0);
}

TEST(Interface, CreateRenderPassFromWindow) {
  constexpr auto cWidth = 1280u;
  constexpr auto cHeight = 1024u;
  constexpr auto cGPU = 0;

  auto window = gfx::Window(gfx::WindowInfo());
  auto info = gfx::RenderPassInfo();
  auto subpass = gfx::Subpass();
  auto attachment = window.attachment();
  subpass.attachments.push_back(attachment);
  info.subpasses.push_back(subpass);
  info.gpu = cGPU;
  info.width = cWidth;
  info.height = cHeight;

  auto rp = gfx::RenderPass(info, window);
  EXPECT_GE(rp.handle(), 0);
}

TEST(Interface, CreateRenderPipeline) {
  constexpr auto cWidth = 1280u;
  constexpr auto cHeight = 1024u;
  constexpr auto cFormat = gfx::ImageFormat::RGBA8;
  constexpr auto cGPU = 0;
  constexpr auto cClearColors = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f};

  auto info = gfx::RenderPassInfo();
  auto subpass = gfx::Subpass();
  auto attachment = gfx::Attachment();
  attachment.info.name = "ColorAttachment";
  attachment.info.width = cWidth;
  attachment.info.height = cHeight;
  attachment.info.gpu = 0;
  attachment.info.format = cFormat;
  attachment.info.gpu = cGPU;
  attachment.clear_color = cClearColors;

  subpass.attachments.push_back(attachment);
  info.subpasses.push_back(subpass);
  info.gpu = cGPU;
  info.width = cWidth;
  info.height = cHeight;

  auto rp = gfx::RenderPass(info);
  
  auto pipe_info = gfx::GraphicsPipelineInfo();
  pipe_info.gpu = cGPU;
  pipe_info.initial_viewport = {};
  pipe_info.shaders = {{"vertex", luna::gfx::ShaderType::Vertex, vertex_shader}, {"fragment", luna::gfx::ShaderType::Fragment, fragment_shader}};
  
  auto pipeline = luna::gfx::GraphicsPipeline(rp, pipe_info);
  EXPECT_GE(pipeline.handle(), 0);
}
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}