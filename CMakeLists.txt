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

# === External Libraries ===
set(CMAKE_MESSAGE_LOG_LEVEL WARNING)
include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/libraries")

# Assimp - 3D Model Importing 
FetchContent_Declare(assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG v6.0.2 
  CMAKE_ARGS 
    -DASSIMP_BUILD_TESTS=OFF 
    -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
    -DASSIMP_BUILD_MDL_IMPORTER=OFF # Unstable
)

# OpenImageIO - Image Input/Output 
find_package(OpenImageIO REQUIRED)

# TinyBVH - Acceleration Structures
FetchContent_Declare(tinybvh
  GIT_REPOSITORY https://github.com/jbikker/tinybvh.git
  GIT_TAG main
)

# GLM - Math
FetchContent_Declare(glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 0.9.9.8
)

# RGFW - Window, input, graphics
FetchContent_Declare(rgfw
  GIT_REPOSITORY https://github.com/ColleagueRiley/RGFW.git
  GIT_TAG main
)

# ImGui - GUI
FetchContent_Declare(imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.89.9
)

FetchContent_MakeAvailable(assimp tinybvh glm rgfw imgui)

# === Options ===
option(PATHTRACER_HEADLESS "Build headless version" OFF)
option(ENABLE_VERIFY_TESTS "Build library verification tests" ON)

# === Library Verification Tests ===
if(ENABLE_VERIFY_TESTS)
  enable_testing()
  
  # Build ImGui library
  add_library(imgui
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
  )
  target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
  
  add_executable(verify_libraries tests/verify_libraries.cpp)
  
  # Include directories
  target_include_directories(verify_libraries PRIVATE 
    ${rgfw_SOURCE_DIR}
    ${glm_SOURCE_DIR}
    ${assimp_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}
  )
  
  target_link_libraries(verify_libraries PRIVATE glm assimp imgui)
  add_test(NAME VerifyLibraries COMMAND verify_libraries)
endif()
# === Main Executable ===
#add_executable(penumbra
#  src/main.cpp
#)
#
#target_include_directories(penumbra PRIVATE
#  ${CMAKE_SOURCE_DIR}/include
#  ${rgfw_SOURCE_DIR}
#  ${glm_SOURCE_DIR}
#  ${assimp_SOURCE_DIR}/include
#  ${openimageio_SOURCE_DIR}/src/include
#  ${tinybvh_SOURCE_DIR}
#  ${imgui_SOURCE_DIR}
#  ${FFMPEG_INCLUDE_DIRS}
#)
#
#target_link_libraries(penumbra PRIVATE 
#  rgfw glm assimp openimageio imgui ${FFMPEG_LIBRARIES}
#)
#
#if(NOT PATHTRACER_HEADLESS)
#  target_compile_definitions(penumbra PRIVATE UI_ENABLED)
#else()
#  target_compile_definitions(penumbra PRIVATE HEADLESS_MODE)
#endif()
#
# === Apply Optimizations ===
#set_target_optimizations(penumbra)