#include "luna-gfx/gfx.hpp"
#include "luna-gfx/ext/ext.hpp"
#include <array>
#include <vector>
#include <iostream>
#include <chrono>
#include <cassert>

// These includes are for raw data files!
#include "unit_tests/raw_data/diffuse_image.hpp"
#include "unit_tests/raw_data/specular_image.hpp"

#include "g_buffer_vert.hpp"
#include "g_buffer_frag.hpp"

#include "deferred_vert.hpp"
#include "deferred_frag.hpp"

struct QuadVertex {
  luna::vec3 position;
  luna::vec2 tex_coords;
};

struct Vertex {
  luna::vec3 position;
  luna::vec3 normal;
  luna::vec2 tex_coord;
};

struct Transformations {
  luna::mat4 model;
};

static_assert(sizeof(Vertex) == (sizeof(float) * 8));
static_assert(sizeof(Transformations) == sizeof(luna::mat4));

struct DeferredTestData {
  luna::gfx::PerspectiveCamera camera;
  luna::gfx::Vector<Transformations> transforms;
  luna::gfx::Vector<luna::gfx::CameraInfo> camera_info;
  luna::gfx::Vector<Vertex> cube_vertices;
  luna::gfx::Vector<QuadVertex> quad_vertices;
  luna::gfx::Vector<int> switch_layer;
  luna::gfx::Image diffuse;
  luna::gfx::Image specular;

  luna::gfx::Window window;
  luna::gfx::RenderPass rp;
  luna::gfx::GraphicsPipeline gbuffer_pipe;
  luna::gfx::GraphicsPipeline final_pipe;
  luna::gfx::FramebufferCreator first_pass;
  luna::gfx::BindGroup gbuffer_bg;

  luna::mat4 projection;
  luna::gfx::MultiBuffered<luna::gfx::BindGroup> deferred_bg;
  luna::gfx::EventRegister event_handler;

  int* layer = nullptr;
};

constexpr auto cWidth = 1280u;
constexpr auto cHeight = 1024u;
constexpr auto cGPU = 0;
constexpr auto cClearColors = std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f};

std::unique_ptr<DeferredTestData> data;

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

std::vector<QuadVertex> quad_vertices = {
    // positions        // texture Coords
    {{-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f}},
    {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 1.0f,  1.0f, 0.0f}, {1.0f, 1.0f}},

    {{ 1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
    {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 1.0f,  1.0f, 0.0f}, {1.0f, 1.0f}}
};

bool running = true;

