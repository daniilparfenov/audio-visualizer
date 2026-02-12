#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "app_state.h"

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    AppState* state = SDL_calloc(1, sizeof(AppState));
    if (!state) {
        return SDL_APP_FAILURE;
    }
    *appstate = state;

    // Initialization of SDL subsystems
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Create Window + Renderer
    if (!SDL_CreateWindowAndRenderer("Music Visualizer", 800, 600, 0, &state->window, &state->renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Load .wav file
    const char* wav_path = "assets/wav_sample1.wav";
    SDL_AudioSpec wav_spec;
    if (!SDL_LoadWAV(wav_path, &wav_spec, &state->wav_data, &state->wav_data_len)) {
        SDL_Log("Couldn't load .wav file: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Create AudioStream
    state->stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &wav_spec, NULL, NULL);
    if (!state->stream) {
        SDL_Log("Couldn't create audio stream: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Put audio into stream
    SDL_PutAudioStreamData(state->stream, state->wav_data, state->wav_data_len);

    // Resume audio (because SDL starts the device paused)
    SDL_ResumeAudioStreamDevice(state->stream);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    AppState* state = (AppState*)appstate;
    const char* message = "Hello World!";
    int w = 0, h = 0;
    float x, y;
    const float scale = 4.0f;

    // Center the message and scale it up
    SDL_GetRenderOutputSize(state->renderer, &w, &h);
    SDL_SetRenderScale(state->renderer, scale, scale);
    x = ((w / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(message)) / 2;
    y = ((h / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE) / 2;

    // Clear the screen with black color
    SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
    SDL_RenderClear(state->renderer);

    // Draw the message
    SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(state->renderer, x, y, message);
    SDL_RenderPresent(state->renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    AppState* state = (AppState*)appstate;
    if (state) {
        if (state->stream) {
            SDL_DestroyAudioStream(state->stream);
        }

        if (state->wav_data) {
            SDL_free(state->wav_data);
        }

        SDL_free(state);
    }
}
