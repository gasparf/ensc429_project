#include "audio_buffer.hpp"

namespace audio {

// Add a cancellation flag to allow clean shutdown

void AudioBuffer::push(const std::vector<float>& data) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(data);
    }
    cv_.notify_one();
}

// Signal all waiting threads to stop and wake them up
void AudioBuffer::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopped_ = true;
    }
    cv_.notify_all();
}

std::vector<float> AudioBuffer::pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]{ return stopped_ || !queue_.empty(); });
    if (stopped_ && queue_.empty()) {
        return {}; // Return empty vector if stopped and no data
    }
    auto data = std::move(queue_.front());
    queue_.pop();
    return data;
}

} // namespace audio