/*
// ENSC 429 Project - Real-Time Vocoder
//
// This project implements a real-time vocoder using PortAudio for audio input/output
// and a custom DSP processing chain for audio effects.
//
// File     : task.hpp
// Date     : 2025-07-18
// Authors  :
//      Keon Kye       301475751
*/

#pragma once
#include <functional>
#include <thread>
#include <atomic>

namespace rtos {

class Task {
public:
    using Job = std::function<void()>;

    explicit Task(Job job);
    ~Task();

    void start();
    void stop();

private:
    Job job_;
    std::thread worker_;
    std::atomic<bool> running_{false};
};

} // namespace rtos