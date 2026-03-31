#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "fft.h"

#define SNR_THRESHOLD 80.0f
#define ABS_THRESHOLD 0.0001f
// #define DEBUG_LOG

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

// Fills array of float numbers with random data
static void generate_rand_float(float* signal, size_t N) {
    time_t seed = time(NULL);
    srand(seed);

#if defined(DEBUG_LOG)
    printf("srand seed = %ld\n", seed);
#endif

    float mn = -10.0f;
    float mx = 10.0f;
    for (size_t i = 0; i < N; ++i) {
        signal[i] = mn + ((float)rand() / (float)RAND_MAX) * (mx - mn);
    }
}

// Calculates Signal-to-Noise Ratio (SNR) for Complex arrays in dB
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

// Calculates Signal-to-Noise Ratio (SNR) for float arrays in dB
static float calculate_snr_float(const float* ref, const float* test, size_t len) {
    float signal_power = 0.0f;
    float noise_power = 0.0f;

    for (size_t i = 0; i < len; ++i) {
        float ref_val = ref[i];
        float diff = ref_val - test[i];

        signal_power += ref_val * ref_val;
        noise_power += diff * diff;
    }

    if (noise_power == 0.0f) {
        return INFINITY;
    }

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
        printf("FAILED: test_fft with SNR = %.2f\n", SNR);
        all_test_passed = 0;
    } else {
        printf("PASSED: test_fft (SNR: %.2f dB)\n", SNR);
    }

#if defined(DEBUG_LOG)
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

static void test_rfft_magnitudes(void) {
    // Generate random real signal
    size_t N = 1024;
    size_t half_N = N / 2;
    float* real_signal = malloc(N * sizeof(float));
    generate_rand_float(real_signal, N);

    // Output arrays of freq magnitudes
    float* rfft_freq_magnitudes = malloc(half_N * sizeof(float));
    float* dft_freq_magnitudes = malloc(half_N * sizeof(float));

    // Pack real signal as complex for DFT
    Complex* dft_in = malloc(N * sizeof(Complex));
    Complex* dft_out = malloc(N * sizeof(Complex));
    for (size_t i = 0; i < N; ++i) {
        dft_in[i].re = real_signal[i];
        dft_in[i].im = 0.0f;
    }

    // Run DFT and RFFT
    dft(dft_in, dft_out, N);
    rfft_magnitudes(real_signal, rfft_freq_magnitudes, N);

    // Calc freq magnitudes from DFT output
    for (size_t i = 0; i < half_N; i++) {
        dft_freq_magnitudes[i] = sqrtf(dft_out[i].re * dft_out[i].re + dft_out[i].im * dft_out[i].im);
    }

    // Compare results
    float SNR = calculate_snr_float(dft_freq_magnitudes, rfft_freq_magnitudes, half_N);
    if (SNR < SNR_THRESHOLD) {
        printf("FAILED: test_rfft_magnitudes with SNR = %f\n", SNR);
        all_test_passed = 0;
    } else {
        printf("PASSED: test_rfft_magnitudes (SNR: %.2f dB)\n", SNR);
    }

#if defined(DEBUG_LOG)
    for (size_t i = 0; i < half_N; ++i) {
        float dft_mag = dft_freq_magnitudes[i];
        float rfft_mag = rfft_freq_magnitudes[i];
        float diff = fabs(dft_mag - rfft_mag);
        printf("%zu) DFT_mag RFFT_mag Diff: %f %f %f\n", i, dft_mag, rfft_mag, diff);
    }
#endif  // defined(DEBUG_LOG)

    free(real_signal);
    free(rfft_freq_magnitudes);
    free(dft_freq_magnitudes);
    free(dft_in);
    free(dft_out);
}

int main() {

    test_dft();
    test_fft();
    test_rfft_magnitudes();

    if (all_test_passed) {
        printf("All tests passed!\n");
    } else {
        printf("There is some errors!\n");
    }

    return 0;
}
