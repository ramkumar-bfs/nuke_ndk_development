cmake_minimum_required(VERSION 3.20)
project(nuke_ndk_custom_node LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set this to your Nuke install root if not provided by environment.
# Example path: D:/deps/NUKE_NDK
set(NUKE_NDK_ROOT "$ENV{NUKE_NDK_ROOT}" CACHE PATH "Path to shared Nuke NDK dependency root")
set(NUKE_VERSION "$ENV{NUKE_VERSION}" CACHE STRING "Nuke version folder name used in plugin output paths")

if(NOT NUKE_NDK_ROOT)
  message(FATAL_ERROR "NUKE_NDK_ROOT is not set. Set env var NUKE_NDK_ROOT or pass -DNUKE_NDK_ROOT=... to CMake.")
endif()

if(NOT NUKE_VERSION)
  message(FATAL_ERROR "NUKE_VERSION is not set. Set env var NUKE_VERSION or pass -DNUKE_VERSION=... to CMake.")
endif()

if(WIN32)
  set(NUKE_OS_TYPE "winx64")
elseif(UNIX AND NOT APPLE)
  set(NUKE_OS_TYPE "linux")
else()
  message(FATAL_ERROR "Unsupported platform for NUKE_NDK layout. Expected Windows or Linux.")
endif()

set(NUKE_DEP_ROOT "${NUKE_NDK_ROOT}/${NUKE_OS_TYPE}/${NUKE_VERSION}")
set(NUKE_INCLUDE_DIR "${NUKE_DEP_ROOT}/include")
set(NUKE_LIB_DIR "${NUKE_DEP_ROOT}/lib")

if(WIN32)
  set(NUKE_DDIMAGE_LIB "${NUKE_LIB_DIR}/DDImage.lib")
else()
  set(NUKE_DDIMAGE_LIB "${NUKE_LIB_DIR}/libDDImage.so")
endif()

if(NOT EXISTS "${NUKE_INCLUDE_DIR}/DDImage/Iop.h")
  message(FATAL_ERROR "Could not find NDK headers under ${NUKE_INCLUDE_DIR}. Check NUKE_NDK_ROOT/os_type/NUKE_VERSION layout.")
endif()

if(NOT EXISTS "${NUKE_LIB_DIR}")
  message(FATAL_ERROR "Could not find NDK libraries under ${NUKE_LIB_DIR}. Check NUKE_NDK_ROOT/os_type/NUKE_VERSION layout.")
endif()

if(NOT EXISTS "${NUKE_DDIMAGE_LIB}")
  message(FATAL_ERROR "Could not find the DDImage library at ${NUKE_DDIMAGE_LIB}. Check the copied NDK lib folder.")
endif()

file(GLOB NODE_SOURCES CONFIGURE_DEPENDS src/*.cpp)
if(NOT NODE_SOURCES)
  message(FATAL_ERROR "No node sources found in src/*.cpp")
endif()

# Put built plugins in build/plugin/<os>/<nuke-version> for easy testing.
set(PLUGIN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/plugin/${CMAKE_SYSTEM_NAME}/${NUKE_VERSION}")

# Stage ready-to-copy binaries per platform/arch.
set(PACKAGE_PLATFORM_DIR "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(PACKAGE_PLUGIN_DIR "${CMAKE_BINARY_DIR}/package/${PACKAGE_PLATFORM_DIR}")

set(PLUGIN_TARGETS)
foreach(NODE_SOURCE IN LISTS NODE_SOURCES)
  get_filename_component(NODE_NAME "${NODE_SOURCE}" NAME_WE)
  add_library(${NODE_NAME} MODULE "${NODE_SOURCE}")

  target_include_directories(${NODE_NAME} PRIVATE "${NUKE_INCLUDE_DIR}" include)
  target_link_directories(${NODE_NAME} PRIVATE "${NUKE_LIB_DIR}")
  target_link_libraries(${NODE_NAME} PRIVATE "${NUKE_DDIMAGE_LIB}")
  if(WIN32)
    target_compile_definitions(${NODE_NAME} PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
  endif()

  # Nuke plugin binaries are loaded from plugin paths added to NUKE_PATH.
  set_target_properties(${NODE_NAME} PROPERTIES
    PREFIX ""
    OUTPUT_NAME "${NODE_NAME}"
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
  )

  add_custom_command(TARGET ${NODE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PACKAGE_PLUGIN_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${NODE_NAME}>" "${PACKAGE_PLUGIN_DIR}/"
    COMMENT "Staging ${NODE_NAME} plugin in ${PACKAGE_PLUGIN_DIR}"
  )

  if(MSVC)
    target_compile_options(${NODE_NAME} PRIVATE /W4 /EHsc)
  else()
    set_target_properties(${NODE_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_compile_options(${NODE_NAME} PRIVATE -Wall -Wextra -Wpedantic)
  endif()

  list(APPEND PLUGIN_TARGETS ${NODE_NAME})
endforeach()

add_custom_target(package_plugin
  DEPENDS ${PLUGIN_TARGETS}
)

message(STATUS "Nuke version folder: ${NUKE_VERSION}")
message(STATUS "Nuke dependency include dir: ${NUKE_INCLUDE_DIR}")
message(STATUS "Nuke dependency lib dir: ${NUKE_LIB_DIR}")
message(STATUS "Nuke DDImage library: ${NUKE_DDIMAGE_LIB}")
message(STATUS "Plugin test output: ${PLUGIN_OUTPUT_DIR}")
message(STATUS "Plugin package output: ${PACKAGE_PLUGIN_DIR}")
