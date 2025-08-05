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
//#include "dsp/BANDPASS_coeff.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <random>


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
    //-----------------------------------------
    // YY Realization of EnvelopeDetector
    //-----------------------------------------
    EnvelopeDetector::EnvelopeDetector(float attackTimeSec, float releaseTimeSec)
        : attackTimeSec_(attackTimeSec)// store how fast to follow rising amplitude
        , releaseTimeSec_(releaseTimeSec)// store how fast to follow falling amplitude
        , attackCoeff_(0.0f) // will hold per-sample attack multiplier
        , releaseCoeff_(0.0f) // will hold per-sample release multiplier
    {
        // constructor body is empty: we only store parameters here
    }

    void EnvelopeDetector::initialize(int sampleRate, int channels, int /*framesPerBuffer*/) {
        // compute multiplier so that attackTimeSec_ seconds corresponds to ~63% change
        attackCoeff_ = std::exp(-1.0f / (attackTimeSec_ * sampleRate));
        // same for release time
        releaseCoeff_ = std::exp(-1.0f / (releaseTimeSec_ * sampleRate));
        // reset envelope state for each channel to zero
        envState_.assign(channels, 0.0f);
    }

    std::vector<float> EnvelopeDetector::process(const std::vector<float>& input, int /*sampleRate*/) {
        // prepare output buffer same size as input
        std::vector<float> output(input.size());
        // number of channels stored
        int chCount = static_cast<int>(envState_.size());

        // process every sample in the block
        for (size_t i = 0; i < input.size(); ++i) {
            // take absolute value to get amplitude
            float inAbs = std::fabs(input[i]);
            // pick which channel state to update (interleaved data)
            float& state = envState_[i % chCount];

            /*if (inAbs > state) {
                // rising edge: use faster attackCoeff_ to track up
                state = attackCoeff_ * (state - inAbs) + inAbs;
            }
            else {
                // output current envelope value
                state = releaseCoeff_ * (state - inAbs) + inAbs;
            }*/state += (inAbs - state) * (inAbs > state ?
                (1.0f - attackCoeff_) : (1.0f - releaseCoeff_));

            output[i] = state;
        }
        return output;
    }

    //---------------------------
    // YY EnvelopeModulator
    //---------------------------

    EnvelopeModulator::EnvelopeModulator(float carrierFreqHz, float depth)
        : carrierFreqHz_(carrierFreqHz)// store carrier sine frequency
        , depth_(depth) // store how strong the modulation is
        , carrierPhase_(0.0f) // start phase at zero
        , phaseIncrement_(0.0f) // will compute per-sample phase step
        , channels_(1)// default to 1 channel until initialize()
    {
        // constructor does not compute anything heavy
    }

    void EnvelopeModulator::initialize(int sampleRate, int channels, int /*framesPerBuffer*/) {
        channels_ = channels;// save channel count
        // compute how much to advance the sine phase each sample
        phaseIncrement_ = 2.0f * PI * carrierFreqHz_ / sampleRate;
        carrierPhase_ = 0.0f; // reset phase
    }

    std::vector<float> EnvelopeModulator::process(const std::vector<float>& input, int /*sampleRate*/) {
        // output buffer same size as input
        std::vector<float> output(input.size());
        
        // process every sample
        for (size_t i = 0; i < input.size(); ++i) {
            float env = input[i];// read envelope from detector
            float carrier = std::sin(carrierPhase_);// generate one sine sample
            
            // multiply envelope by sine, scaled by depth_
            output[i] = depth_ * env * carrier;

            // advance sine phase, wrap at 2π
            carrierPhase_ += phaseIncrement_;
            if (carrierPhase_ >= 2.0f * PI)
                carrierPhase_ -= 2.0f * PI;
        }
        return output;
    }

    // ---- wgn ----
    NoiseModulator::NoiseModulator(float depth)
        : depth_(depth), rng_(std::random_device{}()), dist_(0.0f, 1.0f) {
    }

    void NoiseModulator::initialize(int /*sr*/, int /*ch*/, int /*fpb*/) {
        // nothing to pre-compute
    }

    std::vector<float> NoiseModulator::process(const std::vector<float>& env, int /*sr*/) {
        std::vector<float> out(env.size());
        for (size_t i = 0; i < env.size(); ++i)
            out[i] = depth_ * env[i] * dist_(rng_);   // env × WGN
        return out;
    }


//---------------------------
// YY VocoderEffect
//---------------------------
/*--------------------------------------------------------------------------------------
/YY
/   1. Constructor only stores configuration; real setup happens in initialize().
/   2. initialize() loops over each band to:  
/       a) Make a bandpass filter with your FIR coefficients.
/       b)Create an envelope detector for that band.
/       c)Choose a sinewave carrier frequency.
/       d)Create an envelope modulator.
/       e)Call each object’s initialize() so they know the sample rate and channels.
/   3. process() for each audio block: 
/       a)Run the block through the band filter.
/       b)Extract its envelope.
/       c)Modulate that envelope onto the carrier.
/       d)Add each band’s output into one final mix. 
/--------------------------------------------------------------------------------------*/

    // Constructor: store how many bands you want and the envelope times
    VocoderEffect::VocoderEffect(int numBands, float attack, float release)
        : numBands_(numBands)// number of frequency bands to process
        , attackTime_(attack)// how fast envelopes react to rising signal
        , releaseTime_(release)// how fast envelopes react to falling signal
    {
    }

    // // initialize(): set up all your band filters, detectors, and modulators
    void VocoderEffect::initialize(int sampleRate, int channels, int framesPerBuffer) {
        bandFilters_.clear();
        detectors_.clear();
        modulators_.clear();

        // bandsCoeffs[i]
        extern const std::vector<std::vector<double>> BANDPASS_COEFFS;

        // for each band index b = 0…numBands_-1
        for (int b = 0; b < numBands_; ++b) {
            // 1) create a FIR band‑pass filter using your precomputed coeffs
            bandFilters_.emplace_back(BANDPASS_COEFFS[b]);
            
            // 2) create an envelope detector for this band
            detectors_.emplace_back(attackTime_, releaseTime_);
           
            // 3) pick a carrier frequency for this band
            //    we spread carriers logarithmically between 300 Hz and 3400 Hz
            float centerFreq = 300.0f * std::pow((3400.0f / 300.0f), b / float(numBands_ - 1));
            
            // 4) create an envelope modulator with that carrier
            modulators_.emplace_back(centerFreq, /*depth=*/1.0f);

            // 5) initialize each new object with sample rate & channel count
            bandFilters_.back().initialize(sampleRate, channels, framesPerBuffer);
            detectors_.back().initialize(sampleRate, channels, framesPerBuffer);
            modulators_.back().initialize(sampleRate, channels, framesPerBuffer);
        }
    }

    //  process(): run one block of audio through all bands and mix them
    std::vector<float> VocoderEffect::process(const std::vector<float>& input, int sampleRate) {

        // prepare a mix buffer the same size as input, filled with zeros
        std::vector<float> mix(input.size(), 0.0f), temp;

        // for each band:
        for (int b = 0; b < numBands_; ++b) {
            // a) filter the input into this band
            temp = bandFilters_[b].process(input, sampleRate);
            // b) extract the amplitude envelope of that band
            temp = detectors_[b].process(temp, sampleRate);
            // c) modulate a carrier by that envelope
            temp = modulators_[b].process(temp, sampleRate);
            // d) add this band’s result into the final mix
            for (size_t i = 0; i < mix.size(); ++i)
                mix[i] += temp[i];
        }

        return mix;
    }//End VocoderEffect

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
