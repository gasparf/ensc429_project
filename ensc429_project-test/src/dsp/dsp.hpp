/*
// ENSC 429 Project - Real-Time Vocoder
//
// This project implements a real-time vocoder using PortAudio for audio input/output
// and a custom DSP processing chain for audio effects.
//
// File     : dsp.hpp
// Date     : 2025-07-18
// Authors  :
//      Keon Kye       301475751
*/

#pragma once

#include <vector>
#include <memory>
#include <string>
#include <cmath>// YY

namespace dsp {
constexpr float PI = 3.14159265358979323846;//YY updated: user-defined π constant, replacing M_PI.
extern const std::vector<std::vector<double>> BANDPASS_COEFFS;

// Forward declaration for the designed coefficients
extern const std::vector<double> DESIGNED_FIR_FILTER_COEFFICIENTS;

// FIR Filter Coefficients Configuration
extern const std::vector<float> FIR_FILTER_COEFFICIENTS;

// DSP Effect Classes

class DSPEffect {
public:
    virtual ~DSPEffect() = default;
    virtual std::vector<float> process(const std::vector<float>& input, int sampleRate) = 0;
    virtual void initialize(int sampleRate, int channels, int framesPerBuffer) = 0;
};


class GainEffect : public DSPEffect {
private:
    float gain;
    
public:
    explicit GainEffect(float gain = 1.0f);
    void setGain(float newGain);
    float getGain() const;
    
    std::vector<float> process(const std::vector<float>& input, int sampleRate) override;
    void initialize(int sampleRate, int channels, int framesPerBuffer) override;
};


class FIRFilter : public DSPEffect {
private:
    std::vector<float> coefficients;
    std::vector<float> delayLine;
    int delayIndex;
    
public:
    /**
     * @brief Default constructor - uses DESIGNED_FIR_FILTER_COEFFICIENTS
     */
    FIRFilter();
    
    /**
     * @brief Constructor with float coefficients
     * @param coeffs Vector of FIR filter coefficients (float)
     */
    explicit FIRFilter(const std::vector<float>& coeffs);
    
    /**
     * @brief Constructor with double coefficients
     * @param coeffs Vector of FIR filter coefficients (double)
     */
    explicit FIRFilter(const std::vector<double>& coeffs);
    
    /**
     * @brief Set FIR filter coefficients from float vector
     * @param coeffs Vector of filter coefficients
     */
    void setCoefficients(const std::vector<float>& coeffs);
    
    /**
     * @brief Set FIR filter coefficients from double vector
     * @param coeffs Vector of filter coefficients
     */
    void setCoefficients(const std::vector<double>& coeffs);
    
    /**
     * @brief Set FIR filter coefficients from a space-separated string
     * @param coeffString Space-separated coefficient values (e.g., "0.1 0.2 0.4 0.2 0.1")
     */
    void setCoefficientsFromString(const std::string& coeffString);
    
    /**
     * @brief Use the designed coefficients from FIR_coeff.hpp
     */
    void useDesignedCoefficients();
    
    /**
     * @brief Get current filter coefficients
     * @return Vector of current coefficients
     */
    const std::vector<float>& getCoefficients() const;
    
    /**
     * @brief Get the filter order (number of coefficients)
     * @return Filter order
     */
    size_t getFilterOrder() const;
    
    /**
     * @brief Clear the delay line (reset filter state)
     */
    void clearDelayLine();
    
