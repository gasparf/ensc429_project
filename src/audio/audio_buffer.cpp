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