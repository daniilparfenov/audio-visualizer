#ifndef FFT_H
#define FFT_H

#include <stddef.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Struct representing a complex number
typedef struct {
    float re;
    float im;
} Complex;

// Discrete Fourier transform, O(N^2)
void dft(const Complex* input, Complex* output, size_t N);

// Fast Fourier transform, O(NlogN), N is the power of two
void fft(Complex* data, size_t N);

// Returns the array of frequency magnitudes
// Input array size: N (the power of two)
// Output array size: N / 2
void rfft_magnitudes(const float* input, float* output_magnitudes, size_t N);

// Applies a Hamming window to a real-valued signal buffer in-place.
// Input/Output: signal (array of N floats)
void apply_hamming_window(float* signal, size_t N);

#endif  // FFT_H