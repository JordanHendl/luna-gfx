#pragma once 
#include <string>
#include <vector>
namespace luna {
namespace gfx {
struct GPUInfo {
  std::string name;
  bool dedicated_card;
};

auto gpu_info() -> std::vector<GPUInfo>;
auto synchronize_gpu(int gpu) -> void;
}
}