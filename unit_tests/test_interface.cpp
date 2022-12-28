#include <gtest/gtest.h>
#include "luna-gfx/interface/buffer.hpp"
#include "luna-gfx/interface/image.hpp"
#include "luna-gfx/interface/render_pass.hpp"
#include "luna-gfx/interface/pipeline.hpp"
#include "luna-gfx/interface/window.hpp"
#include "luna-gfx/interface/command_list.hpp"
#include "luna-gfx/interface/event.hpp"

#include <array>
#include <vector>
#include <cstdint>
#include <ratio>
#include <algorithm>
#include <array>

#include "simple_vert.hpp"
#include "simple_frag.hpp"

struct vec3 {
  float x;
  float y;
  float z;
};

template<typename T>
constexpr auto in_range(T min, T max, T val) {return min < val && val < max;}

template<typename T>
inline auto check_vector_values(luna::gfx::Vector<T>& vec, T truth) {
  auto mapped = vec.get_mapped_container();
  for(auto& val : mapped) {
    EXPECT_EQ(val, truth);
  }
}

namespace luna::interface_test {
TEST(Interface, CreateBuffer) {
  constexpr auto cGPU = 0;
  constexpr auto cBufferSize = 1024u;
  auto buffer = luna::gfx::MemoryBuffer(cGPU, cBufferSize);
  EXPECT_GE(buffer.size(), cBufferSize);
}

TEST(Interface, CreateVector) {
  constexpr auto cGPU = 0;
  constexpr auto cNumElements = 1024;
  auto vec = gfx::Vector<float>(cGPU, cNumElements);
  EXPECT_EQ(vec.size(), cNumElements);
  EXPECT_GE(vec.handle(), 0); 
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
  constexpr auto cGPU = 0;
  constexpr auto cWidth = 1280u;
  constexpr auto cHeight = 1024u;
  constexpr auto cTripleBufferCount = 3;
  constexpr auto cFormat = gfx::ImageFormat::RGBA8;
  constexpr auto cClearColors = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f};

  auto info = gfx::RenderPassInfo();
  auto subpass = gfx::Subpass();
  auto attachment = gfx::Attachment();
  auto img_info = gfx::ImageInfo();
  auto framebuffers = std::vector<gfx::Image>();
  img_info.name = "ColorAttachment";
  img_info.width = cWidth;
  img_info.height = cHeight;
  img_info.gpu = 0;
  img_info.format = cFormat;
  img_info.gpu = cGPU;

  attachment.clear_color = cClearColors;
  framebuffers.reserve(cTripleBufferCount);
  for(auto index = 0u; index < cTripleBufferCount; index++) {
    framebuffers.push_back(gfx::Image(img_info));
    attachment.views.push_back(framebuffers.back());
  }

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
  auto attachment = gfx::Attachment();

  attachment.views = window.image_views();
  subpass.attachments.push_back(attachment);
  info.subpasses.push_back(subpass);
  info.gpu = cGPU;
  info.width = cWidth;
  info.height = cHeight;

  auto rp = gfx::RenderPass(info);
  EXPECT_GE(rp.handle(), 0);
}

TEST(Interface, CreateRenderPipeline) {
  constexpr auto cWidth = 1280u;
  constexpr auto cHeight = 1024u;
  constexpr auto cTripleBufferCount = 3;
  constexpr auto cFormat = gfx::ImageFormat::RGBA8;
  constexpr auto cGPU = 0;
  constexpr auto cClearColors = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f};

  auto info = gfx::RenderPassInfo();
  auto subpass = gfx::Subpass();
  auto attachment = gfx::Attachment();
  auto framebuffers = std::vector<gfx::Image>();
  auto img_info = gfx::ImageInfo();
  img_info.name = "ColorAttachment";
  img_info.width = cWidth;
  img_info.height = cHeight;
  img_info.gpu = 0;
  img_info.format = cFormat;
  img_info.gpu = cGPU;

  attachment.clear_color = cClearColors;
  framebuffers.reserve(cTripleBufferCount);
  for(auto index = 0u; index < cTripleBufferCount; index++) {
    framebuffers.push_back(gfx::Image(img_info));
    attachment.views.push_back(framebuffers.back());
  }

  subpass.attachments.push_back(attachment);
  info.subpasses.push_back(subpass);
  info.gpu = cGPU;
  info.width = cWidth;
  info.height = cHeight;

  auto rp = gfx::RenderPass(info);
  
  auto pipe_info = gfx::GraphicsPipelineInfo();
  pipe_info.gpu = cGPU;
  pipe_info.initial_viewport = {};
  auto vert_shader = std::vector<uint32_t>(simple_vert, std::end(simple_vert));
  auto frag_shader = std::vector<uint32_t>(simple_frag, std::end(simple_frag));
  pipe_info.shaders = {{"vertex", luna::gfx::ShaderType::Vertex, vert_shader}, {"fragment", luna::gfx::ShaderType::Fragment, frag_shader}};
  
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

