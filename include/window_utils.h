#ifndef WINDOW_UTILS_H
#define WINDOW_UTILS_H

#include <SDL3/SDL.h>
#include "app_state.h"

SDL_AppResult Window_Init(AppState* state);

void Window_Cleanup(AppState* state);

#endif  // WINDOW_UTILS_H
