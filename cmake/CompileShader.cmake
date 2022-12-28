# Function to help manage building tests.
function(compile_shader)
  CMAKE_POLICY(SET CMP0057 NEW)
  
  # Test configurations.
  set(VARIABLES
    TARGETS
    INCLUDE_DIR
  )

  set(SHOULD_REBUILD FALSE)

  # For each argument provided.
  foreach(ARG ${ARGV})
    # If argument is one of the variables, set it.
    if(${ARG} IN_LIST VARIABLES)
      SET(STATE ${ARG})
    else()
      # If our state is a variable, set that variables value
      if("${${STATE}}")
        set(${STATE} ${ARG})
      else()
        list(APPEND ${STATE} ${ARG})
      endif()

      # If our state is a setter, set the value in the parent scope as well
      if("${STATE}" IN_LIST CONFIGS)
        set(${STATE} ${${STATE}} PARENT_SCOPE)
      endif()
    endif()
  endforeach()

  if(TARGETS)
    find_package(Vulkan)

    if(${Vulkan_FOUND})
      # Compile SPIRV for each target.
      foreach(ARG ${TARGETS})
        # Check if we can build spirv with glslang validator.
        find_program(HAS_GLSLANG_VALIDATOR "glslangValidator")
        get_filename_component(file_name ${ARG} NAME_WLE)
        get_filename_component(extension ${ARG} LAST_EXT)
        get_filename_component(dir_name ${ARG} DIRECTORY)

        string(REPLACE "." "" extension ${extension})

        # Build Spirv
        if(HAS_GLSLANG_VALIDATOR)
        message( "glslangValidator -V -o ${file_name}_${extension}.hpp --vn ${file_name} ${CMAKE_CURRENT_SOURCE_DIR}/${ARG}")
        # Compile SPIRV
        add_custom_command(
            POST_BUILD
            OUTPUT ${file_name}_${extension}.hpp
            COMMAND glslangValidator -V -o ${file_name}_${extension}.hpp --vn ${file_name}_${extension} ${CMAKE_CURRENT_SOURCE_DIR}/${ARG} 
            WORKING_DIRECTORY ${SHADER_DIR}
            DEPENDS ${ARG}
          )

          add_custom_target(
            ${ARG}_compiled ALL
            DEPENDS ${file_name}_${extension}.hpp
          )
        endif()
      endforeach()
    endif()
  endif()
endfunction()