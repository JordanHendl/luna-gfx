cmake_minimum_required(VERSION 3.13)
project(luna-gfx)

option(BuildBenchmarks "Whether or not to build benchmarks for this project." ON)
option(BuildTests "Whether or not to build tests for this project." ON)
option(RunTests "Whether or not to run tests after building this project." ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
enable_testing()
include(Sanitize)
include(CompilerOptions)
include(RunTest)

set(SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_DIR})

AddCompilerOptions() # From cmake/CompilerOptions.cmake
AddSanitizers() # From cmake/Sanitize.cmake

if(RunTests)
  RunTestsAfterBuild()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXPORT_BIN_DIR bin)
set(EXPORT_INCLUDE_DIR include)
set(EXPORT_LIB_DIR lib)
set(EXPORT_DOC_DIR doc)

# Set output directories.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${EXPORT_LIB_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${EXPORT_LIB_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${EXPORT_BIN_DIR})
set(DOCUMENTATION_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${EXPORT_DOC_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(external)
add_subdirectory(luna-gfx)
add_subdirectory(unit_tests)