#pragma once
#include <vector>

enum class DSPMode {
    Bypass,
    HeavenEcho   // Multiple ethereal echoes
};

class HeavenDSP {
public:
    HeavenDSP(int sampleRate, int maxDelayMs = 1500);

    void setMode(DSPMode m) { mode = m; }
    void setWetDry(float w) { wet = w; }          // 0~1
    void setFeedback(float fb) { feedback = fb; }    // 0~1
    void setTaps(const std::vector<int>& delaysMs);           // 设定多重延迟

    // In-situ processing of a frame of floating-point PCM data
    void process(float* data, int nSamples);

private:
    DSPMode mode = DSPMode::Bypass;
    float wet = 0.5f;   // Wet-sound ratio
    float feedback = 0.4f;   // Overall attenuation
    int  sampleRate;
    int  bufSize;
    std::vector<float> buf;      // Annular buffer
    int  writeIdx = 0;
    std::vector<int> tapOffset;  // Sample offset of each echo TAP
};

