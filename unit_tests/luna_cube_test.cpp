#include "luna-gfx/gfx.hpp"
#include "luna-gfx/ext/ext.hpp"
#include <array>
#include <vector>
#include <iostream>
#include <chrono>

#include "unit_tests/raw_data/default_image.hpp"
#include "draw_vert.hpp"
#include "draw_frag.hpp"

struct Vertex {
  luna::vec3 position;
  luna::vec3 normal;
  luna::vec2 tex_coord;
};


struct Transformations {
  float model;
  float offset;
};

static_assert(sizeof(Vertex) == (sizeof(float) * 8));
static_assert(sizeof(Transformations) == (sizeof(float) * 2));

luna::gfx::Window window;
luna::gfx::RenderPass rp;
luna::gfx::GraphicsPipeline pipeline;
luna::gfx::FramebufferCreator framebuffers;

constexpr auto cWidth = 1280u;
constexpr auto cHeight = 1024u;
constexpr auto cGPU = 0;
constexpr auto cClearColors = std::array<float, 4>{0.0f, 0.2f, 0.2f, 1.0f};
constexpr auto cBMPImageHeaderOffset = 54;

const std::vector<Vertex> cVertices = {
        // positions          // normals           // texture coords
        {{-0.2f, -0.2f,  -0.2f},  {0.0f,  0.0f, -1.0f},  {0.0f,  0.0f}},
        {{ 0.2f, -0.2f,  -0.2f},  {0.0f,  0.0f, -1.0f},  {1.0f,  0.0f}},
        {{ 0.2f,  0.2f,  -0.2f},  {0.0f,  0.0f, -1.0f},  {1.0f,  1.0f}},
        {{ 0.2f,  0.2f,  -0.2f},  {0.0f,  0.0f, -1.0f},  {1.0f,  1.0f}},
        {{-0.2f,  0.2f,  -0.2f},  {0.0f,  0.0f, -1.0f},  {0.0f,  1.0f}},
        {{-0.2f, -0.2f,  -0.2f},  {0.0f,  0.0f, -1.0f},  {0.0f,  0.0f}},
        {{-0.2f, -0.2f,   0.2f},  {0.0f,  0.0f,  1.0f},  {0.0f,  0.0f}},
        {{ 0.2f, -0.2f,   0.2f},  {0.0f,  0.0f,  1.0f},  {1.0f,  0.0f}},
        {{ 0.2f,  0.2f,   0.2f},  {0.0f,  0.0f,  1.0f},  {1.0f,  1.0f}},
        {{ 0.2f,  0.2f,   0.2f},  {0.0f,  0.0f,  1.0f},  {1.0f,  1.0f}},
        {{-0.2f,  0.2f,   0.2f},  {0.0f,  0.0f,  1.0f},  {0.0f,  1.0f}},
        {{-0.2f, -0.2f,   0.2f},  {0.0f,  0.0f,  1.0f},  {0.0f,  0.0f}},
        {{-0.2f,  0.2f,   0.2f},  {-1.0f,  0.0f,  0.0f},  {1.0f,  0.0f}},
        {{-0.2f,  0.2f,  -0.2f},  {-1.0f,  0.0f,  0.0f},  {1.0f,  1.0f}},
        {{-0.2f, -0.2f,  -0.2f},  {-1.0f,  0.0f,  0.0f},  {0.0f,  1.0f}},
        {{-0.2f, -0.2f,  -0.2f},  {-1.0f,  0.0f,  0.0f},  {0.0f,  1.0f}},
        {{-0.2f, -0.2f,   0.2f},  {-1.0f,  0.0f,  0.0f},  {0.0f,  0.0f}},
        {{-0.2f,  0.2f,   0.2f},  {-1.0f,  0.0f,  0.0f},  {1.0f,  0.0f}},
        {{ 0.2f,  0.2f,   0.2f},  {1.0f,  0.0f,  0.0f},  {1.0f,  0.0f}},
        {{ 0.2f,  0.2f,  -0.2f},  {1.0f,  0.0f,  0.0f},  {1.0f,  1.0f}},
        {{ 0.2f, -0.2f,  -0.2f},  {1.0f,  0.0f,  0.0f},  {0.0f,  1.0f}},
        {{ 0.2f, -0.2f,  -0.2f},  {1.0f,  0.0f,  0.0f},  {0.0f,  1.0f}},
        {{ 0.2f, -0.2f,   0.2f},  {1.0f,  0.0f,  0.0f},  {0.0f,  0.0f}},
        {{ 0.2f,  0.2f,   0.2f},  {1.0f,  0.0f,  0.0f},  {1.0f,  0.0f}},
        {{-0.2f, -0.2f,  -0.2f},  {0.0f, -1.0f,  0.0f},  {0.0f,  1.0f}},
        {{ 0.2f, -0.2f,  -0.2f},  {0.0f, -1.0f,  0.0f},  {1.0f,  1.0f}},
        {{ 0.2f, -0.2f,   0.2f},  {0.0f, -1.0f,  0.0f},  {1.0f,  0.0f}},
        {{ 0.2f, -0.2f,   0.2f},  {0.0f, -1.0f,  0.0f},  {1.0f,  0.0f}},
        {{-0.2f, -0.2f,   0.2f},  {0.0f, -1.0f,  0.0f},  {0.0f,  0.0f}},
        {{-0.2f, -0.2f,  -0.2f},  {0.0f, -1.0f,  0.0f},  {0.0f,  1.0f}},
        {{-0.2f,  0.2f,  -0.2f},  {0.0f,  1.0f,  0.0f},  {0.0f,  1.0f}},
        {{ 0.2f,  0.2f,  -0.2f},  {0.0f,  1.0f,  0.0f},  {1.0f,  1.0f}},
        {{ 0.2f,  0.2f,   0.2f},  {0.0f,  1.0f,  0.0f},  {1.0f,  0.0f}},
        {{ 0.2f,  0.2f,   0.2f},  {0.0f,  1.0f,  0.0f},  {1.0f,  0.0f}},
        {{-0.2f,  0.2f,   0.2f},  {0.0f,  1.0f,  0.0f},  {0.0f,  0.0f}},
        {{-0.2f,  0.2f,  -0.2f},  {0.0f,  1.0f,  0.0f},  {0.0f,  1.0f}}
    };

