#include <gtest/gtest.h>
#include "luna-gfx/common/dlloader.hpp"
#include "luna-gfx/common/shader.hpp"
#include <utility>
#include <memory>
namespace luna::common_test {
TEST(CommonLibrary, DlloaderTest)
{
  const auto cSymbolName = "vkGetInstanceProcAddr";
  auto loader = luna::gfx::Dlloader();
#ifdef _WIN32
  loader.load("vulkan-1.dll");
#elif __linux__
  loader.load("libvulkan.so.1");
#endif

  EXPECT_TRUE(loader.initialized());
  auto symbol = loader.symbol(cSymbolName);
  EXPECT_TRUE(symbol);
}
}
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
 