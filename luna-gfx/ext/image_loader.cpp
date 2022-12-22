#include "luna-gfx/ext/image_loader.hpp"
#include <stb_image.h>
#include <stb_image_write.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace luna {
namespace gfx {
auto getBytes(const char* path) -> std::vector<unsigned char> {
  auto input = std::ifstream();
  auto bytes = std::vector<unsigned char>();

  input.open(path, std::ios::binary);

  if (input) {
    // Copy stream's contents into char buffer.
    input.seekg(0, std::ios::end);
    bytes.reserve(input.tellg());
    input.seekg(0, std::ios::beg);

    bytes.assign((std::istreambuf_iterator<char>(input)),
                 std::istreambuf_iterator<char>());
  }

  if (bytes.empty())
    throw std::runtime_error(std::string("Failed to load file ") +
                             std::string(path));
  return bytes;
}

auto loadRGBA32F(const char* path) -> CPUImageInfo<float> {
  auto bytes = getBytes(path);
  return loadRGBA32F(bytes.size(), bytes.data());
}

auto loadRGB32F(const char* path) -> CPUImageInfo<float> {
  auto bytes = getBytes(path);
  return loadRGB32F(bytes.size(), bytes.data());
}

auto loadRG32F(const char* path) -> CPUImageInfo<float> {
  auto bytes = getBytes(path);
  return loadRG32F(bytes.size(), bytes.data());
}

auto loadR32F(const char* path) -> CPUImageInfo<float> {
  auto bytes = getBytes(path);
  return loadR32F(bytes.size(), bytes.data());
}

auto loadRGBA8(const char* path) -> CPUImageInfo<unsigned char> {
  auto bytes = getBytes(path);
  return loadRGBA8(bytes.size(), bytes.data());
}

auto loadRGB8(const char* path) -> CPUImageInfo<unsigned char> {
  auto bytes = getBytes(path);
  return loadRGB8(bytes.size(), bytes.data());
}

auto loadRG8(const char* path) -> CPUImageInfo<unsigned char> {
  auto bytes = getBytes(path);
  return loadRG8(bytes.size(), bytes.data());
}

auto loadR8(const char* path) -> CPUImageInfo<unsigned char> {
  auto bytes = getBytes(path);
  return loadR8(bytes.size(), bytes.data());
}

auto loadRGBA32F(unsigned num_bytes, const unsigned char* img_bytes)
    -> CPUImageInfo<float> {
  static constexpr int REQUIRED_CHANNELS = 4;

  CPUImageInfo<float> ret;
  int width = 0;
  int height = 0;
  int channels = 0;

  auto bytes = stbi_load_from_memory(img_bytes, num_bytes, &width, &height,
                                      &channels, REQUIRED_CHANNELS);
  if (!bytes)
    throw std::invalid_argument(std::string("Could not load file path "));

  const auto byte_size = width * height * REQUIRED_CHANNELS;
  
  ret.pixels.resize(byte_size);
  auto trans = [](unsigned char value) {
    return static_cast<float>(value) / static_cast<float>(std::numeric_limits<unsigned char>::max());
  };
  
  std::transform(bytes, bytes + byte_size, ret.pixels.begin(), trans);
//  std::copy(bytes, bytes + byte_size, ret.pixels.begin());

  ret.width = static_cast<unsigned>(width);
  ret.height = static_cast<unsigned>(height);
  ret.channels = static_cast<unsigned>(REQUIRED_CHANNELS);

  return ret;
}

auto loadRGB32F(unsigned num_bytes, const unsigned char* img_bytes)
    -> CPUImageInfo<float> {
  static constexpr int REQUIRED_CHANNELS = 3;

  CPUImageInfo<float> ret;
  int width = 0;
  int height = 0;
  int channels = 0;

  auto bytes = stbi_loadf_from_memory(img_bytes, num_bytes, &width, &height,
                                      &channels, REQUIRED_CHANNELS);
  if (!bytes)
    throw std::invalid_argument(std::string("Could not load file path "));

  const auto byte_size = width * height * REQUIRED_CHANNELS;

  ret.pixels.resize(byte_size);
  std::copy(bytes, bytes + byte_size, ret.pixels.begin());

  ret.width = static_cast<unsigned>(width);
  ret.height = static_cast<unsigned>(height);
  ret.channels = static_cast<unsigned>(REQUIRED_CHANNELS);

  return ret;
}

auto loadRG32F(unsigned num_bytes, const unsigned char* img_bytes)
    -> CPUImageInfo<float> {
  static constexpr int REQUIRED_CHANNELS = 2;

  CPUImageInfo<float> ret;
  int width = 0;
  int height = 0;
  int channels = 0;

  auto bytes = stbi_loadf_from_memory(img_bytes, num_bytes, &width, &height,
                                      &channels, REQUIRED_CHANNELS);
  if (!bytes)
    throw std::invalid_argument(std::string("Could not load file path "));

  const auto byte_size = width * height * REQUIRED_CHANNELS;

  ret.pixels.resize(byte_size);
  std::copy(bytes, bytes + byte_size, ret.pixels.begin());

  ret.width = static_cast<unsigned>(width);
  ret.height = static_cast<unsigned>(height);
  ret.channels = static_cast<unsigned>(REQUIRED_CHANNELS);

  return ret;
}

auto loadR32F(unsigned num_bytes, const unsigned char* img_bytes)
    -> CPUImageInfo<float> {
  static constexpr int REQUIRED_CHANNELS = 1;

  CPUImageInfo<float> ret;
  int width = 0;
  int height = 0;
  int channels = 0;

  auto bytes = stbi_loadf_from_memory(img_bytes, num_bytes, &width, &height,
                                      &channels, REQUIRED_CHANNELS);
  if (!bytes)
    throw std::invalid_argument(std::string("Could not load file path "));

  const auto byte_size = width * height * REQUIRED_CHANNELS;

  ret.pixels.resize(byte_size);
  std::copy(bytes, bytes + byte_size, ret.pixels.begin());

  ret.width = static_cast<unsigned>(width);
  ret.height = static_cast<unsigned>(height);
  ret.channels = static_cast<unsigned>(REQUIRED_CHANNELS);

  return ret;
}

auto loadRGBA8(unsigned num_bytes, const unsigned char* img_bytes)
    -> CPUImageInfo<unsigned char> {
  static constexpr int REQUIRED_CHANNELS = 4;

  CPUImageInfo<unsigned char> ret;
  int width = 0;
  int height = 0;
  int channels = 0;

  auto bytes = stbi_load_from_memory(img_bytes, num_bytes, &width, &height,
                                     &channels, REQUIRED_CHANNELS);
  if (!bytes)
    throw std::invalid_argument(std::string("Could not load file path "));

  const auto byte_size = width * height * REQUIRED_CHANNELS;

  ret.pixels.resize(byte_size);
  std::copy(bytes, bytes + byte_size, ret.pixels.begin());

  ret.width = static_cast<unsigned>(width);
  ret.height = static_cast<unsigned>(height);
  ret.channels = static_cast<unsigned>(REQUIRED_CHANNELS);

  return ret;
}

auto loadRGB8(unsigned num_bytes, const unsigned char* img_bytes)
    -> CPUImageInfo<unsigned char> {
  static constexpr int REQUIRED_CHANNELS = 3;

  CPUImageInfo<unsigned char> ret;
  int width = 0;
  int height = 0;
  int channels = 0;

  auto bytes = stbi_load_from_memory(img_bytes, num_bytes, &width, &height,
                                     &channels, REQUIRED_CHANNELS);
  if (!bytes)
    throw std::invalid_argument(std::string("Could not load file path "));

  const auto byte_size = width * height * REQUIRED_CHANNELS;

  ret.pixels.resize(byte_size);
  std::copy(bytes, bytes + byte_size, ret.pixels.begin());

  ret.width = static_cast<unsigned>(width);
  ret.height = static_cast<unsigned>(height);
  ret.channels = static_cast<unsigned>(REQUIRED_CHANNELS);

  return ret;
}

auto loadRG8(unsigned num_bytes, const unsigned char* img_bytes)
    -> CPUImageInfo<unsigned char> {
  static constexpr int REQUIRED_CHANNELS = 2;

  CPUImageInfo<unsigned char> ret;
  int width = 0;
  int height = 0;
  int channels = 0;

  auto bytes = stbi_load_from_memory(img_bytes, num_bytes, &width, &height,
                                     &channels, REQUIRED_CHANNELS);
  if (!bytes)
    throw std::invalid_argument(std::string("Could not load file path "));

  const auto byte_size = width * height * REQUIRED_CHANNELS;

  ret.pixels.resize(byte_size);
  std::copy(bytes, bytes + byte_size, ret.pixels.begin());

  ret.width = static_cast<unsigned>(width);
  ret.height = static_cast<unsigned>(height);
  ret.channels = static_cast<unsigned>(REQUIRED_CHANNELS);

  return ret;
}

auto loadR8(unsigned num_bytes, const unsigned char* img_bytes)
    -> CPUImageInfo<unsigned char> {
  static constexpr int REQUIRED_CHANNELS = 1;

  CPUImageInfo<unsigned char> ret;
  int width = 0;
  int height = 0;
  int channels = 0;

  auto bytes = stbi_load_from_memory(img_bytes, num_bytes, &width, &height,
                                     &channels, REQUIRED_CHANNELS);
  if (!bytes)
    throw std::invalid_argument(std::string("Could not load file path "));

  const auto byte_size = width * height * REQUIRED_CHANNELS;

  ret.pixels.resize(byte_size);
  std::copy(bytes, bytes + byte_size, ret.pixels.begin());

  ret.width = static_cast<unsigned>(width);
  ret.height = static_cast<unsigned>(height);
  ret.channels = static_cast<unsigned>(REQUIRED_CHANNELS);

  return ret;
}

auto savePNG(const CPUImageInfo<float>& info, const char* path) -> void {
  auto tmp = std::vector<unsigned char>();
  const auto& bytes = info.pixels;

  auto func = [](float f) -> unsigned char {
    return static_cast<unsigned char>(
        f * static_cast<float>(std::numeric_limits<unsigned char>::max()));
  };

  tmp.resize(bytes.size());
  std::fill(tmp.begin(), tmp.end(), 0);
  std::transform(bytes.begin(), bytes.end(), tmp.begin(), func);

  stbi_write_png(path, info.width, info.height, info.channels, tmp.data(), 0);
}

auto savePNG(const CPUImageInfo<unsigned char>& info, const char* path) -> void {
  const auto& bytes = info.pixels;
  stbi_write_png(path, info.width, info.height, info.channels, bytes.data(), 0);
}
}
}