namespace luna {
auto create_first_subpass() -> gfx::Subpass {
  auto subpass = gfx::Subpass();
  auto attachment = gfx::Attachment();

  data->first_pass = gfx::FramebufferCreator(cGPU, data->window.info().width, data->window.info().height, 
  {
    {"position", gfx::ImageFormat::RGBA8},
    {"normal", gfx::ImageFormat::RGBA8},
    {"albedo", gfx::ImageFormat::RGBA8},
    {"depth", gfx::ImageFormat::Depth}
  });

  subpass.name = "GBufferPass";
  subpass.attachments.push_back({data->first_pass.views()["position"], cClearColors});
  subpass.attachments.push_back({data->first_pass.views()["normal"], cClearColors});
  subpass.attachments.push_back({data->first_pass.views()["albedo"], {0.0f, 0.0f, 0.0f, 0.0f}});
  subpass.attachments.push_back({data->first_pass.views()["depth"], {1.0f, 1.0f, 1.0f, 1.0f}});

  return subpass;
}

auto create_second_subpass() -> gfx::Subpass {
  auto subpass = gfx::Subpass();
  auto attachment = gfx::Attachment();
  subpass.name = "FinalDraw";
  subpass.attachments.push_back({data->window.image_views(), cClearColors});
  subpass.dependencies.insert({"GBufferPass", "position"});
  subpass.dependencies.insert({"GBufferPass", "normal"});
  subpass.dependencies.insert({"GBufferPass", "albedo"});
  
  return subpass;
}

auto create_gbuffer_pipeline() -> void {
  // Create Graphics pipeline, attaching it to the render pass we created (pipeline outputs to the render pass)
  auto pipe_info = gfx::GraphicsPipelineInfo();
  pipe_info.gpu = cGPU;
  pipe_info.initial_viewport = {};
  pipe_info.subpass = "GBufferPass";
  auto vert_shader = std::vector<uint32_t>(g_buffer_vert, std::end(g_buffer_vert));
  auto frag_shader = std::vector<uint32_t>(g_buffer_frag, std::end(g_buffer_frag));
  pipe_info.shaders = {{"vertex", luna::gfx::ShaderType::Vertex, vert_shader}, {"fragment", luna::gfx::ShaderType::Fragment, frag_shader}};
  pipe_info.details.depth_test = true;
  data->gbuffer_pipe = luna::gfx::GraphicsPipeline(data->rp, pipe_info);
}

auto create_final_pipeline() -> void {
  auto pipe_info = gfx::GraphicsPipelineInfo();
  pipe_info.gpu = cGPU;
  pipe_info.initial_viewport = {};
  auto vert_shader = std::vector<uint32_t>(deferred_vert, std::end(deferred_vert));
  auto frag_shader = std::vector<uint32_t>(deferred_frag, std::end(deferred_frag));
  pipe_info.shaders = {{"vertex", luna::gfx::ShaderType::Vertex, vert_shader}, {"fragment", luna::gfx::ShaderType::Fragment, frag_shader}};
  pipe_info.subpass = "FinalDraw";
  data->final_pipe = luna::gfx::GraphicsPipeline(data->rp, pipe_info);
}

auto create_uniforms() -> void {
  constexpr auto cLightMapDim = 500;
  const auto cBMPImageHeaderOffset = sizeof(box_diffuse_bmp) - (cLightMapDim * cLightMapDim * 4);
  data->gbuffer_bg = data->gbuffer_pipe.create_bind_group();
  data->diffuse = std::move(gfx::Image({"Diffuse", cGPU, cLightMapDim, cLightMapDim}, box_diffuse_bmp + cBMPImageHeaderOffset));
  data->specular = std::move(gfx::Image({"Diffuse", cGPU, cLightMapDim, cLightMapDim}, box_specular_bmp + cBMPImageHeaderOffset));
  data->transforms = std::move(gfx::Vector<Transformations>(cGPU, 1));
  data->camera_info = std::move(gfx::Vector<gfx::CameraInfo>(cGPU, 1));
  data->switch_layer = std::move(gfx::Vector<int>(cGPU, 1));
  data->switch_layer.map(&data->layer);
  data->layer[0] = 0;

  assert(data->gbuffer_bg.set(data->diffuse, "texture_diffuse1"));
  assert(data->gbuffer_bg.set(data->specular, "texture_specular1"));
  assert(data->gbuffer_bg.set(data->transforms, "transform"));
  assert(data->gbuffer_bg.set(data->camera_info, "camera"));

  // Since we have frames in flight, we need multilayered bind groups bound to each 'frame in flight'.
  for(auto i = 0u; i < data->deferred_bg.layers(); i++) {
    *data->deferred_bg = data->final_pipe.create_bind_group();
    assert(data->deferred_bg->set(data->first_pass["position"][i], "g_position"));
    assert(data->deferred_bg->set(data->first_pass["normal"][i], "g_normal"));
    assert(data->deferred_bg->set(data->first_pass["albedo"][i], "g_albedo_spec"));
    assert(data->deferred_bg->set(data->camera_info, "camera"));
    assert(data->deferred_bg->set(data->switch_layer, "layer"));
    data->deferred_bg.advance();
  }
}

auto create_vertex_buffers() -> void {
  data->cube_vertices = gfx::Vector<Vertex>(cGPU, cVertices.size());
  data->quad_vertices = gfx::Vector<QuadVertex>(cGPU, quad_vertices.size());

  data->cube_vertices.upload(cVertices.data());
  data->quad_vertices.upload(quad_vertices.data());
}

auto init_event_handlers() -> void {
    auto event_cb = [](const gfx::Event& event) {
    constexpr auto camera_speed = 0.1f;
    constexpr auto rotate_speed = 1.0f;
    if(event.type() == gfx::Event::Type::WindowExit) running = false;
    static auto camera_angles = vec3{0.0f, 0.0f, 0.0f};
    switch(event.key()) {
      case gfx::Key::One : *data->layer = 0; break;
      case gfx::Key::Two : *data->layer = 1; break;
      case gfx::Key::Three : *data->layer = 2; break;
      case gfx::Key::Four : *data->layer = 3; break;

      case gfx::Key::W : data->camera.translate(data->camera.front() * camera_speed); break;
      case gfx::Key::S : data->camera.translate(-(data->camera.front() * camera_speed)); break;
      case gfx::Key::D : data->camera.translate(data->camera.right() * camera_speed); break;
      case gfx::Key::A : data->camera.translate(-(data->camera.right() * camera_speed)); break;
      case gfx::Key::Space : data->camera.translate((data->camera.up() * camera_speed)); break;
      case gfx::Key::LShift : data->camera.translate(-(data->camera.up() * camera_speed)); break;

      case gfx::Key::Left : camera_angles.y += rotate_speed; data->camera.rotate_euler(camera_angles); break;
      case gfx::Key::Right : camera_angles.y -= rotate_speed; data->camera.rotate_euler(camera_angles); break;
      case gfx::Key::Up : camera_angles.x += rotate_speed; data->camera.rotate_euler(camera_angles); break;
      case gfx::Key::Down : camera_angles.x -= rotate_speed; data->camera.rotate_euler(camera_angles); break;
      default: break;
    };
  };
  data->event_handler.add(event_cb);
}

auto initialize() -> void {
  data->window = gfx::Window(gfx::WindowInfo());
  auto info = gfx::RenderPassInfo();
  auto subpass = gfx::Subpass();
  auto attachment = gfx::Attachment();

  info.subpasses.push_back(create_first_subpass());
  info.subpasses.push_back(create_second_subpass());

  // Set the render area & GPU to make this render pass on.
  info.gpu = cGPU;
  info.width = cWidth;
  info.height = cHeight;
  data->rp = gfx::RenderPass(info);

  create_gbuffer_pipeline();
  create_final_pipeline();
  create_uniforms();
  create_vertex_buffers();
  init_event_handlers();
  data->projection = luna::perspective(luna::to_radians(90.f), 1280.f / 1024.f, 0.1f, 1000.f);
}

auto draw_loop() -> void {
  auto cmd = gfx::MultiBuffered<gfx::CommandList>(cGPU);

  auto camera = data->camera_info.get_mapped_container();
  auto transform = data->transforms.get_mapped_container();

  transform[0].model = mat4(1.0f);
  while(running) {
    auto info = data->camera.info();
    camera[0].view_matrix = data->projection * info.view_matrix;
    // Combo next gpu action to the cmd list.
    data->window.combo_into(*cmd);
    data->window.acquire();
    
    // Draw some stuff...
    cmd->begin();
    cmd->start_draw(data->rp, data->window.current_frame());

    cmd->bind(data->gbuffer_bg);
    cmd->viewport({});
    cmd->draw(data->cube_vertices);

    cmd->next_subpass();

    cmd->bind(*data->deferred_bg);
    cmd->draw(data->quad_vertices);

    cmd->end_draw();
    cmd->end();

    cmd->combo_into(data->window);
    auto fence = cmd->submit();
    data->window.present();

    cmd.advance();
    data->deferred_bg.advance();
    luna::gfx::poll_events();
  }
  
  // Before we deconstruct everything, make sure we're done working on the GPU.
  gfx::synchronize_gpu(cGPU);
}
}
auto main(int argc, const char* argv[]) -> int {
  data = std::make_unique<DeferredTestData>();
  luna::initialize();
  luna::draw_loop();
  data->switch_layer.unmap();
  data = std::move(std::unique_ptr<DeferredTestData>());
  return 0;
}