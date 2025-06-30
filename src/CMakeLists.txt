cmake_minimum_required(VERSION 3.10)

# Allow CMake to find additional modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Attempt to locate PortAudio
find_package(PortAudio REQUIRED)

# dummy target to demonstrate linkage
add_library(example STATIC example.c)

target_link_libraries(example PRIVATE PortAudio::PortAudio)