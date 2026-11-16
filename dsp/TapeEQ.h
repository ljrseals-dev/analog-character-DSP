#pragma once
#include "Biquad.h"

namespace dsp {

class TapeEQ {
public:
    void prepare(float fs, float speedIps) {
        fs_ = fs;
        speedIps_ = speedIps;
        if (speedIps < 22.f) {
            bump_.set(Biquad::Type::Bell, fs, 60.f, 0.9f, +2.0f);
            hf_  .set(Biquad::Type::LowShelf, fs, 12000.f, 0.7f, -2.0f);
        } else {
            bump_.set(Biquad::Type::Bell, fs, 45.f, 0.9f, +0.8f);
            hf_  .set(Biquad::Type::LowShelf, fs, 18000.f, 0.7f, -1.0f);
        }
    }

    void setWear(float wear) {
        wear = std::clamp(wear, 0.f, 1.f);
        float baseGain = (speedIps_ < 22.f) ? -2.0f : -1.0f;
        float freq     = (speedIps_ < 22.f) ? 12000.f : 18000.f;
        hf_.set(Biquad::Type::LowShelf, fs_, freq, 0.7f, baseGain - 3.f * wear);
    }

    inline float process(float x) {
        return hf_.process(bump_.process(x));
    }

    void reset() { bump_.reset(); hf_.reset(); }

private:
    float fs_{48000.f}, speedIps_{15.f};
    Biquad bump_, hf_;
};

} // namespace dsp
