#ifndef APP_STATE_H
#define APP_STATE_H

#include <SDL3/SDL.h>

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
} AppState;

#endif  // #ifndef APP_STATE_H