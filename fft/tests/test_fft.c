#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "fft.h"

#define SNR_THRESHOLD 80.0f
#define ABS_THRESHOLD 0.0001f
#define DEBUG_LOG

static int all_test_passed = 1;

// Generates harmonic
static void generate_harmonic(Complex* signal, size_t N, int freq) {
    for (size_t i = 0; i < N; ++i) {
        signal[i].re = sinf(2.0f * (float)M_PI * freq * (float)i / (float)N);
        signal[i].im = 0.0f;
    }
}

// Fills array of Complex numbers with random data
static void generate_rand_complex(Complex* signal, size_t N) {
    time_t seed = time(NULL);
    srand(seed);

#if defined(DEBUG_LOG)
    printf("srand seed = %ld\n", seed);
#endif

    float mn = -10.0f;
    float mx = 10.0f;

    for (size_t i = 0; i < N; ++i) {
        signal[i].re = mn + ((float)rand() / (float)RAND_MAX) * (mx - mn);
        signal[i].im = mn + ((float)rand() / (float)RAND_MAX) * (mx - mn);
    }
}

// Calculates Signal-to-Noise Ratio (SNR) in dB
static float calculate_snr(const Complex* ref, const Complex* test, size_t N) {
    float signal_power = 0.0f;
    float noise_power = 0.0f;

    for (size_t i = 0; i < N; ++i) {
        // The power of reference signal
        float re_ref = ref[i].re;
        float im_ref = ref[i].im;
        signal_power += (re_ref * re_ref) + (im_ref * im_ref);

        // The power of noise
        float diff_re = ref[i].re - test[i].re;
        float diff_im = ref[i].im - test[i].im;
        noise_power += (diff_re * diff_re) + (diff_im * diff_im);
    }

    // If noise power is zero, SNR is +inf
    if (noise_power == 0.0f) {
        return INFINITY;
    }

    // Return SNR in dB
    return 10.0f * log10f(signal_power / noise_power);
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

    // Check (peaks should be in the freq and N - freq index)
    // Their total amplitude should be nearly 1.0f
    Complex* peak1 = &output[freq];
    Complex* peak2 = &output[N - freq];
    float amp1 = sqrt(peak1->re * peak1->re + peak1->im * peak1->im);
    float amp2 = sqrt(peak2->re * peak2->re + peak2->im * peak2->im);
    float diff = fabs(amp1 - amp2);
    if (diff > ABS_THRESHOLD) {
        printf("FAILED: test_dft with ABS = %f\n", diff);
        all_test_passed = 0;
    } else {
        printf("PASSED: test_dft\n");
    }

#if defined(DEBUG_LOG)
    // Print output (spectrum): peaks should be in the freq and N - freq index.
    for (size_t i = 0; i < N; ++i) {
        float re = output[i].re;
        float im = output[i].im;
        float amplitude = sqrt(re * re + im * im);
        printf("%zu) %f\n", i, amplitude);
    }
#endif  // defined(DEBUG_LOG)

    free(signal);
    free(output);
}

static void test_fft(void) {
    // Generate random signal
    size_t N = 1024;
    Complex* fft_in_out = malloc(N * sizeof(Complex));
    generate_rand_complex(fft_in_out, N);

    Complex* dft_in = malloc(N * sizeof(Complex));
    Complex* dft_out = malloc(N * sizeof(Complex));
    memcpy(dft_in, fft_in_out, N * sizeof(Complex));

    // Run DFT and FFT
    dft(dft_in, dft_out, N);
    fft(fft_in_out, N);

    float SNR = calculate_snr(dft_out, fft_in_out, N);
    if (SNR < SNR_THRESHOLD) {
        printf("FAILED: test_fft with SNR = %f\n", SNR);
        all_test_passed = 0;
    } else {
        printf("PASSED: test_fft\n");
    }

#if defined(DEBUG_LOG)
    // Print output (spectrum): peaks should be in the freq and N - freq index.
    for (size_t i = 0; i < N; ++i) {
        float re = dft_out[i].re;
        float im = dft_out[i].im;
        float dft_out = sqrt(re * re + im * im);

        re = fft_in_out[i].re;
        im = fft_in_out[i].im;
        float fft_out = sqrt(re * re + im * im);

        printf("%zu) DFT FFT: %f %f %f\n", i, dft_out, fft_out, fabs(fft_out - dft_out));
    }
#endif  // defined(DEBUG_LOG)

    free(fft_in_out);
    free(dft_in);
    free(dft_out);
}

int main() {

    test_dft();
    test_fft();

    if (all_test_passed) {
        printf("All tests passed!\n");
    } else {
        printf("There is some errors!\n");
    }

    return 0;
}
