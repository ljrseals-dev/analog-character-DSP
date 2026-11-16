#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

namespace dsp {

class TapeTransport {
public:
    static constexpr int MAX_DELAY = 4096;

    void prepare(float fs) {
        fs_ = fs;
        dlL_.assign(MAX_DELAY, 0.f);
        dlR_.assign(MAX_DELAY, 0.f);
        write_ = 0;
        baseDelay_ = 0.002f * fs;
        wowInc_   = 2.f * float(M_PI) * 0.7f  / fs;
        flutInc_  = 2.f * float(M_PI) * 8.0f  / fs;
        driftInc_ = 2.f * float(M_PI) * 0.08f / fs;
    }

    inline void process(float& l, float& r,
                        float wow, float flut, float drift)
    {
        wowPhase_  += wowInc_;
        flutPhase_ += flutInc_;
        driftPhase_+= driftInc_;
        if (wowPhase_  > 2.f * float(M_PI)) wowPhase_  -= 2.f * float(M_PI);
        if (flutPhase_ > 2.f * float(M_PI)) flutPhase_ -= 2.f * float(M_PI);
        if (driftPhase_> 2.f * float(M_PI)) driftPhase_-= 2.f * float(M_PI);

        float mod = wow  * 0.0030f * std::sin(wowPhase_)
                  + flut * 0.0004f * std::sin(flutPhase_)
                  + drift* 0.0010f * std::sin(driftPhase_);

        float d = baseDelay_ * (1.f + mod);   // stereo-linked

        dlL_[write_] = l;
        dlR_[write_] = r;
        l = read(dlL_, d);
        r = read(dlR_, d);
        write_ = (write_ + 1) % MAX_DELAY;
    }

private:
    float read(const std::vector<float>& dl, float d) const {
        float rp = float(write_) - d;
        while (rp < 0.f) rp += MAX_DELAY;
        int i0 = int(rp);
        int i1 = (i0 + 1) % MAX_DELAY;
        float f = rp - float(i0);
        return dl[i0] * (1.f - f) + dl[i1] * f;
    }

    float fs_{48000.f};
    std::vector<float> dlL_, dlR_;
    int write_{0};
    float baseDelay_{0.f};
    float wowPhase_{0}, flutPhase_{0}, driftPhase_{0};
    float wowInc_{0}, flutInc_{0}, driftInc_{0};
};

class TapeNoise {
public:
    uint32_t rng{1};
    float lpL{0}, lpR{0};

    inline float white() {
        rng ^= rng << 13;
        rng ^= rng >> 17;
        rng ^= rng << 5;
        return int32_t(rng) * (1.f / 2147483648.f);
    }

    inline void apply(float& l, float& r, float hiss, float wear) {
        float nl = white(), nr = white();
        lpL += 0.05f * (nl - lpL);
        lpR += 0.05f * (nr - lpR);
        float amt = hiss * 0.0008f * (1.f + wear * 0.5f);
        l += amt * lpL;
        r += amt * lpR;
    }
};

class TapeGlue {
public:
    void prepare(float fs) {
        atk_ = std::exp(-1.f / (0.010f * fs));
        rel_ = std::exp(-1.f / (0.300f * fs));
    }

    inline float process(float x, float amt) {
        float a = std::fabs(x);
        env_ = (a > env_) ? atk_ * env_ + (1.f - atk_) * a
                          : rel_ * env_ + (1.f - rel_) * a;
        float gr = (env_ > 0.25f) ? (env_ - 0.25f) * amt : 0.f;
        return x * (1.f - gr);
    }
private:
    float env_{0.f}, atk_{0.f}, rel_{0.f};
};

} // namespace dsp
