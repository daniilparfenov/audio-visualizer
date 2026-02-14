#include "audio_utils.h"

float GetSampleNormalized(AppState* state, Uint32 sample_idx) {
    if (sample_idx >= state->wav_data_len) {
        return 0.0f;
    }

    if (SDL_AUDIO_BYTESIZE(state->wav_spec.format) == 2) {
        // Odd samples are unaligned by 2 bytes so we can't read them properly
        if (sample_idx & 1)
            sample_idx--;

        Sint16 raw_value = *(Sint16*)&state->wav_data[sample_idx];

        return (float)raw_value / 32768.f;
    }

    return 0.0f;
}