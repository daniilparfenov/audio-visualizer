#ifndef AUDIO_UTILS_H
#define AUDIO_UTILS_H

#include <SDL3/SDL.h>
#include "app_state.h"

void FeedAudio(AppState* state);

SDL_AppResult Audio_LoadAndSetup(AppState* state, const char* filepath);

void Audio_Cleanup(AppState* state);

#endif  // #ifndef AUDIO_UTILS_H