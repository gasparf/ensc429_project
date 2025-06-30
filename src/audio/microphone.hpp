#pragma once

#include <vector>
#include <portaudio.h>

namespace audio {

class Microphone {
public:
    Microphone();
    ~Microphone();

    bool open(int sampleRate = 44100, int channels = 1, unsigned long framesPerBuffer = 512);
    std::vector<float> capture(unsigned long frames);
    void close();

private:
    PaStream* stream_ = nullptr;
};

} // namespace audio