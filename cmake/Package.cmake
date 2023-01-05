macro(ConfigurePackaging)
  set(Major "0" CACHE STRING "The major version of this build.")
  set(Minor "1" CACHE STRING "The minor version of this build.")
  set(Branch "0" CACHE STRING "The branch version of this build.")
  set(Revision "0" CACHE STRING "The revision of this build.")

  set(ProjectVersion "${Major}.${Minor}.${Branch}.${Revision}")

  if(WIN32)
    include(PackageWin)
  elseif(UNIX)
    # For UNIX, we need to find out what distro we're on to find out what kind of package we need to create (DEB or RPM)
    find_program(LSB_RELEASE_EXEC lsb_release)
    execute_process(COMMAND ${LSB_RELEASE_EXEC} -is
      OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    # For Ubuntu/Debian, package a .deb
    if(LSB_RELEASE_ID_SHORT STREQUAL "Ubuntu" OR LSB_RELEASE_ID_SHORT STREQUAL "Debian")
      include(PackageDeb)
    endif()
  
    #TODO: Handle RPM Packaging here.
  endif()

  set(CMAKE_INSTALL_RPATH   ${CPACK_PACKAGING_INSTALL_PREFIX}/${EXPORT_LIB_DIR})
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
  set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
  set(CPACK_PACKAGE_NAME    ${CMAKE_PROJECT_NAME})
  set(CPACK_COMPONENTS_ALL release devel)
  set(CPACK_COMPONENT_DEVEL_DISPLAY_NAME "${CMAKE_PROJECT_NAME}-devel")
  set(CPACK_COMPONENT_RELEASE_DISPLAY_NAME "${CMAKE_PROJECT_NAME}-release")
  set(CPACK_COMPONENT_DEVEL_DESCRIPTION "Development Headers & Runtime Libraries for the ${CMAKE_PROJECT_NAME} library")
  set(CPACK_COMPONENT_RELEASE_DESCRIPTION "Runtime Libraries for the ${CMAKE_PROJECT_NAME} library")
  set(CPACK_COMPONENT_DEVEL_DEPENDS release)
  set(CPACK_COMPONENT_DEVEL_REQUIRED release)
  set(CPACK_RPM_DEVEL_PACKAGE_REQUIRES ${CMAKE_PROJECT_NAME}-release) 
  set(CPACK_RPM_TOOLS_PACKAGE_REQUIRES ${CMAKE_PROJECT_NAME}-release) 
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "Luna")

  # Configure the platform/generator-specific parameters.
  ConfigurePlatformPackaging() # From Package<Platform>.cmake

  include (CMakePackageConfigHelpers)
  write_basic_package_version_file (
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
        VERSION ${ProjectVersion}
        COMPATIBILITY AnyNewerVersion
  )

  # Finally, include CPack
  include(CPack)
endmacro()

macro(ExportPackage)
  #install(TARGETS gfx gfx_interface vulkan_impl gfx_common vma_lib spirv_reflect gfx_extended stb_tools
  install(TARGETS gfx gfx_interface vulkan_impl gfx_common vma_lib math spirv_reflect gfx_extended stb_tools
          EXPORT luna-gfx COMPONENT release
          ARCHIVE  DESTINATION ${EXPORT_LIB_DIR}
          RUNTIME  DESTINATION ${EXPORT_LIB_DIR}
          LIBRARY  DESTINATION ${EXPORT_LIB_DIR}
          INCLUDES DESTINATION ${EXPORT_INCLUDE_DIR} )

  install(EXPORT luna-gfx FILE luna-gfx-config.cmake 
                       DESTINATION cmake 
                       NAMESPACE   luna::
                       COMPONENT   devel )
endmacro()