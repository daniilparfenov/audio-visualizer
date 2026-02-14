#include "visualizer.h"

#include <SDL3/SDL.h>

void DrawWaveform(AppState* state) {
    int window_w, window_h;
    if (!SDL_GetRenderOutputSize(state->renderer, &window_w, &window_h))
        return;

    // The color of wave - Green
    SDL_SetRenderDrawColor(state->renderer, 0, 255, 0, 255);

    // Wave position and amplitude
    int center_y = window_h / 2;
    float amplitude = window_h / 2.5f;

    int samples_to_draw = (window_w < state->ring_buffer_len) ? window_w : state->ring_buffer_len;

    // Pre-calculate first point to draw
    int prev_x, prev_y;
    {
        prev_x = 0;
        int sample_idx = (state->ring_buffer_idx + state->ring_buffer_len - samples_to_draw) % state->ring_buffer_len;
        float sample = state->ring_buffer[sample_idx];
        prev_y = center_y - (int)(sample * amplitude);
    }

    // Draw the last "samples_to_draw" samples from the ring buffer
    for (int i = 0; i < samples_to_draw; i++) {
        int offset_from_head = samples_to_draw - i;
        int sample_idx = (state->ring_buffer_idx + state->ring_buffer_len - offset_from_head) % state->ring_buffer_len;

        float sample = state->ring_buffer[sample_idx];

        int x = i;
        int y = center_y - (int)(sample * amplitude);

        SDL_RenderLine(state->renderer, prev_x, prev_y, x, y);

        prev_x = x;
        prev_y = y;
    }
}
