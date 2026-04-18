#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "gui.h"

#include "app_state.h"
#include "audio_utils.h"
#include "player.h"
#include "visualizer.h"
#include "window_utils.h"

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    AppState* state = SDL_calloc(1, sizeof(AppState));
    if (!state) {
        return SDL_APP_FAILURE;
    }
    *appstate = state;

    // Default app configuration
    state->config.window_title = "Music Visualizer";
    state->config.window_w = 1280;
    state->config.window_h = 720;
    state->config.vsync = 1;

    // Initialization of SDL subsystems
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Init window
    if (Window_Init(state) != SDL_APP_CONTINUE) {
        return SDL_APP_FAILURE;
    }

    // Init Audio Mixer
    if (Audio_Init(state) != SDL_APP_CONTINUE) {
        return SDL_APP_FAILURE;
    }

    // Init player
    Player_Init(state);

    // Loading saved settings
    AppState_Load(state);

    // Nuklear initialization
    state->nk_ctx = nk_sdl_init(state->window, state->renderer, nk_sdl_allocator());

    // Loading standard font
    struct nk_font_atlas* atlas = nk_sdl_font_stash_begin(state->nk_ctx);
    nk_sdl_font_stash_end(state->nk_ctx);

    nk_input_begin(state->nk_ctx);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    AppState* state = (AppState*)appstate;

    // Nuklear event handler
    nk_sdl_handle_event(state->nk_ctx, event);

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_ESCAPE) {
            return SDL_APP_SUCCESS;
        } else if (event->key.key == SDLK_UP) {
            state->vis_mode = (state->vis_mode + 1) % VISUALIZER_MODE_COUNT;
            SDL_Log("Switched visualizer mode to: %d", state->vis_mode);
        } else if (event->key.key == SDLK_DOWN) {
            state->vis_mode = (state->vis_mode - 1 + VISUALIZER_MODE_COUNT) % VISUALIZER_MODE_COUNT;
            SDL_Log("Switched visualizer mode to: %d", state->vis_mode);
        } else if (event->key.key == SDLK_SPACE) {
            Player_TogglePause(state);
        } else if (event->key.key == SDLK_LEFT) {
            Player_SeekSeconds(state, -5.0f);
        } else if (event->key.key == SDLK_RIGHT) {
            Player_SeekSeconds(state, 5.0f);
        } else if (event->key.key == SDLK_L) {
            Player_ToggleLoop(state);
        } else if (event->key.key == SDLK_RIGHTBRACKET) {
            Player_NextSong(state);
        } else if (event->key.key == SDLK_LEFTBRACKET) {
            Player_PrevSong(state);
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    AppState* state = (AppState*)appstate;

    // Close the Nuklear input collection to process current frame
    nk_input_end(state->nk_ctx);

    // Draw the interface and get an area for the visualizer
    float vis_x, vis_y, vis_w, vis_h;
    GUI_Draw(state, &vis_x, &vis_y, &vis_w, &vis_h);

    // Clear the screen with color from settings
    SDL_SetRenderDrawBlendMode(state->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(state->renderer, (Uint8)(state->vis_settings.bg_color.r * 255),
                           (Uint8)(state->vis_settings.bg_color.g * 255), (Uint8)(state->vis_settings.bg_color.b * 255),
                           255);
    SDL_RenderClear(state->renderer);

    // Auto-switching of songs
    if (state->player.wants_next_song) {
        state->player.wants_next_song = 0;
        Player_NextSong(state);
    }

    // Feed audio from buffer
    FeedAudio(state);

    // Areas for rendering
    SDL_FRect vis_area = {vis_x, vis_y, vis_w, vis_h};
    SDL_FRect top_half = {vis_x, vis_y, vis_w, vis_h / 2.0f};
    SDL_FRect bottom_half = {vis_x, vis_y + vis_h / 2.0f, vis_w, vis_h / 2.0f};

    // Visualization
    switch (state->vis_mode) {
        case VISUALIZER_MODE_WAVEFORM:
            DrawWaveform(state, &vis_area);
            break;
        case VISUALIZER_MODE_SPECTRUM:
            DrawSpectrum(state, &vis_area);
            break;
        case VISUALIZER_MODE_BOTH:
            DrawWaveform(state, &top_half);
            DrawSpectrum(state, &bottom_half);
            break;
        default:
            break;
    }

    // GUI rendering
    nk_sdl_render(state->nk_ctx, NK_ANTI_ALIASING_ON);

    SDL_RenderPresent(state->renderer);

    // Open the Nuklear input collection for the next frame
    nk_input_begin(state->nk_ctx);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    AppState* state = (AppState*)appstate;
    if (state) {
        // Save the whole application state
        AppState_Save(state);

        // free Nuklear state
        if (state->nk_ctx) {
            nk_sdl_shutdown(state->nk_ctx);
        }

        Audio_Cleanup(state);
        Window_Cleanup(state);
        SDL_free(state);
    }
}
