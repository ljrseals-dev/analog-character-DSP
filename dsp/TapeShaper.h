#pragma once
#include "Utils.h"
#include <array>
#include <cmath>
#include <algorithm>

namespace dsp {

struct TapeFormula {
    const char* name;
    float knee;
    float driveScale;
    float asymBias;
    float hfDamp;
};

inline const std::array<TapeFormula, 4>& tapeFormulas() {
    static const std::array<TapeFormula, 4> F = {{
        {"456",   0.70f, 1.00f, 0.020f, 0.15f},
        {"GP9",   0.80f, 1.10f, 0.030f, 0.10f},
        {"911",   0.60f, 0.90f, 0.010f, 0.20f},
        {"SM900", 0.90f, 1.20f, 0.040f, 0.05f},
    }};
    return F;
}

struct TapeCurve {
    float knee{0.7f};
    float asymBias{0.02f};
    float driveScale{1.0f};

    inline float shape(float x) const {
        float b  = x * driveScale + asymBias;
        float ax = std::fabs(b);
        if (ax < knee) return (b - asymBias) / driveScale;
        float over = ax - knee;
        float comp = knee + over / (1.f + over);
        return (std::copysign(comp, b) - asymBias) / driveScale;
    }

    inline float antideriv(float x) const {
        float b  = x * driveScale + asymBias;
        float ab = std::fabs(b);
        if (ab <= knee)
            return 0.5f * b * b / driveScale;
        float over = ab - knee;
        float F = knee * ab + ab - std::log1p(over);
        return ((b < 0.f) ? -F : F) / driveScale;
    }
};

template <typename Curve>
class ADAAShaper {
public:
    void prepare(const Curve& c) { curve_ = c; xPrev_ = 0.f; }
    void setCurve(const Curve& c) { curve_ = c; }

    inline float process(float x) {
        float dx = x - xPrev_;
        float y = (std::fabs(dx) < 1e-6f)
            ? curve_.shape(0.5f * (x + xPrev_))
            : (curve_.antideriv(x) - curve_.antideriv(xPrev_)) / dx;
        xPrev_ = x;
        return y;
    }
private:
    Curve curve_;
    float xPrev_{0.f};
};

inline void applyBiasCal(TapeCurve& c, float biasNorm, float /*calDb*/) {
    c.knee     *= (0.5f + biasNorm);
    c.asymBias *= (1.f - biasNorm) * 0.5f;
}

} // namespace dsp
