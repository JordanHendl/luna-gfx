option(UseConan "Whether or not to use Conan for packages" ON)

macro(FindRequiredPackages)
  if(UseConan)
    execute_process( 
      COMMAND conan install ${CMAKE_SOURCE_DIR}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
  endif()

  find_package(Vulkan REQUIRED)
endmacro()