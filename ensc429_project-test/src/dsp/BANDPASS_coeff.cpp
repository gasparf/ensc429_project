#include "dsp.hpp"

namespace dsp {

    /* Here is the simplest example: 8-segment bandpass, and each segment temporarily uses the same set of FIR coefficients. 
    You can replace it with a real band-pass coefficient array as needed.*/

    /*If you have generated 8 different sets of bandpass FIR coefficients with Python / Matlab,
    just replace each line of the above example with std::vector<float>{...} of your corresponding segment.
    For example : 
    -------------------------------------------------------------------------------------------------------------
    const std::vector<std::vector<float>> BANDPASS_COEFFS = {
    {0.001f, 0.002f, 0.001f, ...},   // 300–500 Hz 
    {0.0005f, 0.001f, ...},          // 500–800 Hz 
    {...}                            // last
};
-----------------------------------------------------------------------------------------------------------------*/
    const std::vector<std::vector<double>> BANDPASS_COEFFS = {
        DESIGNED_FIR_FILTER_COEFFICIENTS,  // 0
        DESIGNED_FIR_FILTER_COEFFICIENTS,  // 1
        DESIGNED_FIR_FILTER_COEFFICIENTS,  // 2
        DESIGNED_FIR_FILTER_COEFFICIENTS,  // 3
        DESIGNED_FIR_FILTER_COEFFICIENTS,  // 4
        DESIGNED_FIR_FILTER_COEFFICIENTS,  // 5
        DESIGNED_FIR_FILTER_COEFFICIENTS,  // 6
        DESIGNED_FIR_FILTER_COEFFICIENTS   // 7
    };

} // namespace dsp
