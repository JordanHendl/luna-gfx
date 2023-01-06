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
luna::gfx::Renderer renderer;
luna::gfx::FramebufferCreator framebuffers;
luna::gfx::Vector<Transformations> gpu_transforms;
luna::gfx::BindGroup bind_group;
luna::gfx::Image img;

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
  auto vert_shader = std::vector<uint32_t>(draw_vert, std::end(draw_vert));
  auto frag_shader = std::vector<uint32_t>(draw_frag, std::end(draw_frag));

  auto info = gfx::RendererInfo();
  framebuffers = gfx::FramebufferCreator(cGPU, window.width(), window.height(), {{"Depth", gfx::ImageFormat::Depth}});
  info.render_pass_info = {cGPU, window.width(), window.height(), {{"Default", {{"WindowOutput", window.image_views()}, {"DepthAttachment", framebuffers.views()["Depth"], {1.0f, 1.0f, 1.0f, 1.0f}}}}}};
  info.pipeline_infos["DefaultPipeline"] = {cGPU, {{"vertex", luna::gfx::ShaderType::Vertex, vert_shader}, {"fragment", luna::gfx::ShaderType::Fragment, frag_shader}}};
  info.pipeline_infos["DefaultPipeline"].details.depth_test = true;
  renderer = std::move(gfx::Renderer(info));

  img = gfx::Image({"Default_Image", 0, 1024, 1024}); // Name, gpu, width, height
  gpu_transforms = gfx::Vector<Transformations>(cGPU, 1);
  bind_group = renderer.pipeline("DefaultPipeline").create_bind_group();
  bind_group.set(gpu_transforms, "transform");
  bind_group.set(img, "cube_texture");
  img.upload(DEFAULT_bmp + cBMPImageHeaderOffset);
}

auto draw_loop() -> void {
  Transformations* transforms = nullptr;
  auto vertices = gfx::Vector<Vertex>(cGPU, cVertices.size());
  vertices.upload(cVertices.data());

  auto event_handler = gfx::EventRegister();
  auto event_cb = [&transforms](const gfx::Event& event) {
    constexpr auto cOffsetAmt = 0.01f;
    if(event.type() == gfx::Event::Type::WindowExit) running = false;
    switch(event.key()) {
      case gfx::Key::Up : transforms->offset += cOffsetAmt; break;
      case gfx::Key::Down : transforms->offset -= cOffsetAmt; break;
      default: break;
    };
  };

  event_handler.add(event_cb);
  gpu_transforms.map(&transforms);
  transforms->model = 0;
  transforms->offset = 0.4f;
  auto start_time = std::chrono::system_clock::now();
  auto rot = 0.0;
  gpu_transforms.unmap();
  while(running) {
    auto& cmd = renderer.next();
    // Update rotation
    gpu_transforms.map(&transforms);
    auto time_since_start = std::chrono::system_clock::now() - start_time;
    auto time_in_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(time_since_start);
    rot = ((static_cast<double>(time_in_seconds.count()) / 1000.0f));
    transforms->model = static_cast<float>(rot);
    gpu_transforms.flush(); 
    gpu_transforms.unmap();

    // Combo next gpu action to the cmd list.
    window.combo_into(cmd);
    window.acquire();
    
    // Draw some stuff...
    cmd.begin();
    cmd.start_draw(renderer.pass(), window.current_frame());
    cmd.bind(bind_group);
    cmd.viewport({static_cast<float>(window.width()), static_cast<float>(window.height())});
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
  auto info = luna::gfx::WindowInfo();
  info.resizable = true;
  info.resize_callback = {[](){luna::init_graphics_pipeline();}};
  window = luna::gfx::Window(info);
  luna::init_graphics_pipeline();
  luna::draw_loop();
  return 0;
}