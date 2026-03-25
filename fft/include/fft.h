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

#endif  // FFT_H