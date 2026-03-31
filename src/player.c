#include "player.h"

void Player_Init(AppState* state) {
    if (!state)
        return;

    state->player.is_playing = 1;
}

void Player_TogglePause(AppState* state) {
    if (!state || !state->stream)
        return;

    if (state->player.is_playing) {
        SDL_PauseAudioStreamDevice(state->stream);
        state->player.is_playing = 0;
        SDL_Log("Player: Paused");
    } else {
        SDL_ResumeAudioStreamDevice(state->stream);
        state->player.is_playing = 1;
        SDL_Log("Player: Resumed");
    }
}

void Player_SeekSeconds(AppState* state, float seconds) {
    if (!state || !state->stream || state->sample_rate == 0 || state->channels == 0)
        return;

    // Converting the offset from seconds to the number of samples
    int sample_offset = (int)(seconds * state->sample_rate * state->channels);

    // Align not to break the stereo channels
    sample_offset = (sample_offset / state->channels) * state->channels;

    // Calculate target sample idx
    int64_t target_idx = (int64_t)state->cur_sample_idx + (int64_t)sample_offset;
    if (target_idx < 0) {
        target_idx = 0;
    } else if (target_idx >= state->samples_count) {
        target_idx = 0;
    }

    // Update cur sample to play
    state->cur_sample_idx = (Uint32)target_idx;

    // Clear buffers to avoid sound and visualization aliasing
    SDL_ClearAudioStream(state->stream);
    SDL_memset(state->ring_buffer, 0, state->ring_buffer_len * sizeof(float));
    state->ring_buffer_idx = 0;
    SDL_memset(&state->vis_ctx, 0, sizeof(VisContext));

    SDL_Log("Player: Seek by %.1f sec. New idx: %u", seconds, state->cur_sample_idx);
}
