add_subdirectory(test_shaders)

if(WIN32)
  set(APP_FLAG WIN32)
endif()

set(TestDependancies
  GTest::GTest
  vulkan_impl
  gfx_common
  gfx
)
macro(AddTest test_file)
  get_filename_component(test_name ${test_file} NAME_WLE)
  add_executable(${test_name} ${test_file})
  target_link_libraries(${test_name} ${TestDependancies})
  target_include_directories(${test_name} PUBLIC ${SHADER_DIR})
  # Instead of knowing what each test uses for shaders, we just wait for all shaders to compile before compiling tests.
  add_dependencies(${test_name} shader_compilation)
  add_test(NAME ${test_name} COMMAND ${test_name})
endmacro()

if(BuildTests)
if(GTest_FOUND)
  set(luna_gfx_libs gfx)
  AddTest(test_interface.cpp)
  #AddTest(test_common.cpp)
  #AddTest(test_vulkan_wrappers.cpp)
  #AddTest(test_vulkan_utils.cpp)
  #AddTest(test_extension.cpp)
endif()

find_package(Threads)
add_executable(luna_cube luna_cube_test.cpp)
target_link_libraries(luna_cube gfx Threads::Threads)
target_include_directories(luna_cube PUBLIC ${SHADER_DIR})
add_dependencies(luna_cube shader_compilation)

add_executable(luna_deferred luna_deferred_test.cpp)
target_link_libraries(luna_deferred gfx Threads::Threads)
target_include_directories(luna_deferred PUBLIC ${SHADER_DIR})
add_dependencies(luna_deferred shader_compilation)
endif()