    std::vector<float> process(const std::vector<float>& input, int sampleRate) override;
    void initialize(int sampleRate, int channels, int framesPerBuffer) override;
};

// ------------------------------
// Envelope Detector
// ------------------------------

/* -------------------------------------------------------------------------------------------------------------------------------------
/   YY
/   Why these fields?
/   1. attackTimeSec_ & releaseTimeSec_ let you tweak how fast the detector tracks changes.
/   2. The “coeff” values (attackCoeff_, releaseCoeff_) turn those times into persample multipliers (exp(–1/(τ·fs))).
/   3. envState_ holds the running envelope for each channel and lets us reference “previous” amplitude every time we process a new sample.
/-----------------------------------------------------------------------------------------------------------------------------------------*/

class EnvelopeDetector : public DSPEffect {
public:
    /* YY Constructor: you pass in two times (in seconds)
     attackTimeSec:   how quickly the detector follows rising amplitude  
     releaseTimeSec:  how quickly it “lets go” when amplitude falls */
    EnvelopeDetector(float attackTimeSec = 0.01f, 
                     float releaseTimeSec = 0.1f);
    /* YY Called once at start (or when sample rate / channel count changes)
     sampleRate:      samples per second, e.g. 44100  
     channels:        number of audio channels (1 = mono, 2 = stereo)  
     framesPerBuffer: how many samples you’ll process per callback */
    void initialize(int sampleRate, 
                    int channels, 
                    int framesPerBuffer) override;
    /* YY Called on each block of audio:
     input:           a flat array of floats, interleaved per channel  
     sampleRate:      same as above
     Returns another array of the same size: the instantaneous envelope */
    std::vector<float> process(const std::vector<float>& input, int sampleRate) override;

private:
    float attackTimeSec_;// stores the “attack” time you passed in
    float releaseTimeSec_;// stores the “release” time you passed in
    float attackCoeff_;// computed from attackTimeSec & sampleRate
    float releaseCoeff_;// computed from releaseTimeSec & sampleRate
    std::vector<float> envState_;  /*  One “state” per channel: keeps the last envelope value so
                                     that we can smooth rise/fall over time, separately on each channel. */
};

// ------------------------------
// Envelope Modulator
// ------------------------------

/* ------------------------------------------------------------------
/    YY
/    What happens in process()?
/    1. Read the envelope value (input[i]).
/    2. Compute one sine sample at carrierPhase_.
/    3. Multiply: output = depth_ * envelope * carrier.
/    4. Advance carrierPhase_ by phaseIncrement_, wrapping every 2π.
/---------------------------------------------------------------------*/
class EnvelopeModulator : public DSPEffect {
public:
    /*  Constructor : pick a carrier frequency(Hz) and depth[0…1]
     carrierFreqHz:   the sine‑wave you’ll use to modulate  
     depth:           how strongly the envelope affects the carrier */
    //  carrierFreqHz：(Hz)，depth：[0,1]
    EnvelopeModulator(float carrierFreqHz = 440.0f, float depth = 1.0f);
    //  Called once at start (or when sample rate / channel count changes)
    void initialize(int sampleRate, int channels, int framesPerBuffer) override;
    //  Called each audio block
    std::vector<float> process(const std::vector<float>& input, int sampleRate) override;

private:
    float carrierFreqHz_;//  stores the carrier frequency you passed in
    float carrierPhase_;//  current angle of the sine wave, in radians
    float phaseIncrement_;//  equals 2π·carrierFreqHz_/sampleRate
    float depth_;//  how much envelope modulates the carrier
    int   channels_;  // ← How many channels are recorded
};

// YY OPTIONAL: 8-band speech coder(Example)
class VocoderEffect : public DSPEffect {
public:
    VocoderEffect(int numBands = 8, float attack = 0.01f, float release = 0.1f);

    void initialize(int sampleRate, int channels, int framesPerBuffer) override;
    std::vector<float> process(const std::vector<float>& input, int sampleRate) override;

private:
    int numBands_;
    float attackTime_, releaseTime_;
    std::vector<FIRFilter>         bandFilters_;
    std::vector<EnvelopeDetector>  detectors_;
    std::vector<EnvelopeModulator> modulators_;
};//End example

class DSPProcessor {
private:
    std::vector<std::unique_ptr<DSPEffect>> effects;
    int sampleRate;
    int channels;
    int framesPerBuffer;
    bool bypass;
    
public:
    DSPProcessor();
    ~DSPProcessor() = default;

    void initialize(int sampleRate, int channels, int framesPerBuffer);
    void addEffect(std::unique_ptr<DSPEffect> effect);
    void clearEffects();
    std::vector<float> process(const std::vector<float>& input);
    void setBypass(bool enabled);
    bool isBypassed() const;
    size_t getEffectCount() const;
};

} // namespace dsp
