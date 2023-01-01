#pragma once
#include "luna-gfx/common/shader.hpp"
#include "luna-gfx/interface/bind_group.hpp"
#include "luna-gfx/interface/render_pass.hpp"
#include <cstdint>
#include <vector>
#include <string>
#include <variant>
namespace luna {
namespace gfx {

/** Enum class describing the way this pipeline interprets the incoming vertices on any sort of Graphicsing operation.
 */
enum class Topology : int {
  Point,
  Line,
  LineStrip,
  Triangle,
  TriangleStrip,
};

/** Structure to describe a viewport into this pipeline's scene.
 */
struct Viewport {
  float width = 1280;
  float height = 1024;
  std::size_t x_pos = 0;
  std::size_t y_pos = 0;
  float max_depth = 1.0f;
};

/** Structure to describe the specific details of a graphics pipeline. For use of advanced users who want to fine-tune their Graphicsing.
*/
struct GraphicsPipelineDetails {
  // Whether the pipeline should perform a stencil test.
  bool stencil_test = false; 

  // Whether this pipeline should perform a depth test.
  bool depth_test = false; 

  // The way this pipeline interprets incoming vertices for Graphicsing. Defaults to triangles.
  Topology topology = Topology::Triangle;
};

struct GraphicsPipelineInfo {
  using SPIRV = std::vector<std::uint32_t>;
  using PreLoadedFile = std::vector<char>;
  using Filename = std::string;

  // Either a SPIRV buffer, an inline-glsl shader, or a filename to request the library to load.
  using ShaderData = std::variant<SPIRV, PreLoadedFile, Filename>;

  struct ShaderInfo {
    std::string name;
    gfx::ShaderType type;
    ShaderData data;
  };

  // What gpu to allocate this data on.
  int gpu = 0;

  // The shader data that actually describes each part of this pipeline.
  std::vector<ShaderInfo> shaders;

  // The initial viewport for this pipeline.
  Viewport initial_viewport;

  // The extra details of this pipeline. For advanced users.
  GraphicsPipelineDetails details;

  // Name for debugging purposes.
  std::string name = "Pipeline";

  // Name of the subpass this object writes to.
  std::string subpass = "Default";
};

struct ComputePipelineInfo {
  using SPIRV = std::vector<std::uint32_t>;

  // Either a SPIRV buffer, an inline-glsl shader, or a filename to request the library to load.
  using ShaderData = std::variant<SPIRV, std::vector<char>, std::string>;

  struct ShaderInfo {
    std::string name;
    gfx::ShaderType type;
    ShaderData data;
  };

  // What gpu to allocate this data on.
  int gpu = 0;

  // The shader data that actually describes this pipeline.
  ShaderInfo shaders;
};

class GraphicsPipeline {
public:
  GraphicsPipeline(const GraphicsPipeline& cpy) = delete;
  auto operator=(const GraphicsPipeline& cpy) -> GraphicsPipeline& = delete;

  GraphicsPipeline() {this->m_handle = -1;}
  GraphicsPipeline(const RenderPass& pass, GraphicsPipelineInfo info, int subpass = 0);
  GraphicsPipeline(GraphicsPipeline&& mv) {*this = std::move(mv);}
  ~GraphicsPipeline();

  [[nodiscard]] auto create_bind_group() -> BindGroup;
  [[nodiscard]] inline auto handle() const {return this->m_handle;}
  [[nodiscard]] inline auto info() const {return this->m_info;}
  auto operator=(GraphicsPipeline&& mv) -> GraphicsPipeline& {this->m_handle = mv.m_handle; mv.m_handle = -1; this->m_info = mv.m_info; return *this;};
  private:
    std::int32_t m_handle;
    GraphicsPipelineInfo m_info;
};
}
}