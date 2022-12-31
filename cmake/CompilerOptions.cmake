option(BuildRelease "Whether or not to build the release version of this project." ON)

macro(AddCompilerOptions)
  if(BuildRelease)
    #set(CMAKE_BUILD_TYPE Release)
    set(CMAKE_BUILD_TYPE Debug) # This is solely so asserts still work.
  else()
    set(CMAKE_BUILD_TYPE Debug) # This is solely so asserts still work.
  endif()


  if(BuildRelease)
    if(MSVC)
      add_compile_options(/W2 /MD)
    elseif(UNIX)
      add_compile_options(-g -Wall -Werror -fPIC -O2)
    endif()
  else()
    add_definitions(-DLunaGfxDebug)
    if(MSVC)
      add_compile_options(/W2 /DEBUG /MD)
    elseif(UNIX)
      add_compile_options(-Wall -Werror -fno-omit-frame-pointer -fPIC -g)
    endif()
  endif()
endmacro()