option(UseConan "Whether or not to use Conan for packages" ON)

macro(FindRequiredPackages)
  find_package(Vulkan REQUIRED)
  find_package(SDL2 REQUIRED)
endmacro()