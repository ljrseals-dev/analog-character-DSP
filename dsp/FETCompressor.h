#pragma once
#include "Utils.h"
#include <cmath>
#include <algorithm>

namespace dsp {

struct FETCurve {
    float vp{1.0f}, idss{1.0f}, bias{0.35f};

    inline float shape(float x) const {
        float vgs = bias - x * 0.5f;
        float n   = std::max(0.f, 1.f - vgs / vp);
        float id  = idss * n * n;
        float n0  = 1.f - bias / vp;
        float id0 = idss * n0 * n0;
        return (id - id0) / std::max(1e-6f, id0);
    }

    inline float antideriv(float x) const {
        float u = std::max(0.f, 1.f - (bias - x * 0.5f) / vp);
        return idss * (2.f * vp / 3.f) * u * u * u;
    }
};

struct InputTransformer {
    float a{0.05f}, b{0.02f};
    float lp{0.f}, coeff{0.f};

    void prepare(float fs) {
        coeff = 1.f - std::exp(-2.f * float(M_PI) * 80.f / fs);
    }

    inline float shape(float x) {
        lp += coeff * (x - lp);
        float low = lp;
        return low + a * low * low + b * low * low * low + (x - low);
    }

    inline float antideriv(float x) const {
        return 0.5f * x * x + (a / 3.f) * x * x * x + (b / 4.f) * x * x * x * x;
    }
};

struct OutputTransformer {
    float a3{0.02f};
    float lp{0.f}, coeff{0.f};

    void prepare(float fs) {
        coeff = 1.f - std::exp(-2.f * float(M_PI) * 18000.f / fs);
    }

    inline float shape(float x) {
        lp += coeff * (x - lp);
        return lp + a3 * lp * lp * lp;
    }

    inline float antideriv(float x) const {
        return 0.5f * x * x + (a3 / 4.f) * x * x * x * x;
    }
};

class FETCompressor {
public:
    enum class Ratio { R4 = 4, R8 = 8, R12 = 12, R20 = 20 };

    void prepare(float fs) {
        fs_ = fs;
        setAttack(20e-6f);
        setRelease(0.2f);
        inXf_.prepare(fs);
        outXf_.prepare(fs);
    }

    void setThresholdDb(float db) { thresholdDb_ = db; }
    void setRatio(Ratio r)        { ratio_ = float(int(r)); }
    void setAttack(float sec) {
        sec = std::clamp(sec, 20e-6f, 800e-6f);
        atkCoeff_ = std::exp(-1.f / (sec * fs_));
    }
    void setRelease(float sec) {
        sec = std::clamp(sec, 0.05f, 1.1f);
        relFastCoeff_ = std::exp(-1.f / (sec * fs_));
        relSlowCoeff_ = std::exp(-1.f / (3.f * sec * fs_));
    }

    inline float process(float x) {
        float xt = inXf_.shape(x);
        float yf = adaaFet(xt);
        float gr = computeGR(yf);
        return outXf_.shape(yf * gr);
    }

private:
    inline float adaaFet(float x) {
        float dx = x - xPrev_;
        float y = (std::fabs(dx) < 1e-6f)
            ? fet_.shape(0.5f * (x + xPrev_))
            : (fet_.antideriv(x) - fet_.antideriv(xPrev_)) / dx;
        xPrev_ = x;
        return y;
    }

    float computeGR(float x) {
        float level = std::fabs(x);
        envFast_ = (level > envFast_)
            ? level : atkCoeff_ * envFast_ + (1.f - atkCoeff_) * level;
        envSlow_ = (level > envSlow_)
            ? level : atkCoeff_ * envSlow_ + (1.f - atkCoeff_) * level;

        float envDb = 20.f * std::log10(envFast_ + 0.3f * envSlow_ + 1e-9f);
        float over  = envDb - thresholdDb_;
        const float knee = 6.f;
        float grDb = 0.f;
        if (over > knee * 0.5f)
            grDb = (over - knee * 0.5f) * (1.f - 1.f / ratio_);
        else if (over > -knee * 0.5f) {
            float k = over + knee * 0.5f;
            grDb = (k * k) / (2.f * knee) * (1.f - 1.f / ratio_);
        }

        float target = dbToGain(-grDb);
        float mix = std::clamp((grLin_ - target) * 4.f, 0.f, 1.f);
        float coeff = (target < grLin_) ? atkCoeff_
            : mix * relFastCoeff_ + (1.f - mix) * relSlowCoeff_;

        grLin_ = coeff * grLin_ + (1.f - coeff) * target;
        return grLin_;
    }

    float fs_{48000.f};
    FETCurve fet_;
    InputTransformer  inXf_;
    OutputTransformer outXf_;

    float xPrev_{0.f};
    float envFast_{0.f}, envSlow_{0.f}, grLin_{1.f};
    float atkCoeff_{0.f}, relFastCoeff_{0.f}, relSlowCoeff_{0.f};
    float thresholdDb_{-18.f}, ratio_{4.f};
};

} // namespace dsp
