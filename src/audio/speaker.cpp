#include "speaker.hpp"
#include <iostream>

namespace audio {

bool Speaker::open() {
    // Stub: normally we would set up audio playback here
    return true;
}

bool Speaker::play(const std::vector<float>& buffer) {
    // Placeholder playback: show number of samples
    std::cout << "Playing " << buffer.size() << " samples\n";
    return true;
}

} // namespace audio