#pragma once
#include "luna-gfx/gfx.hpp"
#include "luna-gfx/ext/multi_buffered.hpp"
#include <unordered_map>
#include <optional>
#include <string>
namespace luna {
namespace gfx {
struct ForwardRendererInfo {

};

struct RendererInfo {
  RenderPassInfo render_pass_info;
  std::map<std::string, GraphicsPipelineInfo> pipeline_infos;
};

class Renderer {
public:
  Renderer(const Renderer& cpy) = delete;
  auto operator=(const Renderer& cpy) -> Renderer& = delete;

  inline Renderer() = default;
  inline Renderer(RendererInfo info) {
    this->m_pass = std::move(RenderPass(info.render_pass_info));
    for(auto& info : info.pipeline_infos) {
      this->m_pipelines[info.first] = std::move(GraphicsPipeline(this->m_pass, info.second));
    }
    this->m_info = info;
    this->m_lists = gfx::MultiBuffered<gfx::CommandList>(info.render_pass_info.gpu);
  }

  Renderer(Renderer&& mv) = default;
  ~Renderer() = default;
  [[nodiscard]] auto info() {return this->m_info;}
  [[nodiscard]] auto next() -> CommandList& {
    auto& tmp = *this->m_lists;
    this->m_lists.advance();
    return tmp;
  }

  [[nodiscard]] inline auto pass() const -> const RenderPass& {return this->m_pass;}
  [[nodiscard]] inline auto pipeline(std::string name) -> GraphicsPipeline& {return this->m_pipelines.at(name);}
  auto operator=(Renderer&& mv) -> Renderer& = default;
private:
  RendererInfo m_info;
  MultiBuffered<CommandList> m_lists;
  RenderPass m_pass;
  std::map<std::string, GraphicsPipeline> m_pipelines;
};
}
}