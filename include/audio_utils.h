#ifndef AUDIO_UTILS_H
#define AUDIO_UTILS_H

#include <SDL3/SDL.h>
#include "app_state.h"

float GetSampleNormalized(AppState* state, Uint32 sample_idx);

#endif  // #ifndef AUDIO_UTILS_H