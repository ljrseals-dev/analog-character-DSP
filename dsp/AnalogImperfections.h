#pragma once
#include "Utils.h"
#include <cmath>

namespace dsp {

class AnalogImperfections {
public:
    void prepare(float fs, uint32_t seed = 1) {
        rng_ = seed | 1u;
        driftInc_ = 2.f * float(M_PI) * 0.15f / fs;
        humInc_   = 2.f * float(M_PI) * 60.f  / fs;
        noiseLpL_ = noiseLpR_ = 0.f;
    }

    void setHumFreq(float hz, float fs) {
        humInc_ = 2.f * float(M_PI) * hz / fs;
    }

    inline float noise() {
        rng_ ^= rng_ << 13;
        rng_ ^= rng_ >> 17;
        rng_ ^= rng_ << 5;
        return int32_t(rng_) * (1.f / 2147483648.f);
    }

    inline void apply(float& l, float& r,
                      float driftAmt, float humAmt, float noiseAmt)
    {
        driftPhase_ += driftInc_;
        if (driftPhase_ > 2.f * float(M_PI)) driftPhase_ -= 2.f * float(M_PI);
        float drift = 1.f + driftAmt * 0.002f * std::sin(driftPhase_);
        l *= drift;
        r *= drift;

        humPhase_ += humInc_;
        if (humPhase_ > 2.f * float(M_PI)) humPhase_ -= 2.f * float(M_PI);
        float hum = humAmt * 0.001f *
            (std::sin(humPhase_) + 0.5f * std::sin(2.f * humPhase_)
                                 + 0.25f * std::sin(3.f * humPhase_));
        l += hum;
        r += hum;

        float nl = noise(), nr = noise();
        noiseLpL_ += 0.02f * (nl - noiseLpL_);
        noiseLpR_ += 0.02f * (nr - noiseLpR_);
        l += noiseAmt * 0.0005f * noiseLpL_;
        r += noiseAmt * 0.0005f * noiseLpR_;
    }

private:
    uint32_t rng_{1};
    float driftPhase_{0}, driftInc_{0};
    float humPhase_{0},   humInc_{0};
    float noiseLpL_{0}, noiseLpR_{0};
};

} // namespace dsp
