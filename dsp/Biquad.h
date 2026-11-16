#pragma once
#include <cmath>
#include <algorithm>

namespace dsp {

class Biquad {
public:
    enum class Type { LowShelf, HighShelf, Bell };

    void set(Type t, float fs, float f0, float Q, float gainDb) {
        const float A     = std::pow(10.f, gainDb / 40.f);
        const float w0    = 2.f * float(M_PI) * std::clamp(f0, 10.f, fs * 0.499f) / fs;
        const float cw    = std::cos(w0);
        const float sw    = std::sin(w0);
        const float alpha = sw / (2.f * std::max(0.05f, Q));

        float b0, b1, b2, a0, a1, a2;

        switch (t) {
        case Type::LowShelf: {
            const float sqA = std::sqrt(A);
            const float twoSqAAlpha = 2.f * sqA * alpha;
            b0 =    A * ((A + 1.f) - (A - 1.f) * cw + twoSqAAlpha);
            b1 =  2.f * A * ((A - 1.f) - (A + 1.f) * cw);
            b2 =    A * ((A + 1.f) - (A - 1.f) * cw - twoSqAAlpha);
            a0 =        (A + 1.f) + (A - 1.f) * cw + twoSqAAlpha;
            a1 = -2.f * ((A - 1.f) + (A + 1.f) * cw);
            a2 =        (A + 1.f) + (A - 1.f) * cw - twoSqAAlpha;
            break;
        }
        case Type::HighShelf: {
            const float sqA = std::sqrt(A);
            const float twoSqAAlpha = 2.f * sqA * alpha;
            b0 =    A * ((A + 1.f) + (A - 1.f) * cw + twoSqAAlpha);
            b1 = -2.f * A * ((A - 1.f) + (A + 1.f) * cw);
            b2 =    A * ((A + 1.f) + (A - 1.f) * cw - twoSqAAlpha);
            a0 =        (A + 1.f) - (A - 1.f) * cw + twoSqAAlpha;
            a1 =  2.f * ((A - 1.f) - (A + 1.f) * cw);
            a2 =        (A + 1.f) - (A - 1.f) * cw - twoSqAAlpha;
            break;
        }
        case Type::Bell:
        default: {
            b0 = 1.f + alpha * A;
            b1 = -2.f * cw;
            b2 = 1.f - alpha * A;
            a0 = 1.f + alpha / A;
            a1 = -2.f * cw;
            a2 = 1.f - alpha / A;
            break;
        }
        }

        const float inv = 1.f / a0;
        b0_ = b0 * inv; b1_ = b1 * inv; b2_ = b2 * inv;
        a1_ = a1 * inv; a2_ = a2 * inv;
    }

    void reset() { z1_ = z2_ = 0.f; }

    inline float process(float x) {
        const float y = b0_ * x + z1_;
        z1_ = b1_ * x - a1_ * y + z2_;
        z2_ = b2_ * x - a2_ * y;
        return y;
    }

private:
    float b0_{1}, b1_{0}, b2_{0}, a1_{0}, a2_{0};
    float z1_{0}, z2_{0};
};

class FourBandEQ {
public:
    struct Params {
        float lowGainDb{0}, lowFreqHz{80.f};
        float midLoGainDb{0}, midLoFreqHz{500.f}, midLoQ{0.7f};
        float midHiGainDb{0}, midHiFreqHz{3000.f}, midHiQ{0.7f};
        float highGainDb{0}, highFreqHz{10000.f};
    };

    void prepare(float fs) { fs_ = fs; reset(); }
    void reset() {
        low_.reset(); midLo_.reset(); midHi_.reset(); high_.reset();
    }

    void update(const Params& p) {
        low_  .set(Biquad::Type::LowShelf,  fs_, p.lowFreqHz,  0.60f, p.lowGainDb);
        midLo_.set(Biquad::Type::Bell,      fs_, p.midLoFreqHz, std::clamp(p.midLoQ, 0.3f, 1.2f), p.midLoGainDb);
        midHi_.set(Biquad::Type::Bell,      fs_, p.midHiFreqHz, std::clamp(p.midHiQ, 0.3f, 1.2f), p.midHiGainDb);
        high_ .set(Biquad::Type::HighShelf, fs_, p.highFreqHz, 0.50f, p.highGainDb);
    }

    inline float process(float x) {
        x = low_.process(x);
        x = midLo_.process(x);
        x = midHi_.process(x);
        x = high_.process(x);
        return x;
    }

private:
    float fs_{48000.f};
    Biquad low_, midLo_, midHi_, high_;
};

} // namespace dsp
