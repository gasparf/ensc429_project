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
#include <cmath>// New added by YY

namespace dsp {
constexpr float PI = 3.14159265358979323846;
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
class EnvelopeDetector : public DSPEffect {
public:
    // attackTimeSec£ºSound Rise Detection Speed (sec)
    // releaseTimeSec£ºSound drop detection speed (seconds)
    EnvelopeDetector(float attackTimeSec = 0.01f, float releaseTimeSec = 0.1f);

    void initialize(int sampleRate, int channels, int framesPerBuffer) override;
    std::vector<float> process(const std::vector<float>& input, int sampleRate) override;

private:
    float attackTimeSec_;
    float releaseTimeSec_;
    float attackCoeff_;
    float releaseCoeff_;
    std::vector<float> envState_;  // ¡û Will be allocated according to the size of the channels passed in initialize.
};

// ------------------------------
// Envelope Modulator
// ------------------------------
class EnvelopeModulator : public DSPEffect {
public:
    // carrierFreqHz£º(Hz)£¬depth£º[0,1]
    EnvelopeModulator(float carrierFreqHz = 440.0f, float depth = 1.0f);

    void initialize(int sampleRate, int channels, int framesPerBuffer) override;
    std::vector<float> process(const std::vector<float>& input, int sampleRate) override;

private:
    float carrierFreqHz_;
    float carrierPhase_;
    float phaseIncrement_;
    float depth_;
    int   channels_;  // ¡û How many channels are recorded
};

/// 8-band speech coder
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
};

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
