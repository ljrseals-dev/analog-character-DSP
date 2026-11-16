#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <cassert>

namespace dsp {

inline double besselI0(double x) {
    double sum = 1.0, term = 1.0;
    const double x2 = 0.25 * x * x;
    for (int k = 1; k < 64; ++k) {
        term *= x2 / (double(k) * double(k));
        sum  += term;
        if (term < 1e-20 * sum) break;
    }
    return sum;
}

inline double kaiserBeta(double attenDb) {
    if (attenDb > 50.0) return 0.1102 * (attenDb - 8.7);
    if (attenDb > 21.0) return 0.5842 * std::pow(attenDb - 21.0, 0.4) + 0.07886 * (attenDb - 21.0);
    return 0.0;
}

inline std::vector<float> designKaiserLowpass(
    double fs, double cutoffHz, double transitionHz, double attenDb)
{
    const double beta = kaiserBeta(attenDb);
    const double dW   = 2.0 * M_PI * transitionHz / fs;
    int N = (int)std::ceil((attenDb - 7.95) / (2.285 * dW));
    if (N < 15) N = 15;
    if ((N & 1) == 0) ++N;
    const int M = (N - 1) / 2;

    std::vector<float> h(N);
    const double fc  = cutoffHz / fs;
    const double I0b = besselI0(beta);

    double sum = 0.0;
    for (int n = 0; n < N; ++n) {
        const int k = n - M;
        const double sinc = (k == 0) ? 1.0
            : std::sin(2.0 * M_PI * fc * k) / (M_PI * k);
        const double arg = double(k) / double(M);
        const double w   = besselI0(beta * std::sqrt(std::max(0.0, 1.0 - arg * arg))) / I0b;
        h[n] = float(sinc * w);
        sum += h[n];
    }
    for (auto& c : h) c = float(c / sum);
    return h;
}

class PolyphaseFIR {
public:
    void init(const std::vector<float>& prototype, int phases) {
        phases_ = phases;
        const int N = int(prototype.size());
        L_ = N / phases_;
        assert(L_ * phases_ == N);

        poly_.assign(phases_, std::vector<float>(L_));
        for (int p = 0; p < phases_; ++p)
            for (int k = 0; k < L_; ++k)
                poly_[p][k] = prototype[p + phases_ * k];

        hist_.assign(L_, 0.f);
        write_ = 0;
    }

    void push(float x) {
        hist_[write_] = x;
        write_ = (write_ + 1) % L_;
    }

    float conv(int phase) const {
        float acc = 0.f;
        int idx = write_;
        for (int k = 0; k < L_; ++k) {
            idx = (idx + L_ - 1) % L_;
            acc += hist_[idx] * poly_[phase][k];
        }
        return acc;
    }

private:
    int phases_{2}, L_{0}, write_{0};
    std::vector<std::vector<float>> poly_;
    std::vector<float> hist_;
};

class Oversampler {
public:
    void prepare(double baseFs, int factor, int numChannels) {
        factor_ = std::max(1, factor);
        numCh_  = numChannels;

        const double highFs     = baseFs * factor_;
        const double cutoff     = baseFs * 0.45;
        const double transition = baseFs * 0.05;

        auto proto = designKaiserLowpass(highFs, cutoff, transition, 80.0);
        int pad = (factor_ - int(proto.size()) % factor_) % factor_;
        proto.insert(proto.end(), pad, 0.f);

        up_.resize(numCh_);
        down_.resize(numCh_);
        for (int c = 0; c < numCh_; ++c) {
            up_[c].init(proto, factor_);
            down_[c].init(proto, factor_);
        }
    }

    void upBlock(const float* const* in, float* const* out, int numIn) {
        for (int c = 0; c < numCh_; ++c) {
            auto& r = up_[c];
            for (int n = 0; n < numIn; ++n) {
                r.push(in[c][n]);
                for (int p = 0; p < factor_; ++p)
                    out[c][n * factor_ + p] = r.conv(p);
            }
        }
    }

    void downBlock(const float* const* in, float* const* out, int numOut) {
        for (int c = 0; c < numCh_; ++c) {
            auto& r = down_[c];
            for (int n = 0; n < numOut; ++n) {
                for (int p = factor_ - 1; p >= 0; --p)
                    r.push(in[c][n * factor_ + p]);
                out[c][n] = r.conv(0);
            }
        }
    }

    int factor() const { return factor_; }

private:
    int factor_{2}, numCh_{2};
    std::vector<PolyphaseFIR> up_, down_;
};

} // namespace dsp
