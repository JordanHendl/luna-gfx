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

        set(src_file ${SHADER_DIR}/${file_name}_${extension}.hpp)
        set(compile_command glslangValidator -g -V -o ${file_name}_${extension}.hpp --vn ${file_name}_${extension} ${CMAKE_CURRENT_SOURCE_DIR}/${ARG})
        set(new_target ${ARG}_compiled)
        # Compile SPIRV
        add_custom_command(
            POST_BUILD
            OUTPUT ${src_file}
            COMMAND ${compile_command}
            WORKING_DIRECTORY ${SHADER_DIR}
            DEPENDS ${ARG}
          )

          add_custom_target(
            ${new_target} ALL
            DEPENDS ${src_file}
          )

          add_dependencies(shader_compilation ${ARG}_compiled)
        endif()
      endforeach()
    endif()
  endif()
endfunction()