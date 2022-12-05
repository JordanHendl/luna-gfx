option(SanitizeAddress "Whether or not to build with address sanitizers." OFF)
option(SanitizeThread "Whether or not to build with thread sanitizers." OFF)

macro(AddSanitizers)
  if(SanitizeAddress AND SanitizeThread) 
    message(FATAL_ERROR "Unable to have both thread and address sanitizers enabled!")
  endif()

  if(SanitizeAddress)
    add_link_options(-fsanitize=address)
    add_compile_options(-fsanitize=address)
  endif()

  if(SanitizeThread)
    add_link_options(-fsanitize=thread)
    add_compile_options(-fsanitize=thread)
  endif()

  set(Safe_Sanitizers -fsanitize=null -fsanitize=signed-integer-overflow -fsanitize=bounds -fsanitize-address-use-after-scope
  -fsanitize=undefined)

  if(SanitizeThread OR SanitizeAddress)
    add_link_options(${Safe_Sanitizers})
    add_compile_options(${Safe_Sanitizers})
  endif()
endmacro()