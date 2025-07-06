#include <iostream>
#include <csignal>
#include "audio/microphone.hpp"
#include "audio/speaker.hpp"

using namespace audio;

bool running = true;

void signal_handler(int signal) {
    if (signal == SIGINT) {
        running = false;
    }
}

int main() {
    std::cout << "Initializing PortAudio..." << std::endl;
    PaError err = Pa_Initialize();
    if(err != paNoError) {
        std::cerr << "PortAudio init failed: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    std::signal(SIGINT, signal_handler);
    
    const int sampleRate = 44100;
    const int channels = 1;
    const int framesPerBuffer = 512;

    Microphone mic;
    Speaker speaker;

    if (!mic.open(sampleRate, channels, framesPerBuffer)) {
        std::cerr << "Failed to open microphone." << std::endl;
        return 1;
    }

    if (!speaker.open(sampleRate, channels, framesPerBuffer)) {
        std::cerr << "Failed to open speaker." << std::endl;
        mic.close();
        return 1;
    }

    std::cout << "Real-time audio loop started. Press Ctrl+C to stop." << std::endl;
    
    try {
        while (running) {
            auto buffer = mic.capture(framesPerBuffer);
            if (!buffer.empty()) {
                speaker.play(buffer);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    std::cout << "\nShutting down..." << std::endl;
    mic.close();
    speaker.close();
    return 0;
}

