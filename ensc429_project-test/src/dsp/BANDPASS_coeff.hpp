#pragma once
#include <vector>

namespace dsp
{
	// 8-band vocoder. Each band-pass f.i.r coefficient
	// This is just a statement, and the actual data is defined in BANDPASS_coeff.cpp
	extern const std::vector<std::vector<double>> BANDPASS_COEFFS;
} // namespace dsp

