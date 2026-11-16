#include "../dsp/KaiserFIR.h"
#include <cstdio>

int main() {
    auto h = dsp::designKaiserLowpass(48000.0 * 16, 48000.0 * 0.45, 48000.0 * 0.05, 80.0);
    double sum = 0.0;
    for (float c : h) sum += c;
    std::printf("FIR length = %zu\n", h.size());
    std::printf("DC gain    = %.8f  (should be ~1.0)\n", sum);
    return 0;
}
