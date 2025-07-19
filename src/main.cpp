#include <iostream>
#include <csignal>
#include "audio/microphone.hpp"
#include "audio/speaker.hpp"
/*New add DSP header*/
#include "dsp/dsp.h"

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
    /* Add-Establish DSP objects and adjust parameters. */
    HeavenDSP dsp(sampleRate);              // Maximum 1.5 s buffer
    dsp.setMode(DSPMode::HeavenEcho);
    dsp.setWetDry(0.3f);                    // 60% wet sound
    dsp.setFeedback(0.2f);                 // Echo tailing
    dsp.setTaps({ 300, 600});           // 2 echo taps (ms unit)
    /*End added*/
    std::cout << "Real-time audio loop started. Press Ctrl+C to stop." << std::endl;
    
    try {
        while (running) {
            auto buffer = mic.capture(framesPerBuffer);
            if (!buffer.empty()) {
                /* Add-DSP processing before playing */
                dsp.process(buffer.data(), static_cast<int>(buffer.size()));
                /*End added*/
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

