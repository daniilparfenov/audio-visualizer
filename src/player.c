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

void Player_SaveState(AppState* state) {
    if (!state)
        return;

    // Use SDL to get the correct standard OS folder for app settings
    char* pref_path = SDL_GetPrefPath("HSE_CW", "MusicVisualizer");
    if (!pref_path) {
        SDL_Log("Player: Could not get pref path: %s", SDL_GetError());
        return;
    }

    // Construct the full file path
    char file_path[1024];
    SDL_snprintf(file_path, sizeof(file_path), "%splayer_state.txt", pref_path);
    SDL_free(pref_path);  // SDL_GetPrefPath allocates memory

    FILE* f = fopen(file_path, "w");
    if (!f) {
        SDL_Log("Player: Could not open %s for writing", file_path);
        return;
    }

    fprintf(f, "%d\n", state->player.is_looping);
    fprintf(f, "%d\n", state->player.current_song_idx);

    for (int i = 0; i < state->player.playlist_count; ++i) {
        fprintf(f, "%s\n", state->player.playlist[i]);
    }

    fclose(f);
    SDL_Log("Player: State saved successfully to %s", file_path);
}

void Player_LoadState(AppState* state) {
    if (!state)
        return;

    // Use SDL to get the correct standard OS folder for app settings
    char* pref_path = SDL_GetPrefPath("HSE_CW", "MusicVisualizer");  // HSE_CW = HSE_COURSE_WORK
    if (!pref_path) {
        return;
    }

    char file_path[1024];
    SDL_snprintf(file_path, sizeof(file_path), "%splayer_state.txt", pref_path);
    SDL_free(pref_path);

    FILE* f = fopen(file_path, "r");
    if (!f) {
        SDL_Log("Player: No saved state found at %s", file_path);
        return;
    }

    int is_looping = 0;
    int current_song_idx = -1;

    if (fscanf(f, "%d\n", &is_looping) != 1) {
        fclose(f);
        return;
    }
    if (fscanf(f, "%d\n", &current_song_idx) != 1) {
        fclose(f);
        return;
    }

    state->player.is_looping = is_looping;

    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        // Remove newline characters (\r or \n) at the end of the line
        size_t len = SDL_strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[len - 1] = '\0';
            len--;
        }

        // Add to playlist
        if (len > 0 && state->player.playlist_count < MAX_PLAYLIST_SONGS) {
            state->player.playlist[state->player.playlist_count] = SDL_strdup(line);
            state->player.playlist_count++;
        }
    }
    fclose(f);

    // If there was a valid active song, load it and pause
    if (current_song_idx >= 0 && current_song_idx < state->player.playlist_count) {
        if (Player_LoadSongIdx(state, current_song_idx) == SDL_APP_CONTINUE) {
            // Start paused on launch
            if (state->player.is_playing) {
                Player_TogglePause(state);
            }
        }
    }

    SDL_Log("Player: State loaded successfully.");
}
