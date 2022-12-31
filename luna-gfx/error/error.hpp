#pragma once 
#include "luna-gfx/vulkan/vulkan_defines.hpp"
#include <vulkan/vulkan.hpp>
#include <cassert>
#include <iostream>
namespace luna {
namespace vulkan {

namespace assert_impl {
inline auto _luna_assert_impl_variadic() -> void {}

template <typename T>
inline auto _luna_assert_impl_variadic(const T& t) -> void {
     std::cout << t;
}
template <typename First, typename... Rest> 
inline auto _luna_assert_impl_variadic(const First& first, const Rest&... rest) -> void {
     std::cout << first;
    _luna_assert_impl(rest...); // recursive call using pack expansion syntax
}
template<typename ...Args>
inline auto _luna_assert_impl(Args ... args) {
  std::cout << "----- ERROR ---- Assertion failed: ";
  _luna_assert_impl_variadic(args...); 
  std::cout << "---- ERROR ----" << std::endl;
}
}

#define LunaAssert(cond, ...)                                  \
  if (!(cond)) {                                               \
    luna::vulkan::assert_impl::_luna_assert_impl(__VA_ARGS__); \
    assert(cond);                                              \
  }
//#else
//#define LunaAssert(cond, description)
//#endif

  /** Function to take in a vulkan error and handle it.
 * If it is a debug build, checks the error and throws.
 * Otherwise, this function is a simple inline passthrough.
 */
template <typename T>
inline auto error(vk::ResultValue<T> result) {
  LunaAssert(result.result == vk::Result::eSuccess,
            vk::to_string(result.result));
  return result.value;
}

/** Similar to the above function, this one handles vulkan results.
 * In a debug build, checks error and throws if it is a failure
 * Otherwise, this function is a NOP and should get compiled out.
 */
inline auto error(vk::Result result) {
  LunaAssert(result == vk::Result::eSuccess, vk::to_string(result));
}
}
}