TEST(Interface, VectorResize) {
  constexpr auto cGPU = 0;
  constexpr auto cNumElements = 1024;
  constexpr auto cNewNumElements = 5000;
  auto vec = gfx::Vector<float>(cGPU, cNumElements);
  EXPECT_EQ(vec.size(), cNumElements);
  vec.resize(cNewNumElements);
  EXPECT_EQ(vec.size(), cNewNumElements);
}

TEST(Interface, MapBuffer)  {
  float* tmp = nullptr;
  constexpr auto cGPU = 0;
  constexpr auto cBufferSize = 1024u * sizeof(float);
  auto buffer = luna::gfx::MemoryBuffer(cGPU, cBufferSize);
  buffer.map(&tmp);
  EXPECT_TRUE(tmp != nullptr);
  buffer.unmap();

  {
    auto container = buffer.get_mapped_container<char>();
    EXPECT_TRUE(container.begin() != nullptr);
    EXPECT_TRUE(container.data() != nullptr);
    EXPECT_EQ(container.size(), buffer.size());
  }

  {
    auto container = buffer.get_mapped_container<float>();
    EXPECT_TRUE(container.begin() != nullptr);
    EXPECT_TRUE(container.data() != nullptr);
    EXPECT_EQ(container.size(), buffer.size() / sizeof(float));
  }
}

TEST(Interface, InitializeBufferWithData) {
  float* tmp = nullptr;
  constexpr auto cGPU = 0;
  constexpr auto cNumElements = 1024u;
  constexpr auto cBufferSize = cNumElements * sizeof(float);
  constexpr auto cConstantValue = 1337.f;
  const auto v = std::vector<float>(1024, cConstantValue);

  auto buffer = luna::gfx::MemoryBuffer(cGPU, cBufferSize);
  buffer.upload(v.data(), v.size());
  buffer.map(&tmp);
  EXPECT_TRUE(tmp != nullptr);
  for(auto index = 0u; index < cNumElements; index++) {
    EXPECT_FLOAT_EQ(tmp[index], cConstantValue);
  }
  buffer.unmap();
}

