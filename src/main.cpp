#include "rtos/scheduler.hpp"
#include "audio/microphone.hpp"
#include "audio/speaker.hpp"
#include "audio/audio_buffer.hpp"
#include <vector>
#include <thread>
#include <chrono>
#include <iostream>

void processFrame(std::vector<float>& frame) {
    // Placeholder for DSP algorithms. Effects like vocoding, filtering
    // or compression will be applied to each frame here.
}

int main() {
    constexpr size_t frameSize = 512;
    audio::Microphone mic;
    audio::Speaker spk;
    audio::AudioBuffer buffer;

    if (!mic.open() || !spk.open()) {
        std::cerr << "Failed to init audio devices\n";
        return 1;
    }

    rtos::Scheduler sched;

    sched.addTask([&]() {
        std::vector<float> frame(frameSize);
        if (mic.capture(frame)) {
            processFrame(frame);
            buffer.push(frame);
        }
    });

    sched.addTask([&]() {
        auto frame = buffer.pop();
        spk.play(frame);
    });

    sched.start();

    // Run for a short time as a demo
    std::this_thread::sleep_for(std::chrono::seconds(2));
    sched.stop();

    return 0;
}
