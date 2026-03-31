#include "window_utils.h"

SDL_AppResult Window_Init(AppState* state) {
    if (!state)
        return SDL_APP_FAILURE;

    // Create window and renderer based on configuration
    if (!SDL_CreateWindowAndRenderer(state->config.window_title, state->config.window_w, state->config.window_h,
                                     SDL_WINDOW_RESIZABLE, &state->window, &state->renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Turn on/off VSync
    if (!SDL_SetRenderVSync(state->renderer, state->config.vsync)) {
        SDL_Log("Warning: Couldn't set vsync: %s", SDL_GetError());
    }

    return SDL_APP_CONTINUE;
}

void Window_Cleanup(AppState* state) {
    if (!state)
        return;

    if (state->renderer) {
        SDL_DestroyRenderer(state->renderer);
        state->renderer = NULL;
    }
    if (state->window) {
        SDL_DestroyWindow(state->window);
        state->window = NULL;
    }
}