TEST(Interface, CommandBufferCopyBufferToBuffer) {
  constexpr auto cGPU = 0;
  constexpr auto cSize = 1024;
  constexpr auto cExpectedValue = 255;
  constexpr auto cBaselineValue = 0;
  auto buf_a = gfx::MemoryBuffer(cGPU, cSize, gfx::MemoryType::CPUVisible);
  auto buf_b = gfx::MemoryBuffer(cGPU, cSize, gfx::MemoryType::CPUVisible);
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

TEST(Interface, CommandBufferDraw) {
  /** Lots of work to draw something! Maybe make an object to make this go faster to write?
   * 
   * ... but at the same time, this gives you a lot of fine-grain detail. So idk.
   */
  constexpr auto cGPU = 0;
  constexpr auto cWidth = 1280u;
  constexpr auto cHeight = 1024u;
  constexpr auto cFormat = gfx::ImageFormat::RGBA8;
  constexpr auto cTripleBufferCount = 3;
  constexpr auto cClearColors = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f};
  const auto cVertices = std::array<vec3, 3> {{{-0.5f, -0.5f, 0.0f},
                                              { 0.5f, -0.5f, 0.0f},
                                              { 0.0f,  0.5f, 0.0f}}};

  auto info = gfx::RenderPassInfo();
  auto subpass = gfx::Subpass();
  auto attachment = gfx::Attachment();
  auto img_info = gfx::ImageInfo();
  auto framebuffers = std::vector<gfx::Image>();
  img_info.name = "ColorAttachment";
  img_info.width = cWidth;
  img_info.height = cHeight;
  img_info.gpu = 0;
  img_info.format = cFormat;
  img_info.gpu = cGPU;

  attachment.clear_color = cClearColors;
  framebuffers.reserve(cTripleBufferCount);
  for(auto index = 0u; index < cTripleBufferCount; index++) {
    framebuffers.emplace_back(gfx::Image(img_info));
    attachment.views.push_back(framebuffers.back());
  }

  subpass.attachments.push_back(attachment);
  info.subpasses.push_back(subpass);
  info.gpu = cGPU;
  info.width = cWidth;
  info.height = cHeight;

  auto rp = gfx::RenderPass(info);
  auto cmd = gfx::CommandList(cGPU);
  auto pipe_info = gfx::GraphicsPipelineInfo();
  auto vertices = gfx::Vector<vec3>(cGPU, cVertices.size());
  pipe_info.gpu = cGPU;
  pipe_info.initial_viewport = {};
  auto vert_shader = std::vector<uint32_t>(simple_vert, std::end(simple_vert));
  auto frag_shader = std::vector<uint32_t>(simple_frag, std::end(simple_frag));
  pipe_info.shaders = {{"vertex", luna::gfx::ShaderType::Vertex, vert_shader}, {"fragment", luna::gfx::ShaderType::Fragment, frag_shader}};
  
  vertices.upload(cVertices.data());
  auto pipeline = luna::gfx::GraphicsPipeline(rp, pipe_info);
  auto bind_group = pipeline.create_bind_group();

  cmd.begin();
  cmd.start_draw(rp); // Tell backend to start drawing to this render pass
  cmd.bind(bind_group); // Bind resources (pipeline, resources, etc.)
  cmd.viewport({}); // update viewport of the scene (only have to do once).
  cmd.draw(vertices);
  //cmd.next_subpass(); // Not yet implemented, but ideally should push it to the next subpass for any subsequent draws.
  cmd.end_draw  (); // Tell backend to stop drawing to that render pass. (Can attach a separate one if you want)
  cmd.end();
  auto fence = cmd.submit();
  EXPECT_GE(pipeline.handle(), 0);
}

