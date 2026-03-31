#ifndef VISUALIZER_H
#define VISUALIZER_H

#include "app_state.h"

void DrawWaveform(AppState* state, const SDL_FRect* canvas);

void DrawSpectrum(AppState* state, const SDL_FRect* canvas);

#endif  // #ifndef VISUALIZER_H
