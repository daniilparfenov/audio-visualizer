#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "fft.h"

#define N (1024)
#define ITERATIONS 100

static void generate_dummy_data(float* signal, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        signal[i] = (float)rand() / (float)RAND_MAX;
    }
}

int main() {
    printf("RFFT (Magnitude) benchmarks\n");
    printf("------------------------------\n");

    // Init bufffers
    float* input = malloc(N * sizeof(float));
    float* output = malloc(N / 2 * sizeof(float));
    if (!input || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // Generate input data
    generate_dummy_data(input, N);

    // Benchmark
    clock_t start_time = clock();
    for (int i = 0; i < ITERATIONS; ++i) {
        rfft_magnitudes(input, output, N);
    }
    clock_t end_time = clock();

    // Get Benchmark results
    double total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    double time_per_run_ms = (total_time / ITERATIONS) * 1000.0;

    printf("Size (N)          : %d\n", N);
    printf("Iterations        : %d\n", ITERATIONS);
    printf("Total time        : %.4f sec\n", total_time);
    printf("Avg time per RFFT : %.4f ms\n", time_per_run_ms);

    // Free buffers
    free(input);

    return 0;
}