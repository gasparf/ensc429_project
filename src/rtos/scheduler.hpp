/*
// ENSC 429 Project - Real-Time Vocoder
//
// This project implements a real-time vocoder using PortAudio for audio input/output
// and a custom DSP processing chain for audio effects.
//
// File     : scheduler.hpp
// Date     : 2025-07-18
// Authors  :
//      Keon Kye       301475751
*/

#pragma once
#include "task.hpp"
#include <vector>
#include <memory>

namespace rtos {

class Scheduler {
public:
    Scheduler() = default;
    ~Scheduler();

    template<class F>
    void addTask(F&& job) {
        tasks_.emplace_back(std::make_unique<Task>(std::forward<F>(job)));
    }

    void start();
    void stop();

private:
    std::vector<std::unique_ptr<Task>> tasks_;
};

} // namespace rtos