/*
// ENSC 429 Project - Real-Time Vocoder
//
// This project implements a real-time vocoder using PortAudio for audio input/output
// and a custom DSP processing chain for audio effects.
//
// File     : task.cpp
// Date     : 2025-07-18
// Authors  :
//      Keon Kye       301475751
*/

#include "task.hpp"
#include <chrono>

using namespace std::chrono_literals;

namespace rtos {

Task::Task(Job job) : job_(std::move(job)) {}

Task::~Task() {
    stop();
}

void Task::start() {
    running_ = true;
    worker_ = std::thread([this]() {
        while (running_) {
            job_();
            std::this_thread::sleep_for(1ms); // small delay to yield
        }
    });
}

void Task::stop() {
    running_ = false;
    if (worker_.joinable()) {
        worker_.join();
    }
}

} // namespace rtos