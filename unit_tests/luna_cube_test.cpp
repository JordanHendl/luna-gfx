#include "luna-gfx/gfx.hpp"
#include "unit_tests/vertex_shader.hpp"
#include "unit_tests/fragment_shader.hpp"
#include <array>
#include <vector>
#include <iostream>
struct vec3 {
  float x;
  float y;
  float z;
};

luna::gfx::Window window;
luna::gfx::RenderPass rp;
luna::gfx::GraphicsPipeline pipeline;

constexpr auto cWidth = 1280u;
constexpr auto cHeight = 1024u;
constexpr auto cGPU = 0;
constexpr auto cClearColors = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f};

const auto cVertices = std::array<vec3, 3> {{{-0.5f, -0.5f, 0.0f},
                                          { 0.5f, -0.5f, 0.0f},
                                          { 0.0f,  -1.0f, 0.0f}}};
bool running = true;

namespace luna {
auto init_graphics_pipeline() -> void {


  window = gfx::Window(gfx::WindowInfo());
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

  rp = gfx::RenderPass(info);
  
  auto pipe_info = gfx::GraphicsPipelineInfo();
  pipe_info.gpu = cGPU;
  pipe_info.initial_viewport = {};
  pipe_info.shaders = {{"vertex", luna::gfx::ShaderType::Vertex, vertex_shader}, {"fragment", luna::gfx::ShaderType::Fragment, fragment_shader}};
  
  pipeline = luna::gfx::GraphicsPipeline(rp, pipe_info);
}
auto draw_loop() -> void {
  auto cmd = gfx::CommandList(cGPU);
  auto bind_group = pipeline.create_bind_group();
  auto vertices = gfx::Vector<vec3>(cGPU, cVertices.size());
  auto event_handler = gfx::EventRegister();
  event_handler.add([](const gfx::Event& event){std::cout << "event recieved!" << std::endl; if(event.type() == gfx::Event::Type::WindowExit) running = false;});
  vertices.upload(cVertices.data());

  while(running) {

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
    luna::gfx::poll_events();
  }
}
}

auto main(int argc, const char* argv[]) -> int {
  luna::init_graphics_pipeline();
  luna::draw_loop();
  return 0;
}