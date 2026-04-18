#define NK_IMPLEMENTATION
#define NK_SDL3_RENDERER_IMPLEMENTATION

#include "gui.h"
#include "app_state.h"
#include "player.h"

// Callback for processing files from the SDL3 dialog box
static void SDLCALL OnFileSelected(void* userdata, const char* const* filelist, int filter) {
    if (!userdata || !filelist)
        return;

    AppState* state = (AppState*)userdata;

    for (int i = 0; filelist[i] != NULL; i++) {
        if (state->player.playlist_count < MAX_PLAYLIST_SONGS) {
            // Copying the path to the song to the playlist
            state->player.playlist[state->player.playlist_count] = SDL_strdup(filelist[i]);
            state->player.playlist_count++;
            SDL_Log("Added to playlist: %s", filelist[i]);
        } else {
            SDL_Log("Playlist is full! Maximum is %d", MAX_PLAYLIST_SONGS);
            break;
        }
    }
}

// Returns the file name to display
static const char* GetFileName(const char* filepath) {
    if (!filepath)
        return "Unknown";

    const char* name = SDL_strrchr(filepath, '/');
    const char* name_win = SDL_strrchr(filepath, '\\');

    // We select the last slash (from left to right), taking into account the systems \ and /
    if (name_win > name)
        name = name_win;

    if (name) {
        return name + 1;  // Skip the slash itself
    }

    // If there are no slashes, we return the entire string
    return filepath;
}

static void GUI_DrawPlaylist(AppState* state, float width, float height) {
    struct nk_rect playlist_bounds = nk_rect(0, 0, width, height);

    // Set the semi-transparent background of the window
    nk_style_push_style_item(state->nk_ctx, &state->nk_ctx->style.window.fixed_background,
                             nk_style_item_color(nk_rgba(30, 30, 30, 150)));
    if (nk_begin(state->nk_ctx, "Playlist", playlist_bounds,
                 NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_BACKGROUND)) {

        nk_layout_row_dynamic(state->nk_ctx, 20, 1);
        nk_label(state->nk_ctx, "Tracks:", NK_TEXT_LEFT);

        // The button for adding a song
        nk_layout_row_dynamic(state->nk_ctx, 30, 1);
        if (nk_button_label(state->nk_ctx, "+ Add Track")) {
            // File filters
            static const SDL_DialogFileFilter audio_filters[] = {
                {"Audio files", "wav"},  //
                {"All files", "*"}       //
            };
            SDL_ShowOpenFileDialog(OnFileSelected, state, NULL, audio_filters, 2, NULL, true);
        }

        // Track List
        for (int i = 0; i < state->player.playlist_count; ++i) {
            const char* full_path = state->player.playlist[i];
            const char* file_name = GetFileName(full_path);

            // Сhecking if this track is playing now
            int is_active = (i == state->player.current_song_idx);

            // If the track is currently playing, add a marker to it
            char display_name[256];
            if (is_active) {
                snprintf(display_name, sizeof(display_name), ">> %s", file_name);

                // Changing the text color of the active track
                nk_style_push_color(state->nk_ctx, &state->nk_ctx->style.button.text_normal, nk_rgb(100, 255, 100));
                nk_style_push_color(state->nk_ctx, &state->nk_ctx->style.button.text_hover, nk_rgb(150, 255, 150));
            } else {
                snprintf(display_name, sizeof(display_name), "%s", file_name);
            }

            // Draw the track button and delete button for it
            // create a row with two columns: a dynamic part (name) and a fixed one (button [X])
            nk_layout_row_begin(state->nk_ctx, NK_DYNAMIC, 30, 2);
            nk_layout_row_push(state->nk_ctx, 0.8f);  // 80% of the width for track name

            // A click starts this particular track
            if (nk_button_label(state->nk_ctx, display_name)) {
                Player_LoadSongIdx(state, i);

                // Unpause if the track was paused
                if (!state->player.is_playing) {
                    Player_TogglePause(state);
                }
            }

            // Clear color style if it was active track
            // to avoid coloring the following buttons
            if (is_active) {
                nk_style_pop_color(state->nk_ctx);
                nk_style_pop_color(state->nk_ctx);
            }

            nk_layout_row_push(state->nk_ctx, 0.2f);  // 20% of the width for the delete button
            if (nk_button_label(state->nk_ctx, "[X]")) {
                Player_RemoveSongIdx(state, i);
                // Interrupting rendering to avoid a crash due to array modification during iteration
                nk_layout_row_end(state->nk_ctx);
                break;
            }
            nk_layout_row_end(state->nk_ctx);
        }

        // If the playlist is empty, show a hint
        if (state->player.playlist_count == 0) {
            nk_layout_row_dynamic(state->nk_ctx, 20, 1);
            nk_label(state->nk_ctx, "Playlist is empty", NK_TEXT_CENTERED);
        }
    }
    nk_end(state->nk_ctx);

    // Pop style for semi-transparent window
    nk_style_pop_style_item(state->nk_ctx);
}

