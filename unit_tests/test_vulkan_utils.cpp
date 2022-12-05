#include <gtest/gtest.h>
#include "luna-gfx/vulkan/utils/helper_functions.hpp"
#include "luna-gfx/vulkan/global_resources.hpp"
namespace luna::vulkan_utils_test {
TEST(VulkanUtils, CreateBuffer)
{
  constexpr auto cGPU = 0;
  constexpr auto cBufferSize = 1024u;
  constexpr auto cUsageFlags = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
  constexpr auto cMappable = true;
  
  auto& res = luna::vulkan::global_resources();
  auto index = luna::vulkan::create_buffer(cGPU, cBufferSize, cUsageFlags, cMappable);
  auto& buffer = res.buffers[index];

  EXPECT_GE(buffer.info.size, cBufferSize);

  luna::vulkan::destroy_buffer(index);

  EXPECT_EQ(buffer.info.size, 0);
  EXPECT_FALSE(buffer.buffer);
}
}
  
int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}