bool running = true;

namespace luna {
auto init_graphics_pipeline() -> void {
  window = gfx::Window(gfx::WindowInfo());
  auto info = gfx::RenderPassInfo();
  auto subpass = gfx::Subpass();
  auto attachment = gfx::Attachment();

  // Set up the color attachment (writing to the window's image buffers)
  attachment.clear_color = cClearColors;
  attachment.views = window.image_views();
  subpass.attachments.push_back(attachment);

  // Now, set up the depth attachments.
  attachment.views.clear();
  framebuffers = std::move(gfx::FramebufferCreator(cGPU, window.info().width, window.info().height, {{"depth", gfx::ImageFormat::Depth}}));
  subpass.attachments.push_back({framebuffers.views()["depth"], {1.0f, 1.0f, 1.0f, 1.0f}});

  info.subpasses.push_back(subpass);

  // Set the render area & GPU to make this render pass on.
  info.gpu = cGPU;
  info.width = cWidth;
  info.height = cHeight;

  rp = gfx::RenderPass(info);
  
  // Create Graphics pipeline, attaching it to the render pass we created (pipeline outputs to the render pass)
  auto pipe_info = gfx::GraphicsPipelineInfo();
  pipe_info.gpu = cGPU;
  pipe_info.initial_viewport = {};
  auto vert_shader = std::vector<uint32_t>(draw_vert, std::end(draw_vert));
  auto frag_shader = std::vector<uint32_t>(draw_frag, std::end(draw_frag));
  pipe_info.shaders = {{"vertex", luna::gfx::ShaderType::Vertex, vert_shader}, {"fragment", luna::gfx::ShaderType::Fragment, frag_shader}};
  pipe_info.details.depth_test = true;
  pipeline = luna::gfx::GraphicsPipeline(rp, pipe_info);
}

auto draw_loop() -> void {
  Transformations* transforms = nullptr;

  auto cmd = gfx::MultiBuffered<gfx::CommandList>(cGPU);
  auto bind_group = pipeline.create_bind_group();
  auto img = gfx::Image({"Default_Image", 0, 1024, 1024}); // Name, gpu, width, height
  auto gpu_transforms = gfx::Vector<Transformations>(cGPU, 1);
  auto vertices = gfx::Vector<Vertex>(cGPU, cVertices.size());
  auto event_handler = gfx::EventRegister();
  
  img.upload(DEFAULT_bmp + cBMPImageHeaderOffset);

  auto event_cb = [&transforms](const gfx::Event& event) {
    constexpr auto cOffsetAmt = 0.01;
    if(event.type() == gfx::Event::Type::WindowExit) running = false;
    switch(event.key()) {
      case gfx::Key::Up : transforms->offset += cOffsetAmt; break;
      case gfx::Key::Down : transforms->offset -= cOffsetAmt; break;
      default: break;
    };
  };

  event_handler.add(event_cb);
  gpu_transforms.map(&transforms);
  vertices.upload(cVertices.data());
  bind_group.set(gpu_transforms, "transform");
  bind_group.set(img, "cube_texture");
  transforms->model = 0;
  transforms->offset = 0.4f;
  auto start_time = std::chrono::system_clock::now();
  auto rot = 0.0;
  while(running) {
    // Update rotation
    auto time_since_start = std::chrono::system_clock::now() - start_time;
    auto time_in_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(time_since_start);
    rot = ((static_cast<double>(time_in_seconds.count()) / 1000.0f));
    transforms->model = static_cast<float>(rot);
    gpu_transforms.flush(); 
    gfx::synchronize_gpu(cGPU);

    // Combo next gpu action to the cmd list.
    window.combo_into(*cmd);
    window.acquire();
    
    // Draw some stuff...
    cmd->begin();
    cmd->start_draw(rp, window.current_frame());
    cmd->bind(bind_group);
    cmd->viewport({});
    cmd->draw(vertices);
    cmd->end_draw();
    cmd->end();

    // Combo this cmd list submit (sending stuff to the GPU) into the window for it's next GPU operation.
    cmd->combo_into(window);

    // Submit CONSUMES the window combo.
    auto fence = cmd->submit();

    // CONSUMES the cmd combo.
    // Present the window. Since it was combo'd into by the cmd, it will wait on the cmd to finish on the GPU before presenting.
    window.present();

    // Go to next command buffer while this one is in flight.
    cmd.advance();

    fence.wait();
    // Wait for command to finish. Don't have to in realtime, but since this is looping we need to do so.
    luna::gfx::poll_events();
  }
  
  // Before we deconstruct everything, make sure we're done working on the GPU.
  gfx::synchronize_gpu(cGPU);
  gpu_transforms.unmap();
}
}

auto main(int argc, const char* argv[]) -> int {
  luna::init_graphics_pipeline();
  luna::draw_loop();
  return 0;
}