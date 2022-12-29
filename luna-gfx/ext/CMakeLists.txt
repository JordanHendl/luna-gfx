set(luna_gfx_ext_srcs 
  camera.cpp
  image_loader.cpp
)

add_library(luna_gfx_extended STATIC ${luna_gfx_ext_srcs})
target_include_directories(luna_gfx_extended PUBLIC ${stb_tools_include_dirs})
target_link_libraries(luna_gfx_extended PUBLIC ${stb_tools_libraries})