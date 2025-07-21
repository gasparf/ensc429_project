/*
// ENSC 429 Project - Real-Time Vocoder
//
// This project implements a real-time vocoder using PortAudio for audio input/output
// and a custom DSP processing chain for audio effects.
//
// File     : audio_buffer.hpp
// Date     : 2025-07-18
// Authors  :
//      Keon Kye       301475751
//      Michael Yap    301452311
*/

#pragma once
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace audio {

class AudioBuffer {
public:
    void push(const std::vector<float>& data);
    std::vector<float> pop();

private:
    std::queue<std::vector<float>> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

} // namespace audio