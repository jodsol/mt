cmake_minimum_required(VERSION 3.22)

project(jure-engine LANGUAGES C CXX VERSION 0.0.0)

# engine
add_subdirectory(source/juce "${CMAKE_CURRENT_BINARY_DIR}/juce")

# excutable
add_subdirectory(source/sample "${CMAKE_CURRENT_BINARY_DIR}/sample")
