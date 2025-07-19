/*
// ENSC 429 Project - Real-Time Vocoder
//
// This project implements a real-time vocoder using PortAudio for audio input/output
// and a custom DSP processing chain for audio effects.
//
// File     : main.cpp
// Date     : 2025-07-18
// Authors  :
//      Keon Kye       301475751
//      Michael Yap    301452311
*/

#include <iostream>
#include <csignal>
#include "audio/microphone.hpp"
#include "audio/speaker.hpp"

/*New add DSP header*/
#include "dsp/dsp.h"

#include <portaudio.h>
#include "dsp/dsp.hpp"
#include "dsp/FIR_coeff.hpp"
#include <iomanip>
#include <string>


using namespace audio;

bool running = true;

void signal_handler(int signal) {
    if (signal == SIGINT) {
        running = false;
    }
}

void printEnter() {
    std::cout << "\n";
}

void printHeader() {\
    printEnter();
    const int width = 57; 
    const std::string border(width, '=');
    const std::string title1 = "ENSC 429 Project";
    const std::string title2 = "Real-Time Vocoder";

    std::cout << border << std::endl;
    std::cout << std::setw((width + title1.size()) / 2) << title1 << std::endl;
    std::cout << std::setw((width + title2.size()) / 2) << title2 << std::endl;
    std::cout << border << std::endl;
}

void printFooter() {
    printEnter();
    const int width = 57; 
    const std::string border(width, '=');
    const std::string title3 = "Thank you:) Goodbye!";
    std::cout << std::setw((width + title3.size()) / 2) << title3 << std::endl;
    std::cout << border << std::endl;
}

int main() {

    printHeader();
    std::cout << "\nInitializing PortAudio..." << std::endl;

    PaError err = Pa_Initialize();
    if(err != paNoError) {
        std::cerr << "\nPortAudio init failed: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    std::cout << "\nPortAudio Initialized Successfully!\n" << std::endl;

    std::signal(SIGINT, signal_handler);
    
    const int sampleRate = 44100;
    const int channels = 1;
    const int framesPerBuffer = 512;

    // init DSP processor
    dsp::DSPProcessor dspProcessor;
    dspProcessor.initialize(sampleRate, channels, framesPerBuffer);
    
    // gain effect (volume control)
    // testing...
    dspProcessor.addEffect(std::make_unique<dsp::GainEffect>(0.8f)); // volume 80%


    // ===============================================================================================================

    // VOCODER LOGIC
    // BPF -> Envelop detector (per pand) -> Envelop Modulator (* carrier, ie. sine wave or WGN) -> Sum 
    
    // FIR filter using DESIGNED_FIR_FILTER_COEFFICIENTS from FIR_coeff.hpp
    // The filter will automatically use the coefficients you put in FIR_coeff.cpp
    auto firFilter = std::make_unique<dsp::FIRFilter>(dsp::DESIGNED_FIR_FILTER_COEFFICIENTS);

    // TODO = Envelop detector

    // TODO = Envelop Modulator

    // TODO = Sum

    // ===============================================================================================================


    std::cout << "FIR Filter initialized with " << firFilter->getFilterOrder() << " coefficients." << std::endl;

    dspProcessor.addEffect(std::move(firFilter));
    
    std::cout << "DSP Processor initialized with " << dspProcessor.getEffectCount() << " effects." << std::endl;

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
    //Merge the loop start prompts on both sides.
    std::cout << "Real-time audio loop started. Press Ctrl+C to stop." << std::endl;
    std::cout << "\n\nRT Vocoder is looping. Press Ctrl+C to terminate...\n" << std::endl;
    
    try {
        while (running) {
            auto buffer = mic.capture(framesPerBuffer);
            if (!buffer.empty()) {
                
                speaker.play(buffer);
                auto processedBuffer = dspProcessor.process(buffer);
                dsp.process(buffer.data(), static_cast<int>(buffer.size()));
                speaker.play(processedBuffer);

            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    std::cout << "\nShutting down...\n" << std::endl;
    mic.close();
    speaker.close();
    printFooter();

    return 0;
}

