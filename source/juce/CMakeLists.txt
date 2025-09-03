# Vulkan SDK 경로 가져오기
set(VULKAN_SDK $ENV{VULKAN_SDK})

if (NOT VULKAN_SDK)
    message(FATAL_ERROR "VULKAN_SDK environment variable not set. Please install Vulkan SDK.")
endif()


file(GLOB_RECURSE srcs
    "{CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(juce-engine STATIC ${srcs})
add_library(juce::juce ALIAS juce-engine)

set_target_properties(juce-engine PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
)

# include <juce/xxx>
get_filename_component(inc_dir "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

message(STATUS "[juce] ${inc_dir} ${CMAKE_BINARY_DIR}")

target_include_directories(juce-engine PUBLIC ${inc_dir} ${VULKAN_SDK}/Include)
target_link_libraries(juce-engine PUBLIC "${VULKAN_SDK}/Lib/vulkan-1.lib")



