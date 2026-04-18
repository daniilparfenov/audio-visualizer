#include "app_state.h"
#include "player.h"

#include <stdio.h>
#include <string.h>

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

static void SaveVisSettings(AppState* state) {
    char* pref_path = SDL_GetPrefPath("HSE_CW", "MusicVisualizer");
    if (!pref_path)
        return;

    char file_path[1024];
    SDL_snprintf(file_path, sizeof(file_path), "%svis_settings.txt", pref_path);
    SDL_free(pref_path);

    FILE* f = fopen(file_path, "w");
    if (!f)
        return;

    // Vis mode (Waveform, Spectrum or Both)
    fprintf(f, "%d\n", state->vis_mode);

    // Wave config
    fprintf(f, "%f %f %f %f\n", state->vis_settings.wave_color.r, state->vis_settings.wave_color.g,
            state->vis_settings.wave_color.b, state->vis_settings.wave_color.a);
    fprintf(f, "%f %f %f\n", state->vis_settings.wave_smoothing, state->vis_settings.wave_thickness,
            state->vis_settings.wave_amplitude);

    // Spectrum config
    fprintf(f, "%f %f %f %f\n", state->vis_settings.spectrum_color.r, state->vis_settings.spectrum_color.g,
            state->vis_settings.spectrum_color.b, state->vis_settings.spectrum_color.a);
    fprintf(f, "%f %f %f\n", state->vis_settings.spectrum_smooth_up, state->vis_settings.spectrum_smooth_down,
            state->vis_settings.spectrum_amplitude);

    // Background config
    fprintf(f, "%f %f %f %f\n", state->vis_settings.bg_color.r, state->vis_settings.bg_color.g,
            state->vis_settings.bg_color.b, state->vis_settings.bg_color.a);

    fclose(f);
    SDL_Log("AppState: Visualizer settings saved to %s", file_path);
}

static void LoadVisSettings(AppState* state) {
    // Set up default values for visualization
    state->vis_mode = VISUALIZER_MODE_WAVEFORM;
    // Wave
    state->vis_settings.wave_color = (struct nk_colorf){0.0f, 1.0f, 0.0f, 1.0f};  // Green
    state->vis_settings.wave_smoothing = 0.15f;
    state->vis_settings.wave_thickness = 1.0f;
    state->vis_settings.wave_amplitude = 1.0f;

    // Spectrum
    state->vis_settings.spectrum_color = (struct nk_colorf){0.0f, 0.78f, 1.0f, 1.0f};  // Cyan
    state->vis_settings.spectrum_smooth_up = 0.8f;
    state->vis_settings.spectrum_smooth_down = 0.1f;
    state->vis_settings.spectrum_amplitude = 1.0f;

    // Background
    state->vis_settings.bg_color = (struct nk_colorf){0.0f, 0.0f, 0.0f, 1.0f};  // Black

    // Try to load saved settings (use the default ones in case of errors)
    char* pref_path = SDL_GetPrefPath("HSE_CW", "MusicVisualizer");
    if (!pref_path) {
        SDL_Log("AppState: Could not get pref path to load visualizer settings. Using defaults.");
        return;
    }

    char file_path[1024];
    SDL_snprintf(file_path, sizeof(file_path), "%svis_settings.txt", pref_path);
    SDL_free(pref_path);

    FILE* f = fopen(file_path, "r");
    if (!f) {
        SDL_Log("AppState: No saved visualizer settings found at %s. Using defaults.", file_path);
        return;
    }

    // Parse saved settings
    int mode = 0;
    if (fscanf(f, "%d\n", &mode) == 1) {
        state->vis_mode = (VisualizerMode)mode;
    }

    // Wave config
    fscanf(f, "%f %f %f %f\n", &state->vis_settings.wave_color.r, &state->vis_settings.wave_color.g,
           &state->vis_settings.wave_color.b, &state->vis_settings.wave_color.a);
    fscanf(f, "%f %f %f\n", &state->vis_settings.wave_smoothing, &state->vis_settings.wave_thickness,
           &state->vis_settings.wave_amplitude);

    // Spectrum config
    fscanf(f, "%f %f %f %f\n", &state->vis_settings.spectrum_color.r, &state->vis_settings.spectrum_color.g,
           &state->vis_settings.spectrum_color.b, &state->vis_settings.spectrum_color.a);
    fscanf(f, "%f %f %f\n", &state->vis_settings.spectrum_smooth_up, &state->vis_settings.spectrum_smooth_down,
           &state->vis_settings.spectrum_amplitude);

    // Background config
    fscanf(f, "%f %f %f %f\n", &state->vis_settings.bg_color.r, &state->vis_settings.bg_color.g,
           &state->vis_settings.bg_color.b, &state->vis_settings.bg_color.a);

    fclose(f);
    SDL_Log("AppState: Visualizer settings loaded successfully from %s", file_path);
}

void AppState_Save(AppState* state) {
    if (!state)
        return;
    Player_SaveState(state);
    SaveVisSettings(state);
}

void AppState_Load(AppState* state) {
    if (!state)
        return;
    Player_LoadState(state);
    LoadVisSettings(state);
}
