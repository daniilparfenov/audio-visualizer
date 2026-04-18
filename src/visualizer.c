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

void DrawWaveform(AppState* state, const SDL_FRect* canvas) {
    if (!canvas || !state->ring_buffer || state->ring_buffer_len == 0)
        return;

    // Enabling clipping to avoid drawing outside the canvas
    SDL_Rect clip_rect = {(int)canvas->x, (int)canvas->y, (int)canvas->w, (int)canvas->h};
    SDL_SetRenderClipRect(state->renderer, &clip_rect);

    // The color of wave
    SDL_SetRenderDrawColor(state->renderer, (Uint8)(state->vis_settings.wave_color.r * 255.0f),
                           (Uint8)(state->vis_settings.wave_color.g * 255.0f),
                           (Uint8)(state->vis_settings.wave_color.b * 255.0f),
                           (Uint8)(state->vis_settings.wave_color.a * 255.0f));

    // Wave position and amplitude
    float center_y = canvas->y + (canvas->h / 2.0f);
    float amplitude = (canvas->h / 2.5f) * state->vis_settings.wave_amplitude;

    // The number of samples to draw
    int samples_to_draw = SDL_min((int)canvas->w, state->ring_buffer_len);

    // Find a trigger
    int search_range = samples_to_draw / 2;
    int search_start_idx =
        (state->ring_buffer_idx + state->ring_buffer_len - (samples_to_draw + search_range)) % state->ring_buffer_len;
    int trigger_idx = FindTriggerPoint(state, search_start_idx, samples_to_draw / 2);

    // Linear interpolation
    float smooth_factor = state->vis_settings.wave_smoothing;
    float* wave_smoothed = state->vis_ctx.wave_smoothed;
    for (int i = 0; i < samples_to_draw; i++) {
        int sample_idx = (trigger_idx + i) % state->ring_buffer_len;
        float target_sample = state->ring_buffer[sample_idx];

        wave_smoothed[i] += (target_sample - wave_smoothed[i]) * smooth_factor;
    }

    // Pre-calculate first point to draw using smoothed buffer
    float prev_x, prev_y;
    prev_x = 0;
    prev_y = center_y - (int)(wave_smoothed[0] * amplitude);

    // Get half the thickness of the target line to draw several offset lines
    float half_thick = state->vis_settings.wave_thickness / 2.0f;

    // Draw the samples starting from the trigger to avoid jitter
    for (int i = 0; i < samples_to_draw; i++) {
        float sample = wave_smoothed[i];

        float x = i;
        float y = center_y - (int)(sample * amplitude);

        // Simulate the thickness of the line by drawing several "layers" with a Y offset.F
        for (float t = -half_thick; t <= half_thick; t += 1.0f) {
            SDL_RenderLine(state->renderer, prev_x, prev_y + t, x, y + t);
        }

        prev_x = x;
        prev_y = y;
    }

    // Disabling clipping
    SDL_SetRenderClipRect(state->renderer, NULL);
}

#define FFT_SIZE 1024

void DrawSpectrum(AppState* state, const SDL_FRect* canvas) {
    if (!canvas || !state->ring_buffer || state->ring_buffer_len == 0)
        return;

    // Enabling clipping to avoid drawing outside the canvas
    SDL_Rect clip_rect = {(int)canvas->x, (int)canvas->y, (int)canvas->w, (int)canvas->h};
    SDL_SetRenderClipRect(state->renderer, &clip_rect);

    // Fill FFT buffer with last FFT_SIZE samples from ring buffer
    float audio_frame[FFT_SIZE] = {0};
    int start_idx = (state->ring_buffer_idx + state->ring_buffer_len - FFT_SIZE) % state->ring_buffer_len;
    for (int i = 0; i < FFT_SIZE; i++) {
        int idx = (start_idx + i) % state->ring_buffer_len;
        audio_frame[i] = state->ring_buffer[idx];
    }

    // Apply the hamming window
    apply_hamming_window(audio_frame, FFT_SIZE);

    // Get freq magnitudes
    int num_bins = FFT_SIZE / 2;
    float magnitudes[FFT_SIZE / 2] = {0};
    rfft_magnitudes(audio_frame, magnitudes, FFT_SIZE);

    // Normalization and smoothing the amplitudes
    const float smooth_up_factor = state->vis_settings.spectrum_smooth_up;
    const float smooth_down_factor = state->vis_settings.spectrum_smooth_down;
    const float min_db = -60.0f;
    const float max_db = 0.0f;
    const float db_range = max_db - min_db;
    float* pSpectrum_smoothed = state->vis_ctx.spectrum_smoothed;
    for (int i = 0; i < num_bins; i++) {
        float normalized_mag = magnitudes[i] / (FFT_SIZE / 2.0f);
        float db = 20.0f * log10f(normalized_mag + 1e-6f);

        float target_value = (db - min_db) / db_range;
        if (target_value < 0.0f)
            target_value = 0.0f;
        if (target_value > 1.0f)
            target_value = 1.0f;

        // Interpolation
        if (target_value > pSpectrum_smoothed[i]) {
            pSpectrum_smoothed[i] += (target_value - pSpectrum_smoothed[i]) * smooth_up_factor;
        } else {
            pSpectrum_smoothed[i] += (target_value - pSpectrum_smoothed[i]) * smooth_down_factor;
        }
    }

    // Rendering configuration
    int start_bin = 1;
    int end_bin = num_bins / 2;  // We will not draw the highest frequencies, because there is usually no music
    int display_bins = end_bin - start_bin;
    float bar_width = (float)canvas->w / (float)display_bins;
    float max_bar_height = ((float)canvas->h / 2.0f) * state->vis_settings.spectrum_amplitude;
    SDL_SetRenderDrawColor(state->renderer, (Uint8)(state->vis_settings.spectrum_color.r * 255.0f),
                           (Uint8)(state->vis_settings.spectrum_color.g * 255.0f),
                           (Uint8)(state->vis_settings.spectrum_color.b * 255.0f),
                           (Uint8)(state->vis_settings.spectrum_color.a * 255.0f));

    for (int i = start_bin; i < end_bin; i++) {
        const float normalized_mag = pSpectrum_smoothed[i];

        const float bar_height = normalized_mag * max_bar_height;

        SDL_FRect bar_rect;
        bar_rect.x = canvas->x + (i * bar_width);
        bar_rect.y = (canvas->y + canvas->h) - bar_height;
        bar_rect.w = bar_width > 1.0f ? bar_width - 1.0f : 1.0f;  // 1px gap between bars
        bar_rect.h = bar_height;

        SDL_RenderFillRect(state->renderer, &bar_rect);
    }

    // Disabling clipping
    SDL_SetRenderClipRect(state->renderer, NULL);
}
