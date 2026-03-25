#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "fft.h"

// Generates harmonic: 'cycles' is the number of full waves in N samples
void generate_harmonic(Complex* signal, size_t N, int freq) {
    for (size_t i = 0; i < N; ++i) {
        signal[i].re = sinf(2.0f * (float)M_PI * freq * (float)i / (float)N);
        signal[i].im = 0.0f;
    }
}

static void test_dft(void) {
    // Generate signal with one harmonic
    int freq = 12;
    size_t N = 1024;
    Complex* signal = malloc(N * sizeof(Complex));
    Complex* output = malloc(N * sizeof(Complex));
    generate_harmonic(signal, N, freq);

    // Run DFT
    dft(signal, output, N);

    // Print output (spectrum): peaks should be in the freq and N - freq index.
    for (size_t i = 0; i < N; ++i) {
        float re = output[i].re;
        float im = output[i].im;
        float amplitude = sqrt(re * re + im * im);
        printf("%zu) %f\n", i, amplitude);
    }

    free(signal);
    free(output);
}

int main() {

    test_dft();

    return 0;
}
