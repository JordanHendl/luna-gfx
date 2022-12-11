find_package(Threads REQUIRED)
add_library(luna_gfx_common STATIC dlloader.cpp shader.cpp)
target_link_libraries(luna_gfx_common PUBLIC ${CMAKE_DL_LIBS} spirv_reflect shaderc_combined)
target_include_directories(luna_gfx_common PRIVATE ${spirv-reflect_include_dirs} Threads::Threads)