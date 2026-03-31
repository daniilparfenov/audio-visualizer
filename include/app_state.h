#ifndef APP_STATE_H
#define APP_STATE_H

#include <SDL3/SDL.h>

typedef enum {
    VISUALIZER_MODE_WAVEFORM = 0,  //
    VISUALIZER_MODE_SPECTRUM = 1,  //
    VISUALIZER_MODE_BOTH = 2,      //
    VISUALIZER_MODE_COUNT          //
} VisualizerMode;

typedef struct AppConfig {
    const char* window_title;
    int window_w;
    int window_h;
    int vsync;
    const char* audio_filepath;
} AppConfig;

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

    // Visualizer Mode
    VisualizerMode vis_mode;

    // App configuration
    AppConfig config;
} AppState;

#endif  // #ifndef APP_STATE_H