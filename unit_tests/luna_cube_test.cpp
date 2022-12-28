#include "luna-gfx/gfx.hpp"
#include <array>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#include "unit_tests/default_image.hpp"
#include "draw_vert.hpp"
#include "draw_frag.hpp"

struct vec3 {
  float x;
  float y;
  float z;
};

struct vec2 {
  float x;
  float y;
};

struct Vertex {
  vec3 position;
  vec3 normal;
  vec2 tex_coord;
};


struct Transformations {
  float model;
};

static_assert(sizeof(Vertex) == (sizeof(float) * 8));
static_assert(sizeof(vec3) == (sizeof(float) * 3));
static_assert(sizeof(Transformations) == (sizeof(float)));

luna::gfx::Window window;
luna::gfx::RenderPass rp;
luna::gfx::GraphicsPipeline pipeline;
std::vector<luna::gfx::Image> depth_images;
constexpr auto cWidth = 1280u;
constexpr auto cHeight = 1024u;
constexpr auto cGPU = 0;
constexpr auto cClearColors = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f};
constexpr auto cBMPImageHeaderOffset = 54;

//const std::vector<Vertex> cVertices = {
//        // positions          // normals           // texture coords
//        {{1.0f, 0.0f, 0.0f},  {0.0f,  0.0f, -1.0f},  {0.0f,  0.0f}},
//        {{0.0f, 1.0f, 0.0f},  {0.0f,  0.0f, -1.0f},  {1.0f,  0.0f}},
//        {{0.0f, 0.0f, 1.0f},  {0.0f,  0.0f, -1.0f},  {1.0f,  1.0f}},
//        
//        {{0.0f, 0.0f, 1.0f},  {0.0f,  0.0f, -1.0f},  {1.0f,  1.0f}},
//        {{1.0f, 1.0f, 1.0f},  {0.0f,  0.0f, -1.0f},  {0.0f,  1.0f}},
//        {{1.0f, 0.0f, 0.0f},  {0.0f,  0.0f, -1.0f},  {0.0f,  0.0f}},
//    };

