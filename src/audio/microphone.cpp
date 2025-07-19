/*
// ENSC 429 Project - Real-Time Vocoder
//
// This project implements a real-time vocoder using PortAudio for audio input/output
// and a custom DSP processing chain for audio effects.
//
// File     : microphone.cpp
// Date     : 2025-07-18
// Authors  :
//      Keon Kye       301475751
//      Michael Yap    301452311
*/

#include "microphone.hpp"

namespace audio {

Microphone::Microphone() {
    Pa_Initialize();
}

Microphone::~Microphone() {
    close();
    Pa_Terminate();
}

bool Microphone::open(int sampleRate, int channels, unsigned long framesPerBuffer) {
    PaStreamParameters inputParams;
    inputParams.device = Pa_GetDefaultInputDevice();
    if (inputParams.device == paNoDevice) return false;
    inputParams.channelCount = channels;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    return Pa_OpenStream(&stream_, &inputParams, nullptr, sampleRate, framesPerBuffer, paNoFlag, nullptr, nullptr) == paNoError
        && Pa_StartStream(stream_) == paNoError;
}

std::vector<float> Microphone::capture(unsigned long frames) {
    std::vector<float> buffer(frames);
    if (stream_) {
        Pa_ReadStream(stream_, buffer.data(), frames);
    }
    return buffer;
}

void Microphone::close() {
    if (stream_) {
        Pa_StopStream(stream_);
        Pa_CloseStream(stream_);
        stream_ = nullptr;
    }
}

} // namespace audio