static void GUI_DrawBottomPanel(AppState* state, float window_w, float panel_height, float vis_height) {
    struct nk_rect panel_bounds = nk_rect(0, vis_height, window_w, panel_height);

    // Remove default inner paddings
    nk_style_push_vec2(state->nk_ctx, &state->nk_ctx->style.window.padding, nk_vec2(0, 0));

    if (nk_begin(state->nk_ctx, "BottomPanel", panel_bounds, NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {

        // Button size settings
        const float btn_w = 70.0f;    // Width of toggle buttons
        const float play_w = 90.0f;   // Play button will be slightly wider
        const float btn_h = 35.0f;    // Height of all buttons
        const float spacing = 10.0f;  // Indentation between buttons in the group

        // Calculating the Y position for the vertical centering of the buttons on the panel
        const float center_y = (panel_height - btn_h) / 2.0f;

        // Calculating the total width of the central group (4 regular buttons + 1 Play + 4 margins)
        const float group_width = (btn_w * 4) + play_w + (spacing * 4);

        // the X-coordinate for the entire group (it is centered in the window)
        const float start_x = ((float)window_w - group_width) / 2.0f;

        // Enabling a layer with free widget positioning (7 buttons)
        nk_layout_space_begin(state->nk_ctx, NK_STATIC, panel_height, 8);

        // --- Button 0. show/hide playlist ---
        nk_layout_space_push(state->nk_ctx, nk_rect(20.0f, center_y, 110.0f, btn_h));
        if (nk_button_label(state->nk_ctx, "Playlist")) {
            state->show_playlist = !state->show_playlist;
        }

        // --- Button 1. Switching the visualization mode (Located at the left edge) ---
        nk_layout_space_push(state->nk_ctx, nk_rect(140.0f, center_y, 110.0f, btn_h));
        const char* vis_label = "Vis: Wave";
        if (state->vis_mode == VISUALIZER_MODE_SPECTRUM)
            vis_label = "Vis: Spectrum";
        else if (state->vis_mode == VISUALIZER_MODE_BOTH)
            vis_label = "Vis: Both";
        if (nk_button_label(state->nk_ctx, vis_label)) {
            state->vis_mode = (state->vis_mode + 1) % VISUALIZER_MODE_COUNT;
        }

        // --- Button 2. Previous song ---
        nk_layout_space_push(state->nk_ctx, nk_rect(start_x, center_y, btn_w, btn_h));
        if (nk_button_label(state->nk_ctx, "|< Prev")) {
            Player_PrevSong(state);
        }

        // --- Button 3. -5 Seconds ---
        nk_layout_space_push(state->nk_ctx, nk_rect(start_x + btn_w + spacing, center_y, btn_w, btn_h));
        if (nk_button_label(state->nk_ctx, "-5s")) {
            Player_SeekSeconds(state, -5.0f);
        }

        // --- Button 4. Play/Pause button (centered) ---
        nk_layout_space_push(state->nk_ctx, nk_rect(start_x + (btn_w + spacing) * 2, center_y, play_w, btn_h));
        const char* play_label = !state->player.is_playing ? "Play" : "Pause";
        if (nk_button_label(state->nk_ctx, play_label)) {
            Player_TogglePause(state);
        }

        // --- Button 5. +5 Seconds ---
        nk_layout_space_push(state->nk_ctx,
                             nk_rect(start_x + (btn_w + spacing) * 2 + play_w + spacing, center_y, btn_w, btn_h));
        if (nk_button_label(state->nk_ctx, "+5s")) {
            Player_SeekSeconds(state, 5.0f);
        }

        // --- Button 6. Next song ---
        nk_layout_space_push(state->nk_ctx,
                             nk_rect(start_x + (btn_w + spacing) * 3 + play_w + spacing, center_y, btn_w, btn_h));
        if (nk_button_label(state->nk_ctx, "Next >|")) {
            Player_NextSong(state);
        }

        // --- Button 7. Loop ON/OFF button (Located at the right edge) ---
        const float loop_btn_w = 90.0f;
        nk_layout_space_push(state->nk_ctx, nk_rect((float)window_w - loop_btn_w - 20.0f, center_y, loop_btn_w, btn_h));
        const char* loop_label = state->player.is_looping ? "Loop: ON" : "Loop: OFF";
        if (nk_button_label(state->nk_ctx, loop_label)) {
            Player_ToggleLoop(state);
        }

        // Disabling a layer with free widget positioning (6 buttons)
        nk_layout_space_end(state->nk_ctx);
    }
    nk_end(state->nk_ctx);
    nk_style_pop_vec2(state->nk_ctx);
}

void GUI_Draw(AppState* state, float* out_vis_x, float* out_vis_y, float* out_vis_w, float* out_vis_h) {
    int window_w, window_h;
    SDL_GetRenderOutputSize(state->renderer, &window_w, &window_h);

    // Default GUI and visualization size
    const float panel_height = 60.0f;
    const float playlist_width = 250.0f;
    float vis_height = (float)window_h - panel_height;

    // The base coordinates of the visualization (the entire screen except the bottom panel)
    *out_vis_x = 0;
    *out_vis_y = 0;
    *out_vis_w = (float)window_w;
    *out_vis_h = vis_height;

    // Drawing a tracklist (if it is open)
    if (state->show_playlist) {
        GUI_DrawPlaylist(state, playlist_width, vis_height);
    }

    // Drawing the bottom panel
    GUI_DrawBottomPanel(state, (float)window_w, panel_height, vis_height);
}
