#include "visualizer.h"

#include <math.h>
#include <string.h>
#include "fft.h"  // Own library with FFT implementation

#include <SDL3/SDL.h>

static int FindTriggerPoint(AppState* state, int search_start_idx, int search_len) {
    for (int i = 0; i < search_len; i++) {
        int cur_idx = (search_start_idx + i) % state->ring_buffer_len;
        int next_idx = (search_start_idx + i + 1) % state->ring_buffer_len;

        float v1 = state->ring_buffer[cur_idx];
        float v2 = state->ring_buffer[next_idx];

        // Rising Edge
        if (v1 <= 0.0f && v2 > 0) {
            return cur_idx;
        }
    }

    return search_start_idx;
}

void DrawWaveform(AppState* state) {
    int window_w, window_h;
    if (!SDL_GetRenderOutputSize(state->renderer, &window_w, &window_h))
        return;

    // The color of wave - Green
    SDL_SetRenderDrawColor(state->renderer, 0, 255, 0, 255);

    // Wave position and amplitude
    int center_y = window_h / 2;
    float amplitude = window_h / 2.5f;

    // The number of samples to draw
    int samples_to_draw = SDL_min(window_w, state->ring_buffer_len);

    // Find a trigger
    int search_range = samples_to_draw / 2;
    int search_start_idx = (state->ring_buffer_idx + state->ring_buffer_len - (samples_to_draw + search_range)) % state->ring_buffer_len;
    int trigger_idx = FindTriggerPoint(state, search_start_idx, samples_to_draw / 2);

    // Samples buffer to display
    static float display_buffer[8192] = {0};
    if (samples_to_draw > 8192)
        samples_to_draw = 8192;

    // Linear interpolation
    float smooth_factor = 0.15f;
    for (int i = 0; i < samples_to_draw; i++) {
        int sample_idx = (trigger_idx + i) % state->ring_buffer_len;
        float target_sample = state->ring_buffer[sample_idx];

        display_buffer[i] += (target_sample - display_buffer[i]) * smooth_factor;
    }

    // Pre-calculate first point to draw using smoothed buffer
    float prev_x, prev_y;
    prev_x = 0;
    prev_y = center_y - (int)(display_buffer[0] * amplitude);

    // Draw the samples starting from the trigger to avoid jitter
    for (int i = 0; i < samples_to_draw; i++) {
        float sample = display_buffer[i];

        float x = i;
        float y = center_y - (int)(sample * amplitude);

        SDL_RenderLine(state->renderer, prev_x, prev_y, x, y);

        prev_x = x;
        prev_y = y;
    }
}

#define FFT_SIZE 1024

void DrawSpectrum(AppState* state) {
    int window_w, window_h;
    if (!SDL_GetRenderOutputSize(state->renderer, &window_w, &window_h))
        return;

    // Fill FFT buffer with last FFT_SIZE samples from ring buffer
    static float audio_frame[FFT_SIZE] = {0};
    int start_idx = (state->ring_buffer_idx + state->ring_buffer_len - FFT_SIZE) % state->ring_buffer_len;
    for (int i = 0; i < FFT_SIZE; i++) {
        int idx = (start_idx + i) % state->ring_buffer_len;
        audio_frame[i] = state->ring_buffer[idx];
    }

    // Apply the hamming window
    apply_hamming_window(audio_frame, FFT_SIZE);

    // Get freq magnitudes
    int num_bins = FFT_SIZE / 2;
    static float magnitudes[FFT_SIZE / 2] = {0};
    rfft_magnitudes(audio_frame, magnitudes, FFT_SIZE);

    // Normalization and smoothing the amplitudes
    static float smoothed_magnitudes[FFT_SIZE / 2] = {0};
    const float smooth_up_factor = 0.8f;
    const float smooth_down_factor = 0.1f;
    const float min_db = -60.0f;
    const float max_db = 0.0f;
    const float db_range = max_db - min_db;
    for (int i = 0; i < num_bins; i++) {
        float normalized_mag = magnitudes[i] / (FFT_SIZE / 2.0f);
        float db = 20.0f * log10f(normalized_mag + 1e-6f);

        float target_value = (db - min_db) / db_range;
        if (target_value < 0.0f)
            target_value = 0.0f;
        if (target_value > 1.0f)
            target_value = 1.0f;

        // Interpolation
        if (target_value > smoothed_magnitudes[i]) {
            smoothed_magnitudes[i] += (target_value - smoothed_magnitudes[i]) * smooth_up_factor;
        } else {
            smoothed_magnitudes[i] += (target_value - smoothed_magnitudes[i]) * smooth_down_factor;
        }
    }

    // Rendering configuration
    int start_bin = 1;
    int end_bin = num_bins / 2;  // We will not draw the highest frequencies, because there is usually no music
    int display_bins = end_bin - start_bin;
    float bar_width = (float)window_w / (float)display_bins;
    float max_bar_height = (float)window_h / 2.0f;
    SDL_SetRenderDrawColor(state->renderer, 0, 200, 255, 255);

    for (int i = start_bin; i < end_bin; i++) {
        const float normalized_mag = smoothed_magnitudes[i];

        const float bar_height = normalized_mag * max_bar_height;

        SDL_FRect bar_rect;
        bar_rect.x = i * bar_width;
        bar_rect.y = (window_h / 2) - bar_height;
        bar_rect.w = bar_width > 1.0f ? bar_width - 1.0f : 1.0f;  // 1px gap between bars
        bar_rect.h = bar_height;

        SDL_RenderFillRect(state->renderer, &bar_rect);
    }
}