#include "player.h"

void Player_Init(AppState* state) {
    if (!state)
        return;

    state->player.is_playing = 1;
    state->player.is_looping = 1;
}

void Player_TogglePause(AppState* state) {
    if (!state || !state->stream)
        return;

    if (state->player.is_playing) {
        SDL_PauseAudioStreamDevice(state->stream);
        state->player.is_playing = 0;
        SDL_Log("Player: Paused");
    } else {
        // If track has finished playing and paused, we start playing it again
        if (state->cur_sample_idx >= state->samples_count) {
            state->cur_sample_idx = 0;

            // Clear buffers
            SDL_ClearAudioStream(state->stream);
            SDL_memset(state->ring_buffer, 0, state->ring_buffer_len * sizeof(float));
            state->ring_buffer_idx = 0;
            SDL_memset(&state->vis_ctx, 0, sizeof(VisContext));
        }

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
        if (state->player.is_looping) {
            target_idx = 0;
        } else {
            target_idx = state->samples_count;
            state->player.is_playing = 0;
            SDL_PauseAudioStreamDevice(state->stream);
            SDL_Log("Player: Seek reached end of file, playback stopped.");
        }
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

void Player_ToggleLoop(AppState* state) {
    if (!state)
        return;

    state->player.is_looping = !state->player.is_looping;
    SDL_Log("Player: Looping is now %s", state->player.is_looping ? "ON" : "OFF");
}
