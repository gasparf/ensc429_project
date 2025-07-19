/*
// ENSC 429 Project - Real-Time Vocoder
//
// This project implements a real-time vocoder using PortAudio for audio input/output
// and a custom DSP processing chain for audio effects.
//
// File     : scheduler.cpp
// Date     : 2025-07-18
// Authors  :
//      Keon Kye       301475751
*/

#include "scheduler.hpp"

namespace rtos {

Scheduler::~Scheduler() {
    stop();
}

void Scheduler::start() {
    for (auto& t : tasks_) {
        t->start();
    }
}

void Scheduler::stop() {
    for (auto& t : tasks_) {
        t->stop();
    }
}

} // namespace rtos