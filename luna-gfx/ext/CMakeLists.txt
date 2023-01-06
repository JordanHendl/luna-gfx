set(luna_gfx_ext_headers
  camera.hpp
  image_loader.hpp
  ext.hpp
  math.hpp
  multi_buffered.hpp
  renderer.hpp
)

set(luna_gfx_ext_srcs 
  camera.cpp
  image_loader.cpp
)

add_library(math INTERFACE)
target_include_directories(math INTERFACE 
$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>)

add_library(gfx_extended STATIC ${luna_gfx_ext_srcs}) 
target_include_directories(gfx_extended PRIVATE ${stb_tools_include_dirs})
target_link_libraries(gfx_extended PRIVATE stb_tools glm::glm)

install(FILES ${luna_gfx_ext_headers} DESTINATION ${header_install_dir}/ext COMPONENT devel)