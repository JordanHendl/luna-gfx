#include <gtest/gtest.h>
#include "luna-gfx/interface/buffer.hpp"
#include "luna-gfx/interface/image.hpp"
#include "luna-gfx/interface/render_pass.hpp"
#include "luna-gfx/interface/pipeline.hpp"
#include "luna-gfx/interface/window.hpp"
#include "luna-gfx/interface/command_list.hpp"

#include "unit_tests/vertex_shader.hpp"
#include "unit_tests/fragment_shader.hpp"
#include <array>
#include <vector>
#include <cstdint>
#include <ratio>
#include <algorithm>
template<typename T>
constexpr auto in_range(T min, T max, T val) {return min < val && val < max;}

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

TEST(Interface, CreateCommandBuffer) {
  constexpr auto cGPU = 0;
  auto cmd = gfx::CommandList(cGPU); 
  EXPECT_GE(cmd.handle(), 0);
  
  {
    auto tmp = std::move(cmd);
  }

  EXPECT_LT(cmd.handle(), 0);
}

TEST(Interface, CommandBufferCopyBufferToBuffer) {
  constexpr auto cGPU = 0;
  constexpr auto cSize = 1024;
  constexpr auto cExpectedValue = 255;
  constexpr auto cBaselineValue = 0;
  auto buf_a = gfx::Buffer(cGPU, cSize, gfx::Buffer::Type::CPUVisible);
  auto buf_b = gfx::Buffer(cGPU, cSize, gfx::Buffer::Type::CPUVisible);
  auto cmd = gfx::CommandList(cGPU);

  {
    auto container_a = buf_a.get_mapped_container<unsigned char>();
    auto container_b = buf_b.get_mapped_container<unsigned char>();
    std::fill(container_a.begin(), container_a.end(), cExpectedValue);
    std::fill(container_b.begin(), container_b.end(), cBaselineValue);
  }

  cmd.begin();
  cmd.copy(buf_a, buf_b);
  cmd.end();
  auto sync = cmd.submit();
  sync.wait();
  
  {
    auto container_a = buf_a.get_mapped_container<unsigned char>();
    auto container_b = buf_b.get_mapped_container<unsigned char>();
    for(auto& a : container_a) {EXPECT_EQ(a, cExpectedValue);}
    for(auto& a : container_b) {EXPECT_EQ(a, cExpectedValue);}
  }
}

TEST(Interface, CommandBufferTiming) {
  constexpr auto cGPU = 0;
  constexpr auto cSize = 1024;
  constexpr auto cNumIterations = 2048;
  constexpr auto cMinTimeMillis = 0.05;
  constexpr auto cMaxTimeMillis = 1.00;
  auto buf_a = gfx::Buffer(cGPU, cSize);
  auto buf_b = gfx::Buffer(cGPU, cSize);
  auto cmd = gfx::CommandList(cGPU);
  cmd.begin();
  cmd.start_time_stamp();
  for(auto index = 0; index < cNumIterations; index++) {
    cmd.copy(buf_a, buf_b);
  }
  auto duration = cmd.end_time_stamp();
  cmd.end();
  auto wait = cmd.submit();

  duration.wait();
  auto time = duration.get();
  auto time_in_millis = std::chrono::duration<double, std::milli>(time);
  EXPECT_TRUE(in_range(cMinTimeMillis, cMaxTimeMillis, time_in_millis.count()));
}
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}