
file(GLOB_RECURSE srcs
    "{CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(jure-engine STATIC ${srcs})
add_library(jure::jure ALIAS jure-engine)

set_target_properties(jure-engine PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
)

# include <juce/xxx>
get_filename_component(inc_dir "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

message(STATUS "[juce] ${inc_dir} ${CMAKE_BINARY_DIR}")

target_include_directories(jure-engine PUBLIC ${inc_dir})
