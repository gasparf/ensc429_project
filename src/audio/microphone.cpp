#include "microphone.hpp"
#include <algorithm>

namespace audio {

bool Microphone::open() {
    // Stub: normally we would initialize the audio backend here
    return true;
}

bool Microphone::capture(std::vector<float>& buffer) {
    // Placeholder capturing: fill buffer with zeros
    std::fill(buffer.begin(), buffer.end(), 0.0f);
    return true;
}

} // namespace audio