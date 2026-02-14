#include "visualizer.h"

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

    // Pre-calculate first point to draw
    float prev_x, prev_y;
    prev_x = 0;
    {
        float sample = state->ring_buffer[trigger_idx];
        prev_y = center_y - (int)(sample * amplitude);
    }

    // Draw the samples starting from the trigger to avoid jitter
    for (int i = 0; i < samples_to_draw; i++) {
        int sample_idx = (trigger_idx + i) % state->ring_buffer_len;

        float sample = state->ring_buffer[sample_idx];

        float x = i;
        float y = center_y - (int)(sample * amplitude);

        SDL_RenderLine(state->renderer, prev_x, prev_y, x, y);

        prev_x = x;
        prev_y = y;
    }
}
