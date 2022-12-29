#include "luna-gfx/gfx.hpp"
#include "luna-gfx/ext/ext.hpp"
#include <array>
#include <vector>
#include <iostream>
#include <chrono>

#include "g_buffer_vert.hpp"
#include "g_buffer_frag.hpp"

#include "deferred_vert.hpp"
#include "deferred_frag.hpp"

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
  luna::gfx::Image diffuse;
  luna::gfx::Image specular;

  luna::gfx::Window window;
  luna::gfx::RenderPass rp;
  luna::gfx::GraphicsPipeline gbuffer_pipe;
  luna::gfx::GraphicsPipeline final_pipe;
  luna::gfx::FramebufferCreator first_pass;
  luna::gfx::BindGroup gbuffer_bg;
  luna::gfx::BindGroup deferred_bg;
};

constexpr auto cWidth = 1280u;
constexpr auto cHeight = 1024u;
constexpr auto cGPU = 0;
constexpr auto cClearColors = std::array<float, 4>{0.0f, 0.2f, 0.2f, 1.0f};
constexpr auto cBMPImageHeaderOffset = 54;

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
  subpass.attachments.push_back({data->first_pass.views()["albedo"], cClearColors});
  subpass.attachments.push_back({data->first_pass.views()["depth"], cClearColors});

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
  data->gbuffer_bg = data->gbuffer_pipe.create_bind_group();
  data->deferred_bg = data->final_pipe.create_bind_group();

  data->diffuse = std::move(gfx::Image({"Diffuse", cGPU, data->window.info().width, data->window.info().height}));
  data->specular = std::move(gfx::Image({"Diffuse", cGPU, data->window.info().width, data->window.info().height}));
  data->transforms = std::move(gfx::Vector<Transformations>(cGPU, 1));

  data->gbuffer_bg.set(data->diffuse, "texture_diffuse1");
  data->gbuffer_bg.set(data->specular, "texture_specular1");
  data->gbuffer_bg.set(data->transforms, "transform");

  //data->deferred_bg.set("//")
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
}

auto draw_loop() -> void {
  //Transformations* transforms = nullptr;
  //auto cmd = gfx::MultiBuffered<gfx::CommandList>(cGPU);
}
}
auto main(int argc, const char* argv[]) -> int {
  data = std::make_unique<DeferredTestData>();
  luna::initialize();
  luna::draw_loop();
  data = std::move(std::unique_ptr<DeferredTestData>());
  return 0;
}