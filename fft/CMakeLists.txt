cmake_minimum_required(VERSION 4.0.0)
project(FFTLibrary C)

add_library(fftLib STATIC
    src/fft.c
    src/dft.c
)

# Add target headers
target_include_directories(fftLib PUBLIC include)

# Build tests
enable_testing()
add_executable(test_fft tests/test_fft.c)
target_link_libraries(test_fft PRIVATE fftLib)
add_test(NAME TestFFT COMMAND test_fft)

# Build benchmarks
add_executable(bench_fft benchmarks/bench_fft.c)
target_link_libraries(bench_fft PRIVATE fftLib)

add_executable(bench_dft benchmarks/bench_dft.c)
target_link_libraries(bench_dft PRIVATE fftLib)
