#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "app_state.h"
#include "audio_utils.h"
#include "visualizer.h"

#define RING_BUFFER_SIZE 8192

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
    const char* wav_path = "assets/440.wav";
    SDL_AudioSpec wav_spec;
    Uint8* wav_data = NULL;
    Uint32 wav_data_len = 0;
    if (!SDL_LoadWAV(wav_path, &wav_spec, &wav_data, &wav_data_len)) {
        SDL_Log("Couldn't load .wav file: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Convert audio to floats
    SDL_AudioSpec target_spec = wav_spec;
    target_spec.format = SDL_AUDIO_F32;

    Uint8* converted_data = NULL;
    int converted_len = 0;
    if (!SDL_ConvertAudioSamples(&wav_spec, wav_data, wav_data_len, &target_spec, &converted_data, &converted_len)) {
        SDL_Log("Failed to convert audio samples: %s", SDL_GetError());
        SDL_free(wav_data);
        return SDL_APP_FAILURE;
    }

    // Store converted data in our AppState
    state->samples = (float*)converted_data;
    state->samples_count = converted_len / sizeof(float);  // Total floats (samples * channels)
    state->sample_rate = target_spec.freq;
    state->channels = target_spec.channels;
    state->cur_sample_idx = 0;

    // Free original raw data
    SDL_free(wav_data);

    // Setup Ring Buffer
    state->ring_buffer_len = RING_BUFFER_SIZE;
    state->ring_buffer = SDL_calloc(state->ring_buffer_len, sizeof(float));
    state->ring_buffer_idx = 0;

    // Create AudioStream that mathes converted format (F32)
    state->stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &target_spec, NULL, NULL);
    if (!state->stream) {
        SDL_Log("Couldn't create audio stream: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Resume audio (because SDL starts the device paused)
    SDL_ResumeAudioStreamDevice(state->stream);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_ESCAPE) {
            return SDL_APP_SUCCESS;
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    AppState* state = (AppState*)appstate;

    // Clear the screen with black color
    SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
    SDL_RenderClear(state->renderer);

    FeedAudio(state);
    DrawWaveform(state);

    SDL_RenderPresent(state->renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    AppState* state = (AppState*)appstate;
    if (state) {
        if (state->stream) {
            SDL_DestroyAudioStream(state->stream);
        }

        if (state->samples) {
            SDL_free(state->samples);
        }

        if (state->ring_buffer) {
            SDL_free(state->ring_buffer);
        }

        if (state->renderer) {
            SDL_DestroyRenderer(state->renderer);
        }

        if (state->window) {
            SDL_DestroyWindow(state->window);
        }

        SDL_free(state);
    }
}
