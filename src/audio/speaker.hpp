#pragma once
#include <vector>

namespace audio {

class Speaker {
public:
    bool open();
    bool play(const std::vector<float>& buffer);
};

} // namespace audio