/*
// ENSC 429 Project - Real-Time Vocoder
//
// This project implements a real-time vocoder using PortAudio for audio input/output
// and a custom DSP processing chain for audio effects.
//
// File     : dsp.cpp
// Date     : 2025-07-18
// Authors  :
//      Keon Kye       301475751
*/

#include "dsp.hpp"
#include "FIR_coeff.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper function for clamping values (C++11 compatible)
template<typename T>
T clamp(T value, T min, T max) {
    return std::max(min, std::min(value, max));
}

namespace dsp {

GainEffect::GainEffect(float gain) : gain(gain) {}

void GainEffect::setGain(float newGain) {
    gain = newGain;
}

float GainEffect::getGain() const {
    return gain;
}

std::vector<float> GainEffect::process(const std::vector<float>& input, int sampleRate) {
    std::vector<float> output(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] = input[i] * gain;
    }
    return output;
}

void GainEffect::initialize(int sampleRate, int channels, int framesPerBuffer) {
}


// FIRFilter Implementation

FIRFilter::FIRFilter() : delayIndex(0) {
    // Use the designed coefficients by default
    useDesignedCoefficients();
}

FIRFilter::FIRFilter(const std::vector<float>& coeffs) : delayIndex(0) {
    setCoefficients(coeffs);
}

FIRFilter::FIRFilter(const std::vector<double>& coeffs) : delayIndex(0) {
    setCoefficients(coeffs);
}

void FIRFilter::setCoefficients(const std::vector<float>& coeffs) {
    if (coeffs.empty()) {
        // If empty, use designed coefficients
        useDesignedCoefficients();
        return;
    }
    
    coefficients = coeffs;
    // Resize delay line to match filter order
    delayLine.resize(coefficients.size(), 0.0f);
    delayIndex = 0;
}

void FIRFilter::setCoefficients(const std::vector<double>& coeffs) {
    if (coeffs.empty()) {
        // If empty, use designed coefficients
        useDesignedCoefficients();
        return;
    }
    
    // Convert double to float
    coefficients.clear();
    coefficients.reserve(coeffs.size());
    for (double coeff : coeffs) {
        coefficients.push_back(static_cast<float>(coeff));
    }
    
    // Resize delay line to match filter order
    delayLine.resize(coefficients.size(), 0.0f);
    delayIndex = 0;
}

void FIRFilter::useDesignedCoefficients() {
    setCoefficients(DESIGNED_FIR_FILTER_COEFFICIENTS);
}

void FIRFilter::setCoefficientsFromString(const std::string& coeffString) {
    std::vector<float> coeffs;
    std::istringstream iss(coeffString);
    std::string token;
    
    while (iss >> token) {
        try {
            float coeff = std::stof(token);
            coeffs.push_back(coeff);
        } catch (const std::exception&) {
            continue;
        }
    }
    
    if (!coeffs.empty()) {
        setCoefficients(coeffs);
    }
}

const std::vector<float>& FIRFilter::getCoefficients() const {
    return coefficients;
}

size_t FIRFilter::getFilterOrder() const {
    return coefficients.size();
}

void FIRFilter::clearDelayLine() {
    std::fill(delayLine.begin(), delayLine.end(), 0.0f);
    delayIndex = 0;
}

std::vector<float> FIRFilter::process(const std::vector<float>& input, int sampleRate) {
    std::vector<float> output(input.size());
    
    if (coefficients.empty() || delayLine.empty()) {
        return input;
    }
    
    for (size_t i = 0; i < input.size(); ++i) {
        // Store input sample in delay line
        delayLine[delayIndex] = input[i];
        
        // Compute FIR filter output
        float result = 0.0f;
        for (size_t j = 0; j < coefficients.size(); ++j) {
            // Calculate delay line index with wraparound
            int tapIndex = (delayIndex - j + delayLine.size()) % delayLine.size();
            result += coefficients[j] * delayLine[tapIndex];
        }
        
        output[i] = result;
        
        // Update delay line index
        delayIndex = (delayIndex + 1) % delayLine.size();
    }
    
    return output;
}

void FIRFilter::initialize(int sampleRate, int channels, int framesPerBuffer) {
    // Clear delay line on initialization
    clearDelayLine();
}


// DSPProcessor Implementation

DSPProcessor::DSPProcessor() : sampleRate(44100), channels(1), framesPerBuffer(512), bypass(false) {}

void DSPProcessor::initialize(int sampleRate, int channels, int framesPerBuffer) {
    this->sampleRate = sampleRate;
    this->channels = channels;
    this->framesPerBuffer = framesPerBuffer;
    
    // Initialize all effects with new parameters
    for (auto& effect : effects) {
        effect->initialize(sampleRate, channels, framesPerBuffer);
    }
}

void DSPProcessor::addEffect(std::unique_ptr<DSPEffect> effect) {
    if (effect) {
        effect->initialize(sampleRate, channels, framesPerBuffer);
        effects.push_back(std::move(effect));
    }
}

void DSPProcessor::clearEffects() {
    effects.clear();
}

std::vector<float> DSPProcessor::process(const std::vector<float>& input) {
    if (bypass || effects.empty()) {
        return input; // Return unprocessed input
    }
    
    std::vector<float> buffer = input;
    
    // Process through each effect in the chain
    for (auto& effect : effects) {
        buffer = effect->process(buffer, sampleRate);
    }
    
    return buffer;
}

void DSPProcessor::setBypass(bool enabled) {
    bypass = enabled;
}

bool DSPProcessor::isBypassed() const {
    return bypass;
}

size_t DSPProcessor::getEffectCount() const {
    return effects.size();
}

} // namespace dsp
