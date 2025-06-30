#pragma once
#include <vector>

namespace audio {

class Microphone {
public:
    bool open();
    bool capture(std::vector<float>& buffer);
};

} // namespace audio