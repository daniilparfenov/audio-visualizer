#ifndef APP_STATE_H
#define APP_STATE_H

#include <SDL3/SDL.h>

#define RING_BUFFER_SIZE 8192
#define FFT_SIZE 1024
#define MAX_PLAYLIST_SONGS 32

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

typedef struct VisContext {
    float wave_smoothed[RING_BUFFER_SIZE];
    float spectrum_smoothed[FFT_SIZE / 2];
} VisContext;

typedef struct PlayerState {
    int is_playing;
    int is_looping;
    int wants_next_song;

    // Playlist
    const char* playlist[MAX_PLAYLIST_SONGS];  // Array of file paths
    int playlist_count;                        // The number of songs in the playlist
    int current_song_idx;                      // Index of currently playing song
} PlayerState;

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

    // Visualization cache
    VisContext vis_ctx;

    // Player - structure to control audio playback by the user
    PlayerState player;
} AppState;

#endif  // #ifndef APP_STATE_H