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