#include <math.h>
#include "fft.h"

void dft(const Complex* input, Complex* output, size_t N) {
    for (size_t k = 0; k < N; ++k) {
        float outRe = 0, outIm = 0;
        for (size_t n = 0; n < N; ++n) {
            float arg = 2.0 * M_PI * k * n / N;
            float twdRe = cosf(arg);
            float twdIm = -sinf(arg);

            float inRe = input[n].re;
            float inIm = input[n].im;

            outRe += inRe * twdRe - inIm * twdIm;
            outIm += inIm * twdRe + inRe * twdIm;
        }
        output[k].re = outRe;
        output[k].im = outIm;
    }
}
