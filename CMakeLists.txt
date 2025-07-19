cmake_minimum_required(VERSION 3.10...3.28)  # Flexible version range
project(voice_encoder)

# Add PortAudio subproject (use actual relative path)
add_subdirectory(portaudio)

# List source files explicitly (better than GLOB)
set(SOURCES
    src/audio/microphone.cpp
    src/audio/speaker.cpp
    src/main.cpp
    dsp/dsp.cpp)# NEW add!

# Gather your sources (better to list files explicitly)
#This second line covers the variable SOURCES with the same name ¡ý¡ý¡ý

#file(GLOB_RECURSE SOURCES "src/*.cpp")


# Create executable
add_executable(voice_encoder ${SOURCES})

# Link PortAudio
target_link_libraries(voice_encoder PRIVATE portaudio_static)

target_include_directories(voice_encoder PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}  
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio  # Explicitly add audio directory
    ${CMAKE_CURRENT_SOURCE_DIR}/dsp # NEW!
)