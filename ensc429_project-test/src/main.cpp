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
#include <portaudio.h>
#include "dsp/dsp.hpp"
#include "dsp/FIR_coeff.hpp"
#include <iomanip>
#include <string>
#include <memory>
#include <algorithm>

using namespace audio;

// Helper function for clamping values (C++11 compatible)
template<typename T>
T clamp(T value, T min, T max) {
    return std::max(min, std::min(value, max));
}

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

int main(int argc, char* argv[]) {
    // YY allow gain override via command line
    float gainValue = 0.8f; // default gain
    
    if (argc > 1) {
        try {
            gainValue = std::stof(argv[1]);
            // Clamp gain between 0.0 and 2.0 for safety
            gainValue = clamp(gainValue, 0.0f, 2.0f);
        } catch (const std::exception& e) {
            std::cerr << "Invalid gain value. Using default 0.8" << std::endl;
            gainValue = 0.8f;
        }
    }

    printHeader();
    std::cout << "\nGain Control: " << std::fixed << std::setprecision(2) << gainValue << std::endl;
    std::cout << "\nInitializing PortAudio..." << std::endl;

    PaError err = Pa_Initialize();
    if(err != paNoError) {
        std::cerr << "\nPortAudio init failed: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    std::cout << "\nPortAudio Initialized Successfully!\n" << std::endl;

    std::signal(SIGINT, signal_handler);
    
    // common audio settings
    const int sampleRate = 44100;
    const int channels = 1; // mono input/output
    const int framesPerBuffer = 512;// ~11.6 ms blocks

    // init DSP processor
    dsp::DSPProcessor dspProcessor;
    dspProcessor.initialize(sampleRate, channels, framesPerBuffer);
    
    // gain effect (volume control) - using command line parameter
    dspProcessor.addEffect(std::make_unique<dsp::GainEffect>(gainValue));


    // ===============================================================================================================

    // VOCODER LOGIC
    // BPF -> Envelop detector (per pand) -> Envelop Modulator (* carrier, ie. sine wave or WGN) -> Sum 
    
    // FIR filter using DESIGNED_FIR_FILTER_COEFFICIENTS from FIR_coeff.hpp
    // The filter will automatically use the coefficients you put in FIR_coeff.cpp
    auto firFilter = std::make_unique<dsp::FIRFilter>(dsp::DESIGNED_FIR_FILTER_COEFFICIENTS);
    // ===============================================================================================================
    
    /* DONE.TODO = Envelop detector
       DONE. TODO = Envelop Modulator*/

    /* ———————————————————————————————————————————————————————
    / YY
    / ORIGINAL: You had one DSPProcessor for all effects.
    / dsp::DSPProcessor dspProcessor;
    / dspProcessor.initialize(...);
    / dspProcessor.addEffect(GainEffect);
    / dspProcessor.addEffect(FIRFilter);
    / dspProcessor.addEffect(EnvelopeDetector);
    / dspProcessor.addEffect(EnvelopeModulator);
    / dspProcessor.addEffect(SumEffect);
    / ———————————————————————————————————————————————————————*/

    // YY UPDATED: Split into two chains for clarity:
  
    // YY 1) dspModulator: just envelope detection + modulation
    // YY Create a processing chain that only does envelope detection and modulation.
    dsp::DSPProcessor dspModulator;
    dspModulator.initialize(sampleRate, channels, framesPerBuffer);
    dspModulator.addEffect(
        std::make_unique<dsp::EnvelopeDetector>(0.005f, 0.1f));
    dspModulator.addEffect(
        std::make_unique<dsp::EnvelopeModulator>(440.0f, 0.8f));

    // YY 2) dspFilter: gain + FIR filtering
    // YY Create a processing chain that only does gain and fir filtering.
    dsp::DSPProcessor dspFilter;
    dspFilter.initialize(sampleRate, channels, framesPerBuffer);
    // Optional: Move the gain here - using command line parameter
    dspFilter.addEffect(
        std::make_unique<dsp::GainEffect>(gainValue));
    dspFilter.addEffect(
        std::make_unique<dsp::FIRFilter>(dsp::DESIGNED_FIR_FILTER_COEFFICIENTS));

    std::cout << "Vocoder Modulator initialized with 2 effects." << std::endl;
    std::cout << "Filter Processor initialized with "
        << dspFilter.getEffectCount() << " effects." << std::endl;//END YY
   
    //  TODO = Sum



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

    std::cout << "\n\nRT Vocoder is looping. Press Ctrl+C to terminate...\n" << std::endl;
    
    // YY UPDATED: main processing loop
    try {
        while (running) {
            // 1) capture raw audio
            auto raw = mic.capture(framesPerBuffer);
            if (raw.empty()) continue;

            // 2) envelope → modulation
            auto modulated = dspModulator.process(raw);

            // 3) mix original + modulated
            std::vector<float> mixed(raw.size());
            for (size_t i = 0; i < raw.size(); ++i) {
                mixed[i] = raw[i] + modulated[i];
            }

            // 4) post‑processing: gain + FIR
            auto finalOut = dspFilter.process(mixed);

            // 5) play to speaker
            speaker.play(finalOut);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    } // END YY

    std::cout << "\nShutting down...\n" << std::endl;
    mic.close();
    speaker.close();
    printFooter();

    return 0;
}

