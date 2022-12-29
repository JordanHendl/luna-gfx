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

struct DeferredRendererInfo {
  std::optional<WindowInfo> window_info;
  RenderPassInfo render_pass_info;
  GraphicsPipelineInfo pipeline_info;
};

class DeferredRenderer {
public:
  DeferredRenderer();
  DeferredRenderer(DeferredRendererInfo info);
  DeferredRenderer(DeferredRenderer&& mv) = default;
  ~DeferredRenderer();
  [[nodiscard]] auto next() -> CommandList&;
  [[nodiscard]] inline auto pass() const -> const RenderPass& {return this->m_pass;}
  [[nodiscard]] inline auto window() const -> const std::optional<Window>& {return this->m_window;}
  [[nodiscard]] inline auto pipeline() const -> const GraphicsPipeline& {return this->m_pipeline;}
private:
  MultiBuffered<CommandList> m_lists;
  std::optional<Window> m_window;
  GraphicsPipeline m_pipeline;
  RenderPass m_pass;
};
}
}