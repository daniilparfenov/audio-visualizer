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
