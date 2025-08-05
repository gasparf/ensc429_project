// dsp/FIRDesign.hpp
#pragma once
#include <vector>
#include <cmath>

// Design of bandpass FIR with windowed©\sinc method
// lowcut, highcut unit Hz£»order must be even£»fs = smapling rate
static std::vector<double> designBandpassFIR(double lowcut,
    double highcut,
    int order,
    double fs)
{
    std::vector<double> h(order + 1);
    double fc1 = lowcut / fs;
    double fc2 = highcut / fs;
    int M = order;
    for (int n = 0; n <= M; ++n) {
        double m = n - M / 2.0;
        // Ideal bandpass sinc 
        double ideal;
        if (m == 0.0) {
            ideal = 2.0 * (fc2 - fc1);
        }
        else {
            ideal = (std::sin(2.0 * M_PI * fc2 * m)
                - std::sin(2.0 * M_PI * fc1 * m))
                / (M_PI * m);
        }
        // Hamming window
        double w = 0.54 - 0.46 * std::cos(2.0 * M_PI * n / M);
        h[n] = ideal * w;
    }
    return h;
}

