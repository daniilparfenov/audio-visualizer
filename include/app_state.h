#ifndef APP_STATE_H
#define APP_STATE_H

#include <SDL3/SDL.h>

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;

    // Sound
    SDL_AudioStream* stream;
    Uint8* wav_data;
    Uint32 wav_data_len;
} AppState;

#endif  // #ifndef APP_STATE_H