
/*
// ENSC 429 Project - Real-Time Vocoder
//
// This project implements a real-time vocoder using PortAudio for audio input/output
// and a custom DSP processing chain for audio effects.
//
// File     : FIR_coeff.hpp
// Date     : 2025-07-18
// Authors  :
//      Keon Kye       301475751
*/


#pragma once

#include <vector>

namespace dsp {
	/*
	sampling frequency: 44100 Hz

	* 0 Hz - 250 Hz
	  gain = 0
	  desired attenuation = -40 dB
	  actual attenuation = -40.16667267926087 dB

	* 300 Hz - 900 Hz
	  gain = 1
	  desired ripple = 1 dB
	  actual ripple = 0.7377212395392587 dB

	* 950 Hz - 22050 Hz
	  gain = 0
	  desired attenuation = -40 dB
	  actual attenuation = -40.16667267926087 dB
	*/
#define FILTER_TAP_NUM 1391
	extern const std::vector<double> DESIGNED_FIR_FILTER_COEFFICIENTS;
} // namespace dsp

