#pragma once
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace luna {
namespace gfx {
class GraphicsPipelineInfo;
class ComputePipelineInfo;
inline namespace v1 {
/** Class to handle combining multiple glsl/hlsl shaders into a reflected binary
 * compacted with SPIR-V.
 */
class Shader {
 public:
  enum class Type : int {
    None,
    Vertex,
    Fragment,
    Geometry,
    TesselationControl,
    TesselationEval,
    Compute,
  };

  struct Stage {
    struct Attribute {
      enum class Type : int {
        Undefined,
        eInt,
        eFloat,
        eVec2,
        eIVec2,
        eVec3,
        eIVec3,
        eVec4,
        eIVec4,
        eMat2,
        eMat3,
        eMat4,
      };

      std::string name;
      Type type;
      size_t location;
    };

    struct Variable {
      enum class Type : int {
        None,
        Sampler,
        Sampled,
        Image,
        UTexel,
        STexel,
        Uniform,
        Storage,
        UniformDynamic,
        StorateDynamic,
        Input,
        Acceleration,
      };

      size_t set;
      size_t binding;
      size_t size;
      Type type;
    };

    Type type;
    std::string name;
    std::map<std::string, Variable> variables;
    std::vector<uint32_t> spirv;
    std::vector<Attribute> in_attributes;
    std::vector<Attribute> out_attributes;
  };

  explicit Shader();
  explicit Shader(const GraphicsPipelineInfo& info, std::vector<std::string> include_dirs = {});
  explicit Shader(const ComputePipelineInfo& info, std::vector<std::string> include_dirs = {});
  explicit Shader(Shader&& mv);
  ~Shader();
  auto operator=(Shader&& mv) -> Shader&;
  auto stages() const -> const std::vector<Stage>&;
  auto save(std::string_view path) -> bool;
  auto load(std::string_view path) -> bool;

 private:
  struct ShaderData;
  std::shared_ptr<ShaderData> data;
};

using ShaderType = Shader::Type;
using ShaderVariable = Shader::Stage::Variable;
using VariableType = Shader::Stage::Variable::Type;
using AttributeType = Shader::Stage::Attribute::Type;

auto to_string(ShaderType type) -> std::string;
}  // namespace v1
}  // namespace gfx
}  // namespace luna