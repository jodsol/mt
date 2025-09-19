# examples
file(GLOB_RECURSE srcs *.h *.cpp)

add_executable(game "${srcs}")

target_link_libraries(game PRIVATE juce::juce)

set_target_properties(game PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${juce_bin_dir}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${juce_bin_dir}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${juce_bin_dir}
)