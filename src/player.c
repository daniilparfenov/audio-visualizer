#include "player.h"
#include "audio_utils.h"

#include <stdio.h>

void Player_Init(AppState* state) {
    if (!state)
        return;

    state->player.is_playing = 0;
    state->player.is_looping = 0;
    state->player.playlist_count = 0;
    state->player.current_song_idx = -1;
}

void Player_TogglePause(AppState* state) {
    if (!state || !state->track)
        return;

    if (state->player.is_playing) {
        MIX_PauseTrack(state->track);
        state->player.is_playing = 0;
        SDL_Log("Player: Paused");
    } else {
        MIX_ResumeTrack(state->track);
        state->player.is_playing = 1;
        SDL_Log("Player: Resumed");
    }
}

void Player_SeekSeconds(AppState* state, float seconds) {
    if (!state || !state->track || !state->audio)
        return;

    // Get current position in frames
    Sint64 current_pos = MIX_GetTrackPlaybackPosition(state->track);
    if (current_pos < 0) {
        SDL_Log("Player: Cannot get track position: %s", SDL_GetError());
        return;
    }

    // Converting the offset from seconds to the number of frames
    Sint64 offset_frames = (Sint64)(seconds * (float)state->sample_rate);
    Sint64 new_pos = current_pos + offset_frames;

    // Protection against negative time
    if (new_pos < 0) {
        new_pos = 0;
    }

    // Checking if we reached the end of the file
    Sint64 total_frames = MIX_GetAudioDuration(state->audio);
    if (total_frames > 0 && new_pos >= total_frames) {
        if (state->player.is_looping) {
            // Loop back to the beginning
            new_pos = 0;
        } else {
            // Stop playback if we seeked past the end
            state->player.is_playing = 0;
            MIX_StopTrack(state->track, 0);
            SDL_Log("Player: Seek reached end of file, playback stopped.");
            return;
        }
    }

    // Set new position for the mixer track
    if (!MIX_SetTrackPlaybackPosition(state->track, new_pos)) {
        SDL_Log("Player: Failed to seek track: %s", SDL_GetError());
        return;
    }

    SDL_Log("Player: Seek by %.1f sec. New frame pos: %lld", seconds, (long long)new_pos);
}

void Player_ToggleLoop(AppState* state) {
    if (!state)
        return;

    state->player.is_looping = !state->player.is_looping;
    SDL_Log("Player: Looping is now %s", state->player.is_looping ? "ON" : "OFF");
}

SDL_AppResult Player_LoadSongIdx(AppState* state, int idx) {
    if (!state || idx < 0 || idx >= state->player.playlist_count)
        return SDL_APP_FAILURE;

    // Pause track
    if (state->track) {
        MIX_StopTrack(state->track, 0);
    }
    SDL_memset(&state->vis_ctx, 0, sizeof(VisContext));

    // Load new song
    const char* new_song_path = state->player.playlist[idx];
    if (Audio_LoadAndSetup(state, new_song_path) != SDL_APP_CONTINUE) {
        SDL_Log("Player: Failed to load next song: %s", new_song_path);
        return SDL_APP_FAILURE;
    }
    state->player.current_song_idx = idx;

    // Pause the song if the previous one was paused
    if (!state->player.is_playing && state->track) {
        MIX_PauseTrack(state->track);
    }

    SDL_Log("Player: Now playing [%d]: %s", idx, new_song_path);
    return SDL_APP_CONTINUE;
}

void Player_NextSong(AppState* state) {
    if (!state || state->player.playlist_count <= 1)
        return;

    int next_idx = (state->player.current_song_idx + 1) % state->player.playlist_count;
    Player_LoadSongIdx(state, next_idx);
}

void Player_PrevSong(AppState* state) {
    if (!state || state->player.playlist_count <= 1)
        return;

    int prev_idx = (state->player.current_song_idx - 1 + state->player.playlist_count) % state->player.playlist_count;
    Player_LoadSongIdx(state, prev_idx);
}

void Player_RemoveSongIdx(AppState* state, int idx) {
    if (!state || idx < 0 || idx >= state->player.playlist_count)
        return;

    // In case of deleting of current playing song
    if (idx == state->player.current_song_idx) {
        state->player.is_playing = 0;
        if (state->track) {
            MIX_StopTrack(state->track, 0);
        }
        state->player.current_song_idx = -1;
    }
    // Free memory for the song filepath
    SDL_free((void*)state->player.playlist[idx]);

    // Moving the remaining tracks to the left
    for (int i = idx; i < state->player.playlist_count - 1; ++i) {
        state->player.playlist[i] = state->player.playlist[i + 1];
    }
    state->player.playlist_count--;

    // Adjusting the index of the current song if it has moved
    if (state->player.current_song_idx > idx) {
        state->player.current_song_idx--;
    }
}