TEST(Interface, CommandBufferTiming) {
  constexpr auto cGPU = 0;
  constexpr auto cSize = 1024;
  constexpr auto cNumIterations = 2048;
  constexpr auto cMinTimeMillis = 0.05;
  constexpr auto cMaxTimeMillis = 1.00;
  auto buf_a = gfx::MemoryBuffer(cGPU, cSize);
  auto buf_b = gfx::MemoryBuffer(cGPU, cSize);
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

TEST(Interface, CommandBufferCombos) {
  constexpr auto cGPU = 0;
  constexpr auto cSize = 1024;
  constexpr auto cExample1 = 10.0f;
  constexpr auto cExample2 = 55.0f;
  constexpr auto cExample3 = 144.0f;
  constexpr auto cExample4 = 277.0f;
  const auto data1 = std::vector<float>(cSize, cExample1);
  const auto data2 = std::vector<float>(cSize, cExample2);
  const auto data3 = std::vector<float>(cSize, cExample3);
  const auto data4 = std::vector<float>(cSize, cExample4);
  auto buf_a = gfx::Vector<float>(cGPU, cSize);
  auto buf_b = gfx::Vector<float>(cGPU, cSize);
  auto buf_c = gfx::Vector<float>(cGPU, cSize);
  auto buf_d = gfx::Vector<float>(cGPU, cSize);
  auto cmd1 = gfx::CommandList(cGPU);
  auto cmd2 = gfx::CommandList(cGPU);
  auto cmd3 = gfx::CommandList(cGPU);

  // Fill buffers with initial data.
  buf_a.upload(data1.data());
  buf_b.upload(data2.data());
  buf_c.upload(data3.data());
  buf_d.upload(data4.data());

  // Record a sequence of copies and combo them together.
  // Ideally, a -> b -> c -> d and all should happen in that order. 
  // So, at the end, a, b, c, and d should all be cExample1.
  cmd1.begin();
  cmd1.copy(buf_a, buf_b);
  cmd1.end();

  cmd2.begin();
  cmd2.copy(buf_b, buf_c);
  cmd2.end();

  cmd3.begin();
  cmd3.copy(buf_c, buf_d);
  cmd3.end();
  
  // Combos!!!
  cmd1.combo_into(cmd2);
  auto wait1 = cmd1.submit();
  
  cmd2.combo_into(cmd3);
  auto wait2 = cmd2.submit();

  auto wait3 = cmd3.submit();

  // Only wait for 3 to complete. Doing this SHOULD make all of the others finished as well.
  wait3.wait();

  // Check and make sure the vectors are correct.
  check_vector_values(buf_a, cExample1);
  check_vector_values(buf_b, cExample1);
  check_vector_values(buf_c, cExample1);
  check_vector_values(buf_d, cExample1);
}

TEST(Interface, CommandBufferDrawToWindow) {
  constexpr auto cWidth = 1280u;
  constexpr auto cHeight = 1024u;
  constexpr auto cGPU = 0;
  constexpr auto cNumIterations = 1;
  constexpr auto cClearColors = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f};
  const auto cVertices = std::array<vec3, 3> {{{-0.5f, -0.5f, 0.0f},
                                          { 0.5f, -0.5f, 0.0f},
                                          { 0.0f,  0.5f, 0.0f}}};

  auto window = gfx::Window(gfx::WindowInfo());
  auto info = gfx::RenderPassInfo();
  auto subpass = gfx::Subpass();
  auto attachment = gfx::Attachment();
  attachment.clear_color = cClearColors;
  attachment.views = window.image_views();
  subpass.attachments.push_back(attachment);
  info.subpasses.push_back(subpass);
  info.gpu = cGPU;
  info.width = cWidth;
  info.height = cHeight;

  auto rp = gfx::RenderPass(info);
  
  auto pipe_info = gfx::GraphicsPipelineInfo();
  pipe_info.gpu = cGPU;
  pipe_info.initial_viewport = {};
  auto vert_shader = std::vector<uint32_t>(simple_vert, std::end(simple_vert));
  auto frag_shader = std::vector<uint32_t>(simple_frag, std::end(simple_frag));
  pipe_info.shaders = {{"vertex", luna::gfx::ShaderType::Vertex, vert_shader}, {"fragment", luna::gfx::ShaderType::Fragment, frag_shader}};
  
  auto pipeline = luna::gfx::GraphicsPipeline(rp, pipe_info);

  auto cmd = gfx::CommandList(cGPU);
  auto bind_group = pipeline.create_bind_group();
  auto vertices = gfx::Vector<vec3>(cGPU, cVertices.size());
  vertices.upload(cVertices.data());

  for(auto i = 0u; i < cNumIterations; i++) {

    // Combo next gpu action to the cmd list.
    window.combo_into(cmd);
    window.acquire();

    // Draw some stuff...
    cmd.begin();
    cmd.start_draw(rp, window.current_frame());
    cmd.bind(bind_group);
    cmd.viewport({});
    cmd.draw(vertices);
    cmd.end_draw();
    cmd.end();

    // Combo this cmd list submit (sending stuff to the GPU) into the window for it's next GPU operation.
    cmd.combo_into(window);

    // Submit CONSUMES the window combo.
    auto fence = cmd.submit();

    // CONSUMES the cmd combo.
    // Present the window. Since it was combo'd into by the cmd, it will wait on the cmd to finish on the GPU before presenting.
    window.present();

    // Wait for command to finish. Don't have to in realtime, but since this is looping we need to do so.
    fence.wait();
  }
}
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}