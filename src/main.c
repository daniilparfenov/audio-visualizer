#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

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
    state->config.audio_filepath = "./assets/wav_sample1.wav";

    // Initialization of SDL subsystems
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Init window
    if (Window_Init(state) != SDL_APP_CONTINUE) {
        return SDL_APP_FAILURE;
    }

    // Load and setup audio
    if (Audio_LoadAndSetup(state, state->config.audio_filepath) != SDL_APP_CONTINUE) {
        return SDL_APP_FAILURE;
    }

    // Init player
    Player_Init(state);

    // Setup visualizer mode
    state->vis_mode = VISUALIZER_MODE_WAVEFORM;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    AppState* state = (AppState*)appstate;

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_ESCAPE) {
            return SDL_APP_SUCCESS;
        } else if (event->key.key == SDLK_SPACE) {
            state->vis_mode = (state->vis_mode + 1) % VISUALIZER_MODE_COUNT;
            SDL_Log("Switched visualizer mode to: %d", state->vis_mode);
        } else if (event->key.key == SDLK_P) {
            Player_TogglePause(state);
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    AppState* state = (AppState*)appstate;

    // Clear the screen with black color
    SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
    SDL_RenderClear(state->renderer);

    // Feed audio from buffer
    FeedAudio(state);

    int window_w, window_h;
    SDL_GetRenderOutputSize(state->renderer, &window_w, &window_h);

    // Areas for rendering
    SDL_FRect full_screen = {0, 0, (float)window_w, (float)window_h};
    SDL_FRect top_half = {0, 0, (float)window_w, (float)window_h / 2.0f};
    SDL_FRect bottom_half = {0, (float)window_h / 2.0f, (float)window_w, (float)window_h / 2.0f};

    // Visualization
    switch (state->vis_mode) {
        case VISUALIZER_MODE_WAVEFORM:
            DrawWaveform(state, &full_screen);
            break;
        case VISUALIZER_MODE_SPECTRUM:
            DrawSpectrum(state, &full_screen);
            break;
        case VISUALIZER_MODE_BOTH:
            DrawWaveform(state, &top_half);
            DrawSpectrum(state, &bottom_half);
            break;
        default:
            break;
    }

    SDL_RenderPresent(state->renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    AppState* state = (AppState*)appstate;
    if (state) {
        Audio_Cleanup(state);
        Window_Cleanup(state);
        SDL_free(state);
    }
}
