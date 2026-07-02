cmake_minimum_required(VERSION 3.20)
project(nuke_ndk_custom_node LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set this to your Nuke install root if not provided by environment.
# Example Windows path: C:/Program Files/Nuke16.0v1
set(NUKE_ROOT "$ENV{NUKE_ROOT}" CACHE PATH "Path to Nuke installation root")
set(NUKE_VERSION "$ENV{NUKE_VERSION}" CACHE STRING "Nuke version folder name used in plugin output paths")

if(NOT NUKE_ROOT)
  message(FATAL_ERROR "NUKE_ROOT is not set. Set env var NUKE_ROOT or pass -DNUKE_ROOT=... to CMake.")
endif()

if(NOT NUKE_VERSION)
  get_filename_component(NUKE_VERSION "${NUKE_ROOT}" NAME)
endif()

set(NUKE_INCLUDE_DIR "${NUKE_ROOT}/include")
if(NOT EXISTS "${NUKE_INCLUDE_DIR}/DDImage/Iop.h")
  message(FATAL_ERROR "Could not find NDK headers under ${NUKE_INCLUDE_DIR}. Check NUKE_ROOT.")
endif()

file(GLOB NODE_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(PassThroughLite MODULE ${NODE_SOURCES})

target_include_directories(PassThroughLite PRIVATE "${NUKE_INCLUDE_DIR}" include)
if(WIN32)
  target_compile_definitions(PassThroughLite PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
endif()

# Nuke plugin binaries are loaded from plugin paths added to NUKE_PATH.
set_target_properties(PassThroughLite PROPERTIES
  PREFIX ""
  OUTPUT_NAME "PassThroughLite"
)

# Put the built plugin in build/plugin/<os>/<nuke-version> for easy testing.
set(PLUGIN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/plugin/${CMAKE_SYSTEM_NAME}/${NUKE_VERSION}")

set_target_properties(PassThroughLite PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
  RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

# Also stage a ready-to-copy package directory per platform/arch.
set(PACKAGE_PLATFORM_DIR "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(PACKAGE_PLUGIN_DIR "${CMAKE_BINARY_DIR}/package/${PACKAGE_PLATFORM_DIR}")

add_custom_command(TARGET PassThroughLite POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "${PACKAGE_PLUGIN_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:PassThroughLite>" "${PACKAGE_PLUGIN_DIR}/"
  COMMENT "Staging plugin in ${PACKAGE_PLUGIN_DIR}"
)

add_custom_target(package_plugin
  DEPENDS PassThroughLite
)

if(MSVC)
  target_compile_options(PassThroughLite PRIVATE /W4 /EHsc)
else()
  set_target_properties(PassThroughLite PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_compile_options(PassThroughLite PRIVATE -Wall -Wextra -Wpedantic)
endif()

message(STATUS "Nuke version folder: ${NUKE_VERSION}")
message(STATUS "Plugin test output: ${PLUGIN_OUTPUT_DIR}")
message(STATUS "Plugin package output: ${PACKAGE_PLUGIN_DIR}")