const std::vector<Vertex> cVertices = {
        // positions          // normals           // texture coords
        {{-0.5f, -0.5f,  0.0f},  {0.0f,  0.0f, -1.0f},  {0.0f,  0.0f}},
        {{ 0.5f, -0.5f,  0.0f},  {0.0f,  0.0f, -1.0f},  {1.0f,  0.0f}},
        {{ 0.5f,  0.5f,  0.0f},  {0.0f,  0.0f, -1.0f},  {1.0f,  1.0f}},
        {{ 0.5f,  0.5f,  0.0f},  {0.0f,  0.0f, -1.0f},  {1.0f,  1.0f}},
        {{-0.5f,  0.5f,  0.0f},  {0.0f,  0.0f, -1.0f},  {0.0f,  1.0f}},
        {{-0.5f, -0.5f,  0.0f},  {0.0f,  0.0f, -1.0f},  {0.0f,  0.0f}},
        {{-0.5f, -0.5f,  1.0f},  {0.0f,  0.0f,  1.0f},  {0.0f,  0.0f}},
        {{ 0.5f, -0.5f,  1.0f},  {0.0f,  0.0f,  1.0f},  {1.0f,  0.0f}},
        {{ 0.5f,  0.5f,  1.0f},  {0.0f,  0.0f,  1.0f},  {1.0f,  1.0f}},
        {{ 0.5f,  0.5f,  1.0f},  {0.0f,  0.0f,  1.0f},  {1.0f,  1.0f}},
        {{-0.5f,  0.5f,  1.0f},  {0.0f,  0.0f,  1.0f},  {0.0f,  1.0f}},
        {{-0.5f, -0.5f,  1.0f},  {0.0f,  0.0f,  1.0f},  {0.0f,  0.0f}},
        {{-0.5f,  0.5f,  1.0f},  {-1.0f,  0.0f,  0.0f},  {1.0f,  0.0f}},
        {{-0.5f,  0.5f,  0.0f},  {-1.0f,  0.0f,  0.0f},  {1.0f,  1.0f}},
        {{-0.5f, -0.5f,  0.0f},  {-1.0f,  0.0f,  0.0f},  {0.0f,  1.0f}},
        {{-0.5f, -0.5f,  0.0f},  {-1.0f,  0.0f,  0.0f},  {0.0f,  1.0f}},
        {{-0.5f, -0.5f,  1.0f},  {-1.0f,  0.0f,  0.0f},  {0.0f,  0.0f}},
        {{-0.5f,  0.5f,  1.0f},  {-1.0f,  0.0f,  0.0f},  {1.0f,  0.0f}},
        {{ 0.5f,  0.5f,  1.0f},  {1.0f,  0.0f,  0.0f},  {1.0f,  0.0f}},
        {{ 0.5f,  0.5f,  0.0f},  {1.0f,  0.0f,  0.0f},  {1.0f,  1.0f}},
        {{ 0.5f, -0.5f,  0.0f},  {1.0f,  0.0f,  0.0f},  {0.0f,  1.0f}},
        {{ 0.5f, -0.5f,  0.0f},  {1.0f,  0.0f,  0.0f},  {0.0f,  1.0f}},
        {{ 0.5f, -0.5f,  1.0f},  {1.0f,  0.0f,  0.0f},  {0.0f,  0.0f}},
        {{ 0.5f,  0.5f,  1.0f},  {1.0f,  0.0f,  0.0f},  {1.0f,  0.0f}},
        {{-0.5f, -0.5f,  0.0f},  {0.0f, -1.0f,  0.0f},  {0.0f,  1.0f}},
        {{ 0.5f, -0.5f,  0.0f},  {0.0f, -1.0f,  0.0f},  {1.0f,  1.0f}},
        {{ 0.5f, -0.5f,  1.0f},  {0.0f, -1.0f,  0.0f},  {1.0f,  0.0f}},
        {{ 0.5f, -0.5f,  1.0f},  {0.0f, -1.0f,  0.0f},  {1.0f,  0.0f}},
        {{-0.5f, -0.5f,  1.0f},  {0.0f, -1.0f,  0.0f},  {0.0f,  0.0f}},
        {{-0.5f, -0.5f,  0.0f},  {0.0f, -1.0f,  0.0f},  {0.0f,  1.0f}},
        {{-0.5f,  0.5f,  0.0f},  {0.0f,  1.0f,  0.0f},  {0.0f,  1.0f}},
        {{ 0.5f,  0.5f,  0.0f},  {0.0f,  1.0f,  0.0f},  {1.0f,  1.0f}},
        {{ 0.5f,  0.5f,  1.0f},  {0.0f,  1.0f,  0.0f},  {1.0f,  0.0f}},
        {{ 0.5f,  0.5f,  1.0f},  {0.0f,  1.0f,  0.0f},  {1.0f,  0.0f}},
        {{-0.5f,  0.5f,  1.0f},  {0.0f,  1.0f,  0.0f},  {0.0f,  0.0f}},
        {{-0.5f,  0.5f,  0.0f},  {0.0f,  1.0f,  0.0f},  {0.0f,  1.0f}}
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
  auto depth_img_info = gfx::ImageInfo();
  depth_images.resize(window.image_views().size());
  depth_img_info.format = luna::gfx::ImageFormat::Depth;
  depth_img_info.width = window.info().width;
  depth_img_info.height = window.info().height;
  depth_img_info.gpu = cGPU;
  attachment.clear_color = {1.0f, 1.0f, 1.0f, 1.0f};

  // They need to be the same amount of buffers as the image attachments (probably triple-buffered)
  for(auto& img : depth_images) {
    img = std::move(luna::gfx::Image(depth_img_info));
    attachment.views.push_back(img);
  }

  subpass.enable_depth = true;
  subpass.attachments.push_back(attachment);
  info.subpasses.push_back(subpass);
  info.gpu = cGPU;
  info.width = cWidth;
  info.height = cHeight;

  rp = gfx::RenderPass(info);
  
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
  auto img = gfx::Image({"Default_Image", 0, 1024, 1024}); // Name, gpu, width, height
  auto gpu_transforms = gfx::Vector<Transformations>(cGPU, 1);
  auto cmd = gfx::CommandList(cGPU);
  auto bind_group = pipeline.create_bind_group();
  auto vertices = gfx::Vector<Vertex>(cGPU, cVertices.size());
  auto event_handler = gfx::EventRegister();

  img.upload(DEFAULT_bmp + cBMPImageHeaderOffset);

  event_handler.add([](const gfx::Event& event){if(event.type() == gfx::Event::Type::WindowExit) running = false;});
  gpu_transforms.map(&transforms);
  vertices.upload(cVertices.data());
  bind_group.set(gpu_transforms, "transform");
  bind_group.set(img, "cube_texture");
  transforms->model = 0;//glm::mat4(1.0f);

  auto start_time = std::chrono::system_clock::now();
  auto rot = 0.0f;
  while(running) {
    // Update rotation
    auto time_since_start = std::chrono::system_clock::now() - start_time;
    auto time_in_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(time_since_start);
    rot += ((static_cast<float>(time_in_seconds.count()) / 1000.0f));
    start_time = std::chrono::system_clock::now();
    transforms->model = rot;//glm::rotate(glm::mat4(1.0f), rot, glm::vec3(1.0f, 0.3f, 0.5f));

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

    fence.wait();
    // Wait for command to finish. Don't have to in realtime, but since this is looping we need to do so.
    luna::gfx::poll_events();
  }
}
}

auto main(int argc, const char* argv[]) -> int {
  luna::init_graphics_pipeline();
  luna::draw_loop();
  return 0;
}