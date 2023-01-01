set(common_headers
  dlloader.hpp
  shader.hpp
)

find_package(Threads REQUIRED)
add_library(gfx_common STATIC dlloader.cpp shader.cpp)
target_link_libraries(gfx_common PRIVATE ${CMAKE_DL_LIBS} spirv_reflect Vulkan::Headers shaderc::shaderc SDL2::SDL2)
target_include_directories(gfx_common PRIVATE ${spirv-reflect_include_dirs} Threads::Threads)

install(FILES ${common_headers} DESTINATION ${header_install_dir}/common COMPONENT devel)