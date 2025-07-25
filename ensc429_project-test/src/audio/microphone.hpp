/*
// ENSC 429 Project - Real-Time Vocoder
//
// This project implements a real-time vocoder using PortAudio for audio input/output
// and a custom DSP processing chain for audio effects.
//
// File     : microphone.hpp
// Date     : 2025-07-18
// Authors  :
//      Keon Kye       301475751
//      Michael Yap    301452311
*/

#pragma once

#include <vector>
#include <portaudio.h>

namespace audio {

class Microphone {
public:
    Microphone();
    ~Microphone();

    bool open(int sampleRate = 44100, int channels = 1, unsigned long framesPerBuffer = 512);
    std::vector<float> capture(unsigned long frames);
    void close();

private:
    PaStream* stream_ = nullptr;
};

} // namespace audio