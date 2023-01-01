option(UseConan "Whether or not to use Conan for packages" ON)

macro(FindRequiredPackages)
  if(UseConan)
    include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
  endif()

  find_package(Vulkan REQUIRED)
  find_package(shaderc REQUIRED)
  find_package(glm REQUIRED)
  find_package(VulkanHeaders REQUIRED)
  find_package(vulkan-memory-allocator REQUIRED)
  find_package(SDL2 REQUIRED)
endmacro()