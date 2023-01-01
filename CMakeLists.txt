cmake_minimum_required(VERSION 3.13)
project(LunaGraphics)

option(BuildBenchmarks "Whether or not to build benchmarks for this project." ON)
option(BuildTests "Whether or not to build tests for this project." ON)
option(RunTests "Whether or not to run tests after building this project." ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
enable_testing()
include(Sanitize)
include(CompilerOptions)
include(RunTest)
include(FindDependencies)
include(Package)

set(SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_DIR})

FindRequiredPackages() # From cmake/FindPackages.cmake
AddCompilerOptions() # From cmake/CompilerOptions.cmake
AddSanitizers() # From cmake/Sanitize.cmake
ConfigurePackaging() # From Package.cmake

if(RunTests)
RunTestsAfterBuild()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXPORT_BIN_DIR bin)
set(EXPORT_INCLUDE_DIR include)
set(EXPORT_LIB_DIR lib)
set(EXPORT_DOC_DIR doc)

set(header_install_dir ${EXPORT_INCLUDE_DIR}/luna-gfx)

# Set output directories.
if(WIN32)
  add_definitions(-DLunaLibraryExport)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
  foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
      string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
      set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${EXPORT_LIB_DIR})
      set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${EXPORT_LIB_DIR})
      set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${EXPORT_LIB_DIR})
  endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${EXPORT_LIB_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${EXPORT_LIB_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${EXPORT_BIN_DIR})
set(DOCUMENTATION_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${EXPORT_DOC_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(external)
add_subdirectory(luna-gfx)
add_subdirectory(unit_tests)

ExportPackage() # From Package.cmake