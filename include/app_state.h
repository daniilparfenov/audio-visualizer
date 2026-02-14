#ifndef APP_STATE_H
#define APP_STATE_H

#include <SDL3/SDL.h>

typedef struct AppState {
    SDL_Window* window;
    SDL_Renderer* renderer;

    // Audio stream
    SDL_AudioStream* stream;

    // Main audio data (loaded file)
    float* samples;        // Full audio data in Float32 format
    Uint32 samples_count;  // Total number of samples
    int sample_rate;
    int channels;

    // Playback state
    Uint32 cur_sample_idx;  // Current playback position in samples

    // Ring buffer for visualization and audio processing
    float* ring_buffer;
    Uint32 ring_buffer_len;
    Uint32 ring_buffer_idx;  // Current write position in ring buffer
} AppState;

#endif  // #ifndef APP_STATE_H