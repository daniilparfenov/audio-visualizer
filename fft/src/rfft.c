#include <math.h>
#include <stdlib.h>
#include "fft.h"

void rfft_magnitudes(const float* input, float* output_magnitudes, size_t N) {
    size_t half_N = N / 2;
    Complex* z = (Complex*)malloc(half_N * sizeof(Complex));
    if (!z)
        return;

    // Pack real signal as complex for FFT
    for (size_t i = 0; i < half_N; i++) {
        z[i].re = input[2 * i];
        z[i].im = input[2 * i + 1];
    }

    // Perform FFT
    fft(z, half_N);

    // Restore the spectrum of the original signal
    output_magnitudes[0] = fabsf(z[0].re + z[0].im);
    for (size_t k = 1; k < half_N; k++) {
        float angle = -2.0f * (float)M_PI * (float)k / (float)N;
        Complex twiddle = {cosf(angle), sinf(angle)};

        Complex z_k = z[k];
        Complex z_nk_conj = {z[half_N - k].re, -z[half_N - k].im};

        float E_re = (z_k.re + z_nk_conj.re) * 0.5f;
        float E_im = (z_k.im + z_nk_conj.im) * 0.5f;

        float O_re = (z_k.re - z_nk_conj.re) * 0.5f;
        float O_im = (z_k.im - z_nk_conj.im) * 0.5f;

        float M_re = O_re * twiddle.re - O_im * twiddle.im;
        float M_im = O_re * twiddle.im + O_im * twiddle.re;

        float out_re = E_re + M_im;
        float out_im = E_im - M_re;

        // Only magnitude is required for output
        output_magnitudes[k] = sqrtf(out_re * out_re + out_im * out_im);
    }

    free(z);
}