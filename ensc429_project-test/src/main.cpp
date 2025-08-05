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
//#include "dsp/FIR_coeff.hpp"
#include <vector> 
#include <iomanip>
#include <string>
#include <memory>
#include <algorithm>
//#include <utility>
//#include "dsp/BANDPASS_coeff.hpp"
#include "dsp/FIRDesign.hpp"

using namespace audio;
// ---------- Global audio parameters ----------
const int sampleRate = 44100;
const int channels = 1;
const int framesPerBuffer = 1024;
// ----------------------------------

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

void banner() {
    std::cout << "\n=========================================================\n"
        << "                    ENSC 429  Real-Time Vocoder\n"
        << "=========================================================\n";
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

    banner();
    std::signal(SIGINT, signal_handler);

    // ---------- PortAudio ----------
    if (Pa_Initialize() != paNoError) {
        std::cerr << "PortAudio init failed\n"; return 1;
    }//
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
   // ---------- DSP ----------
    // Number of bands you want: 16 or 32, or more, depending on freq range
    int numBands = 16;
    // Use equal ratio or equal width distribution, and the following example uses logarithmic equal ratio.
    double f_low = 100, f_high = 10000.0;
    std::vector<double> edges(numBands + 1);
    for (int i = 0; i <= numBands; ++i) {
        edges[i] = f_low * std::pow(f_high / f_low, double(i) / numBands);
    }

    // Prepare for each band Filter / Env / Mod
    std::vector<dsp::FIRFilter>        filters;
    std::vector<dsp::EnvelopeDetector> envs;
    std::vector<dsp::EnvelopeModulator> mods;
    /*-------------------------------------------------------------------------------------------
    * WGN is not working here:
    * If you want something that actually sounds like your own voice (even in a robotic way), 
    * you’ll need to switch to a sinusoidal vocoder or a phase-vocoder approach—one that extracts 
    * and re-synthesizes the voice’s fundamental frequency and harmonics, not just its envelope.
    ----------------------------------------------------------------------------------------------*/
    //std::vector<dsp::NoiseModulator>   mods;

    // Even number, convolution length = order+1
    int order = 96;

    for (int b = 0; b < numBands; ++b) {
        // (1) BP coeff 
        auto coeffs = designBandpassFIR(edges[b], edges[b + 1], order, sampleRate);
        filters.emplace_back(coeffs);
        // (2) Envelope detection
        envs.emplace_back(0.010f, 0.02f);
        // (3) Sinusoidal carrier: f0 in f_low, f_high logarithmic ratio division.
        double cf = std::sqrt(edges[b] * edges[b + 1]);
        mods.emplace_back(cf, 0.31f);
        
        // (3) white‐noise carrier (depth = 0.3)
        //mods.emplace_back(0.3f);

        filters.back().initialize(sampleRate, channels, framesPerBuffer);
        envs.back().initialize(sampleRate, channels, framesPerBuffer);
        mods.back().initialize(sampleRate, channels, framesPerBuffer);
    }
    std::cout << "Single-band vocoder running...  Ctrl+C to stop.\n";

    // ============ loop ============
    while (running) {

        auto raw = mic.capture(framesPerBuffer);
        std::vector<float> mix(raw.size(), 0.0f);

        for (int b = 0; b < numBands; ++b) {
            auto band = filters[b].process(raw, sampleRate);
            band = envs[b].process(band, sampleRate);
            band = mods[b].process(band, sampleRate);//sine
            //band = mods[b].process(band, sampleRate);//white noise
            for (size_t i = 0; i < mix.size(); ++i)
                mix[i] += band[i];
        }
        for (auto& s : mix)
            s = std::clamp(s * 4.0f, -1.0f, 1.0f);

        speaker.play(mix);
    }
    
    std::cout << "\nShutting down...\n" << std::endl;
    mic.close();
    speaker.close();
    printFooter();

    return 0;
}