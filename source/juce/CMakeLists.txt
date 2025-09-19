# Vulkan SDK 경로 가져오기
set(VULKAN_SDK $ENV{VULKAN_SDK})

if(NOT VULKAN_SDK)
    message(FATAL_ERROR "VULKAN_SDK environment variable not set. Please install Vulkan SDK.")
endif()

file(GLOB_RECURSE srcs
    "{CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(juce STATIC ${srcs})
add_library(juce::juce ALIAS juce)

set_target_properties(juce PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${juce_bin_dir}"
    RUNTIME_OUTPUT_DIRECTORY "${juce_bin_dir}"
)

target_include_directories(juce PUBLIC
    ${juce_inc_dir}
    ${VULKAN_SDK}/Include
)

target_link_libraries(juce PUBLIC "${VULKAN_SDK}/Lib/vulkan-1.lib")

if(JUCE_USE_EXPERIMENTAL)
    target_compile_definitions(juce PUBLIC USE_EXPERIMENTAL)
endif()
