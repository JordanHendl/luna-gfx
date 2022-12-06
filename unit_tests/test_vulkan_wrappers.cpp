#include <gtest/gtest.h>
#include "luna-gfx/vulkan/instance.hpp"
#include "luna-gfx/vulkan/device.hpp"
#include <utility>
#include <memory>
namespace luna::vulkan_wrappers_test {
TEST(VulkanWrappers, CreateInstance)
{
  auto loader = luna::gfx::Dlloader();
#ifdef _WIN32
  loader.load("vulkan-1.dll");
#elif __linux__
  loader.load("libvulkan.so.1");
#endif
  auto instance = luna::vulkan::Instance(loader, nullptr, "test");
  EXPECT_TRUE(instance.valid());
  EXPECT_FALSE(instance.m_devices.empty());

  {
    auto tmp = std::move(instance);
  }

  EXPECT_FALSE(instance.valid());
  EXPECT_TRUE(instance.m_devices.empty());
}

TEST(VulkanWrappers, CreateVirtualDevice) {
  auto loader = luna::gfx::Dlloader();
#ifdef _WIN32
  loader.load("vulkan-1.dll");
#elif __linux__
  loader.load("libvulkan.so.1");
#endif
  auto instance = luna::vulkan::Instance(loader, nullptr, "test");
  for(auto& dev : instance.m_devices) {
    auto device = luna::vulkan::Device(loader, nullptr, dev, instance, instance.m_dispatch);
    EXPECT_FALSE(device.properties.deviceName.empty());
  }
}
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
 