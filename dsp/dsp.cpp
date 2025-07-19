#include "dsp.h"
#include <algorithm>
#include <cmath>

HeavenDSP::HeavenDSP(int sr, int maxDelayMs)
    : sampleRate(sr)
{
    bufSize = sr * maxDelayMs / 1000 + 1;   //Ring buffer size
    buf.assign(bufSize, 0.0f);

    //Default triple echo: 200 ms/400 ms/800 ms.
    setTaps({ 200, 400, 800 });
}

void HeavenDSP::setTaps(const std::vector<int>& delaysMs)
{
    tapOffset.clear();
    for (int d : delaysMs) {
        tapOffset.push_back(d * sampleRate / 1000);
    }
}

void HeavenDSP::process(float* data, int nSamples)
{
    if (mode == DSPMode::Bypass) return;

    for (int i = 0; i < nSamples; ++i) {
        float dryIn = data[i];
        float accumulated = 0.0f;

        // Read all taps
        for (size_t t = 0; t < tapOffset.size(); ++t) {
            int idx = (writeIdx + bufSize - tapOffset[t]) % bufSize;
            accumulated += buf[idx];
        }

        // Normalization and mixing
        float wetOut = accumulated / static_cast<float>(tapOffset.size());
        float out = dryIn * (1.0f - wet) + wetOut * wet;
        data[i] = out;

        // Write a new sample+feedback
        buf[writeIdx] = dryIn + wetOut * feedback;

        // Forward write pointer
        if (++writeIdx >= bufSize) writeIdx = 0;
    }
}
