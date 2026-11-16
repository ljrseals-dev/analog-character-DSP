#pragma once
#include <cmath>
#include <array>
#include <random>
#include <cstdint>
#include <algorithm>

namespace dsp {

inline float dbToGain(float db) noexcept {
    return std::pow(10.f, db * 0.05f);
}

inline float gainToDb(float g) noexcept {
    return 20.f * std::log10(std::max(1e-12f, g));
}

// One-pole parameter smoother
class SmoothedValue {
public:
    void prepare(float fs, float timeMs = 20.f) {
        coeff_ = std::exp(-1.f / (0.001f * timeMs * fs));
    }
    void setTarget(float t) noexcept { target_ = t; }
    void snap(float v) noexcept { current_ = target_ = v; }
    float next() noexcept {
        current_ += (1.f - coeff_) * (target_ - current_);
        return current_;
    }
    float current() const noexcept { return current_; }
private:
    float coeff_{0.999f}, current_{0.f}, target_{0.f};
};

// 48-channel static tolerance (TMT-style)
struct ChannelTolerance {
    float gainTrimDb  = 0.f;
    float eqTiltDb    = 0.f;
    float satBias     = 0.f;
    float compOffset  = 0.f;
    uint32_t noiseSeed = 1;
};

inline std::array<ChannelTolerance, 48> buildToleranceTable(uint32_t seed = 0xC0FFEE) {
    std::array<ChannelTolerance, 48> t{};
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> u(-1.f, 1.f);
    for (auto& c : t) {
        c.gainTrimDb  = u(rng) * 0.35f;
        c.eqTiltDb    = u(rng) * 0.15f;
        c.satBias     = u(rng) * 0.02f;
        c.compOffset  = u(rng) * 0.10f;
        c.noiseSeed   = rng();
    }
    return t;
}

} // namespace dsp
