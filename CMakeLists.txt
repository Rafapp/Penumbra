cmake_minimum_required(VERSION 3.5...3.20)
project(Penumbra)

# === C++ Standard ===
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# === Build Type ===
if(NOT CMAKE_BUILD_TYPE) 
  set(CMAKE_BUILD_TYPE Release)
endif()

# === Platform ===
# Apple (homebrew)
# TODO: Add other package managers
if(APPLE)
  set(CMAKE_PREFIX_PATH "/opt/homebrew" ${CMAKE_PREFIX_PATH})
  set(CMAKE_FIND_FRAMEWORK LAST)
  set(CMAKE_FIND_APPBUNDLE NEVER)
  message(STATUS "CMAKE_PREFIX_PATH set to: ${CMAKE_PREFIX_PATH}")
endif()

# === CMake Modules ===
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(cmake/CompilerOptimizations.cmake)

# === External Libraries ===
set(FETCHCONTENT_QUIET OFF)
message(STATUS "FetchContent logging enabled")
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

# Minipbrt - read .pbrt scenes
FetchContent_Declare(
  minipbrt
  GIT_REPOSITORY https://github.com/vilya/minipbrt.git
  GIT_TAG a6f9dadb55dae3a8a051c66863fe9b2622f8c7b3
)

# OpenImageIO - Image Input/Output 
find_package(OpenImageIO REQUIRED)

# TinyBVH - Acceleration Structures
FetchContent_Declare(tinybvh
  GIT_REPOSITORY https://github.com/jbikker/tinybvh.git
  GIT_TAG 4b5b649f1193374762b0247ee20de3345633bf9b 
)

# GLM - Math
FetchContent_Declare(glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.2 
)

# GLFW - Window, input, graphics
FetchContent_Declare(glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
)

# ImGui - GUI
FetchContent_Declare(imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.89.9
)

FetchContent_MakeAvailable(assimp minipbrt tinybvh glm glfw imgui)

# Fix the typo in main.cpp for minipbrt (put a PR already)
file(READ ${minipbrt_SOURCE_DIR}/main.cpp contents)
string(REPLACE "cropwwindow" "cropwindow" contents "${contents}")
file(WRITE ${minipbrt_SOURCE_DIR}/main.cpp "${contents}")

# === Options ===
option(PATHTRACER_HEADLESS "Build headless version" OFF)
option(ENABLE_VERIFY_TESTS "Build library verification tests" ON)

# === Inlcude ImGui library with GLFW backend ===
add_library(imgui
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw)

# == Include Glad ==
add_library(glad STATIC
  ${CMAKE_SOURCE_DIR}/third_party/glad/glad.c
)
target_include_directories(glad PUBLIC
  ${CMAKE_SOURCE_DIR}/third_party/glad
)

# === Library Verification Tests ===
if(ENABLE_VERIFY_TESTS)
  enable_testing()
  
  add_executable(verify_libraries tests/verify_libraries.cpp)
  
  target_include_directories(verify_libraries PRIVATE 
		${CMAKE_SOURCE_DIR}/penumbra/include
		${CMAKE_SOURCE_DIR}/third_party
		${glfw_SOURCE_DIR}/include
		${glm_SOURCE_DIR}
		${assimp_SOURCE_DIR}/include
		${minipbrt_SOURCE_DIR}
		${tinybvh_SOURCE_DIR}
		${imgui_SOURCE_DIR}
		${imgui_SOURCE_DIR}/backends
  )
  
  target_link_libraries(verify_libraries PRIVATE
		glfw glm assimp imgui OpenImageIO::OpenImageIO glad
  )
  
  add_test(NAME VerifyLibraries COMMAND verify_libraries)
endif()

# === Main Executable ===
file(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/penumbra/src/*.cpp)
file(GLOB HEADER_FILES ${CMAKE_SOURCE_DIR}/penumbra/include/*.h)
add_executable(penumbra ${SRC_FILES} ${HEADER_FILES})

target_sources(penumbra PRIVATE
	${minipbrt_SOURCE_DIR}/minipbrt.cpp
)

target_include_directories(penumbra PRIVATE
  ${CMAKE_SOURCE_DIR}/penumbra/include
	${CMAKE_SOURCE_DIR}/third_party
  ${glfw_SOURCE_DIR}/include
  ${glm_SOURCE_DIR}
  ${assimp_SOURCE_DIR}/include
	${minipbrt_SOURCE_DIR}
  ${tinybvh_SOURCE_DIR}
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)

# Silence TinyBVH warnings
target_include_directories(penumbra SYSTEM PRIVATE
    ${tinybvh_SOURCE_DIR}
)

target_link_libraries(penumbra PRIVATE 
	glfw glm assimp imgui OpenImageIO::OpenImageIO glad
)

# macOS requires Cocoa framework
if(APPLE)
	target_compile_definitions(penumbra PRIVATE GL_SILENCE_DEPRECATION)
  target_link_libraries(penumbra PRIVATE "-framework Cocoa")
endif()

if(NOT PATHTRACER_HEADLESS)
  target_compile_definitions(penumbra PRIVATE UI_ENABLED)
else()
  target_compile_definitions(penumbra PRIVATE HEADLESS_MODE)
endif()

# == Copy resources ==
add_custom_command(TARGET penumbra POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resources"
    $<TARGET_FILE_DIR:penumbra>/resources
)

# === Apply Optimizations ===
set_target_optimizations(penumbra)
