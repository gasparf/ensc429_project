/*
// ENSC 429 Project - Real-Time Vocoder
//
// This project implements a real-time vocoder using PortAudio for audio input/output
// and a custom DSP processing chain for audio effects.
//
// File     : audio_buffer.cpp
// Date     : 2025-07-18
// Authors  :
//      Keon Kye       301475751
//      Michael Yap    301452311
*/

#include "audio_buffer.hpp"

namespace audio {

void AudioBuffer::push(const std::vector<float>& data) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(data);
    }
    cv_.notify_one();
}

std::vector<float> AudioBuffer::pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]{ return !queue_.empty(); });
    auto data = std::move(queue_.front());
    queue_.pop();
    return data;
}

} // namespace audio