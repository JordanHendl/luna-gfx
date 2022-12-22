set(ext_stbtools_name      stb_tools)
set(ext_stbtools_tar_dir   ${CMAKE_CURRENT_SOURCE_DIR}/${ext_stbtools_name}.tgz) 
set(ext_stbtools_untar_dir ${CMAKE_BINARY_DIR}/ext/${ext_stbtools_name}/src/)
set(ext_stbtools_bld_dir   ${CMAKE_BINARY_DIR}/ext/${ext_stbtools_name}/build)
set(ext_stbtools_src_dir   ${CMAKE_BINARY_DIR}/ext/${ext_stbtools_name}/src/${ext_stbtools_name}/)
set(ext_stbtools_cmake_dir ${ext_stbtools_src_dir})
set(ext_stbtools_incs      ${ext_stbtools_src_dir}/include)

if( UNIX )
  set(ext_stbtools_libs ${ext_stbtools_bld_dir}/libstb_tools.a)
  set(ext_stbtools_lib  ${ext_stbtools_bld_dir}/libstb_tools.a)
endif()

file(MAKE_DIRECTORY ${ext_stbtools_bld_dir})
file(MAKE_DIRECTORY ${ext_stbtools_src_dir})

execute_process( 
  COMMAND tar -xf ${ext_stbtools_tar_dir}
  WORKING_DIRECTORY ${ext_stbtools_untar_dir})

execute_process( 
  COMMAND ${CMAKE_COMMAND} ${ext_stbtools_cmake_dir}
  WORKING_DIRECTORY ${ext_stbtools_bld_dir})

if(WIN32)
  add_custom_command(
    PRE_BUILD
    OUTPUT ${ext_stbtools_libs}
    COMMAND MSBuild Stb_image.sln /property:Configuration=Release -maxcpucount:7
    WORKING_DIRECTORY ${ext_stbtools_bld_dir})
endif()

if(UNIX)
  add_custom_command(
    PRE_BUILD
    OUTPUT ${ext_stbtools_libs}
    COMMAND $(MAKE)
    WORKING_DIRECTORY ${ext_stbtools_bld_dir})
endif()

add_custom_target(
  EXT_Stb_Tools ALL
  DEPENDS ${ext_stbtools_libs})

set(stb_tools_libraries    ${ext_stbtools_libs} CACHE INTERNAL "stb tools libaries")
set(stb_tools_include_dirs ${ext_stbtools_incs} CACHE INTERNAL "stb tools include dirs")