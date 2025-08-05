#pragma once
#include <vector>

namespace dsp {

    // Each Effect inherits effect and implements process ()
    class Effect {
    public:
        virtual ~Effect() = default;
        virtual void process(std::vector<float>& buffer) = 0;
    };

} // namespace dsp

