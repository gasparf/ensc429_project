add_library(rtos
    rtos/task.cpp
    rtos/scheduler.cpp
    audio/microphone.cpp
    audio/speaker.cpp
    audio/audio_buffer.cpp
)

target_include_directories(rtos PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(voice_encoder main.cpp)
target_link_libraries(voice_encoder PRIVATE rtos)