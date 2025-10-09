cmake_minimum_required(VERSION 3.22)

project(juce-engine LANGUAGES C CXX VERSION 0.0.0)

option(JUCE_USE_EXPERIMENTAL "enable the experimental features" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-nullability-completeness)
endif()

set(juce_bin_dir "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(juce_inc_dir "${CMAKE_CURRENT_SOURCE_DIR}/source")

# engine
add_subdirectory(source/juce "${CMAKE_CURRENT_BINARY_DIR}/juce")

# excutable
add_subdirectory(source/sample "${CMAKE_CURRENT_BINARY_DIR}/sample")

find_program(GLSLANG_VALIDATOR glslangValidator)

if(NOT GLSLANG_VALIDATOR)
    message(FATAL_ERROR "glslangValidator not found! Please install Vulkan SDK.")
else()
    message(STATUS "Found glslangValidator: ${GLSLANG_VALIDATOR}")
endif()
