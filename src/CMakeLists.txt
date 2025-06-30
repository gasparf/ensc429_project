add_library(audio
    audio/microphone.cpp
    audio/microphone.h
    audio/speaker.h
    audio/speaker.cpp
)

find_package(portaudio REQUIRED)

# Include directories
target_include_directories(audio PUBLIC ${PORTAUDIO_INCLUDE_DIRS})
target_include_directories(audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/audio)

# Link libraries
target_link_libraries(audio PUBLIC ${PORTAUDIO_LIBRARIES})
