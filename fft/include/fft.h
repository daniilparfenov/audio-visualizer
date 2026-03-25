#ifndef FFT_H
#define FFT_H

#include <stddef.h>

// Struct representing a complex number
typedef struct {
    float real;
    float imag;
} Complex;

// Discrete Fourier transform, O(N^2)
void dft(const Complex* input, Complex* output, size_t n);

// Fast Fourier transform, O(NlogN), N is the power of two
void fft(Complex* data, size_t n);

#endif  // FFT_H