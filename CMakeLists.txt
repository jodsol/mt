cmake_minimum_required(VERSION 3.22)

project(juce-engine LANGUAGES C CXX VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(juce_bin_dir "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(juce_inc_dir "${CMAKE_CURRENT_SOURCE_DIR}/source")

# engine
add_subdirectory(source/juce "${CMAKE_CURRENT_BINARY_DIR}/juce")

# excutable
add_subdirectory(source/sample "${CMAKE_CURRENT_BINARY_DIR}/sample")
