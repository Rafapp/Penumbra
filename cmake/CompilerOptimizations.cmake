# CompilerOptimizations.cmake
# Sets up compiler optimizations for maximum performance
# Usage: include(cmake/CompilerOptimizations.cmake)
#        set_target_optimizations(my_target)
#
# Control with: cmake .. -DENABLE_OPTIMIZATIONS=ON

option(ENABLE_OPTIMIZATIONS "Enable maximum O3 optimizations" ON)

function(set_target_optimizations target)
  if(NOT TARGET ${target})
    message(FATAL_ERROR "set_target_optimizations: Target '${target}' does not exist")
  endif()
  
  if(ENABLE_OPTIMIZATIONS)
    if(MSVC)
      target_compile_options(${target} PRIVATE /O2 /Ob2)
      message(STATUS "Optimizations enabled for ${target}: /O2 /Ob2")
    else()
      # Check if compiler supports these flags
      include(CheckCXXCompilerFlag)
      
      check_cxx_compiler_flag("-O3" SUPPORTS_O3)
      check_cxx_compiler_flag("-march=native" SUPPORTS_MARCH_NATIVE)
      check_cxx_compiler_flag("-flto" SUPPORTS_LTO)
      check_cxx_compiler_flag("-ffast-math" SUPPORTS_FAST_MATH)
      
      if(SUPPORTS_O3)
        target_compile_options(${target} PRIVATE -O3)
      else()
        message(STATUS "Skipping -O3, compiler does not support it, using -O2")
        target_compile_options(${target} PRIVATE -O2)
      endif()
      
      if(SUPPORTS_MARCH_NATIVE)
        target_compile_options(${target} PRIVATE -march=native)
      else()
        message(STATUS "Skipping -march=native, compiler does not support it")
      endif()
      
      if(SUPPORTS_LTO)
        target_compile_options(${target} PRIVATE -flto)
        target_link_options(${target} PRIVATE -flto)
      else()
        message(STATUS "Skipping -flto, compiler does not support it")
      endif()
      
      if(SUPPORTS_FAST_MATH)
        target_compile_options(${target} PRIVATE -ffast-math)
      else()
        message(STATUS "Skipping -ffast-math, compiler does not support it")
      endif()
      
      message(STATUS "Optimizations enabled for ${target}: -O3 -march=native -flto -ffast-math")
    endif()
  else()
    message(STATUS "Optimizations disabled for ${target}")
  endif()
endfunction()
