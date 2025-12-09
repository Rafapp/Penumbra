# TODO: Fix
set(CMAKE_PREFIX_PATH "C:/Users/rpadi/Documents/Dev/Penumbra/buildscripts/vcpkg/installed/x64-windows" ${CMAKE_PREFIX_PATH})

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

# Enable parallel builds
include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
  set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
  message(STATUS "Building with ${N} parallel jobs") 
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
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG v6.0.2
  CMAKE_ARGS 
    -DASSIMP_BUILD_TESTS=OFF 
    -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
    -DASSIMP_BUILD_MDL_IMPORTER=OFF
    -DASSIMP_INSTALL=OFF
    -DBUILD_SHARED_LIBS=ON
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
  CMAKE_ARGS
    -DBUILD_SPEEDTEST=OFF
    -DBUILD_SHARED_LIBS=OFF
)

# GLM - Math
FetchContent_Declare(glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.2 
  CMAKE_ARGS
    -DBUILD_SHARED_LIBS=OFF
)

# GLFW - Window, input, graphics
FetchContent_Declare(glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
  CMAKE_ARGS
    -DGLFW_BUILD_DOCS=OFF
    -DGLFW_BUILD_TESTS=OFF
    -DGLFW_BUILD_EXAMPLES=OFF
    -DBUILD_SHARED_LIBS=ON
)

# ImGui - GUI
FetchContent_Declare(imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.89.9
)

FetchContent_MakeAvailable(assimp minipbrt glm glfw imgui)

# Fix the typo in main.cpp for minipbrt (put a PR already)
file(READ ${minipbrt_SOURCE_DIR}/main.cpp contents)
string(REPLACE "cropwwindow" "cropwindow" contents "${contents}")
file(WRITE ${minipbrt_SOURCE_DIR}/main.cpp "${contents}")

# === Options ===
option(PATHTRACER_HEADLESS "Build headless version" OFF)
option(ENABLE_VERIFY_TESTS "Build library verification tests" ON)

# === Inlcude ImGui library with GLFW backend ===
add_library(imgui STATIC
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
		glfw assimp imgui OpenImageIO::OpenImageIO glad
  )

	if (WIN32)
		set(_runtime_dlls
			"${assimp_BINARY_DIR}/bin/Release/assimp-vc143-mt.dll"
			"${glfw_BINARY_DIR}/src/Release/glfw3.dll"
			"${CMAKE_PREFIX_PATH}/bin/OpenImageIO.dll"
			"${CMAKE_PREFIX_PATH}/bin/OpenImageIO_Util.dll"
			"${CMAKE_PREFIX_PATH}/bin/OpenEXR-3_4.dll"
			"${CMAKE_PREFIX_PATH}/bin/IlmThread-3_4.dll"
			"${CMAKE_PREFIX_PATH}/bin/Iex-3_4.dll"
			"${CMAKE_PREFIX_PATH}/bin/OpenEXRCore-3_4.dll"
			"${CMAKE_PREFIX_PATH}/bin/Imath-3_2.dll"
			"${CMAKE_PREFIX_PATH}/bin/deflate.dll"
			"${CMAKE_PREFIX_PATH}/bin/openjph.0.25.dll"
		)

		add_custom_command(
			TARGET verify_libraries POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E echo "-- Copying runtime DLLs for verify_libraries"
		)

		foreach(dll IN LISTS _runtime_dlls)
			add_custom_command(
				TARGET verify_libraries POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
					"${dll}"
					"$<TARGET_FILE_DIR:verify_libraries>"
			)
		endforeach()
	endif()
  add_test(NAME VerifyLibraries COMMAND verify_libraries)
endif() 

# === Main Executable ===
file(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/penumbra/src/*.cpp)
list(APPEND SRC_FILES ${CMAKE_SOURCE_DIR}/penumbra/src/threading.cpp)
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
	glfw assimp imgui OpenImageIO::OpenImageIO glad
)

if (WIN32)
    set(_runtime_dlls
        "${assimp_BINARY_DIR}/bin/Release/assimp-vc143-mt.dll"
        "${glfw_BINARY_DIR}/src/Release/glfw3.dll"
        "${CMAKE_PREFIX_PATH}/bin/OpenImageIO.dll"
        "${CMAKE_PREFIX_PATH}/bin/OpenImageIO_Util.dll"
        "${CMAKE_PREFIX_PATH}/bin/OpenEXR-3_4.dll"
        "${CMAKE_PREFIX_PATH}/bin/IlmThread-3_4.dll"
        "${CMAKE_PREFIX_PATH}/bin/Iex-3_4.dll"
        "${CMAKE_PREFIX_PATH}/bin/OpenEXRCore-3_4.dll"
        "${CMAKE_PREFIX_PATH}/bin/Imath-3_2.dll"
        "${CMAKE_PREFIX_PATH}/bin/deflate.dll"
        "${CMAKE_PREFIX_PATH}/bin/openjph.0.25.dll"
    )

    add_custom_command(
        TARGET penumbra POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "-- Copying runtime DLLs for penumbra"
    )

    foreach(dll IN LISTS _runtime_dlls)
        add_custom_command(
            TARGET penumbra POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${dll}"
                "$<TARGET_FILE_DIR:penumbra>"
        )
    endforeach()
endif()

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
