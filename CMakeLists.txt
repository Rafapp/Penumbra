cmake_minimum_required(VERSION 3.20)

project(Penumbra)

# === C++ Standard ===
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# === Build Type ===
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# === CMake Modules ===
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(cmake/CompilerOptimizations.cmake)

# === Install external libraries ===
set(CMAKE_MESSAGE_LOG_LEVEL WARNING)
include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/libraries")

FetchContent_Declare(rgfw
  GIT_REPOSITORY https://github.com/ColleagueRiley/RGFW.git
  GIT_TAG main
)

FetchContent_Declare(glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 0.9.9.8
)

FetchContent_MakeAvailable(rgfw glm)

# === Options ===
option(PATHTRACER_HEADLESS "Build headless version" OFF)
option(ENABLE_VERIFY_TESTS "Build library verification tests" ON)

# === Library Verification Tests ===
if(ENABLE_VERIFY_TESTS)
  enable_testing()
  add_executable(verify_libraries tests/verify_libraries.cpp)
  
  # Include directories
  target_include_directories(verify_libraries PRIVATE 
    ${rgfw_SOURCE_DIR}
    ${glm_SOURCE_DIR}
  )
  
  target_link_libraries(verify_libraries PRIVATE glm)
  add_test(NAME VerifyLibraries COMMAND verify_libraries)
endif()
# === Main Executable ===
#add_executable(penumbra
#  src/main.cpp
#)

# target_link_libraries(penumbra PRIVATE rgfw glm)

#if(NOT PATHTRACER_HEADLESS)
#  target_compile_definitions(penumbra PRIVATE UI_ENABLED)
#else()
#  target_compile_definitions(penumbra PRIVATE HEADLESS_MODE)
#endif()

# === Apply Optimizations ===
# set_target_optimizations(penumbra)
