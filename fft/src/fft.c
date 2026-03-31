#include "fft.h"
#include <math.h>

static void swap(Complex* a, Complex* b) {
    Complex tmp = *a;
    *a = *b;
    *b = tmp;
}

static void bit_reverse(Complex* data, size_t N) {
    size_t j = 0;
    for (size_t i = 1; i < N; ++i) {
        size_t bit = N >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;

        if (i < j) {
            swap(&data[i], &data[j]);
        }
    }
}

void fft(Complex* data, size_t N) {
    // Reorder input data
    bit_reverse(data, N);

    // Do Butterflies
    for (size_t len = 2; len <= N; len <<= 1) {
        float angle = -2.0f * (float)M_PI / (float)len;
        Complex wLen = {cosf(angle), sinf(angle)};

        for (size_t i = 0; i < N; i += len) {
            Complex w = {1.0f, 0.0f};
            for (size_t k = 0; k < len / 2; ++k) {
                size_t idx1 = i + k;
                size_t idx2 = i + k + len / 2;

                Complex u = data[idx1];
                Complex v;  // w * data[idx2]

                // Multiply by a twiddle factor
                v.re = data[idx2].re * w.re - data[idx2].im * w.im;
                v.im = data[idx2].re * w.im + data[idx2].im * w.re;

                // Butterfly
                data[idx1].re = u.re + v.re;
                data[idx1].im = u.im + v.im;
                data[idx2].re = u.re - v.re;
                data[idx2].im = u.im - v.im;

                // Update twiddle factor: W_next = WLen * w
                float next_w_re = w.re * wLen.re - w.im * wLen.im;
                float next_w_im = w.re * wLen.im + w.im * wLen.re;
                w.re = next_w_re;
                w.im = next_w_im;
            }
        }
    }
}