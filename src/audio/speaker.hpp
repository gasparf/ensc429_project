#pragma once

#include <vector>
#include <portaudio.h>

namespace audio {

class Speaker {
public:
    Speaker();
    ~Speaker();

    bool open(int sampleRate = 44100, int channels = 1, unsigned long framesPerBuffer = 512);
    void play(const std::vector<float>& samples);
    void close();

private:
    PaStream* stream_ = nullptr;
};

} // namespace audio