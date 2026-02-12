#include "visualizer.h"

#include <SDL3/SDL.h>
#include "audio_utils.h"

void DrawWaveform(AppState* state) {

    static Uint64 start_time = 0;
    if (start_time == 0)
        start_time = SDL_GetTicks();

    Uint64 cur_time = SDL_GetTicks();
    float seconds_elapsed = (cur_time - start_time) / 1000.f;

    int bytes_per_sample = SDL_AUDIO_BYTESIZE(state->wav_spec.format);
    int bytes_per_second = state->wav_spec.freq * state->wav_spec.channels * bytes_per_sample;
    Uint32 cur_byte_idx = (Uint32)(seconds_elapsed * bytes_per_second);

    if (cur_byte_idx >= state->wav_data_len) {
        start_time = SDL_GetTicks();

        SDL_ClearAudioStream(state->stream);
        SDL_PutAudioStreamData(state->stream, state->wav_data, state->wav_data_len);
        cur_byte_idx = 0;
    }

    SDL_SetRenderDrawColor(state->renderer, 0, 255, 0, 255);  // Green

    int window_w, window_h;
    SDL_GetRenderOutputSize(state->renderer, &window_w, &window_h);

    int center_y = window_h / 2;
    float amplitude = window_h / 3.0f;
    for (int x = 0; x < window_w; x++) {
        Uint32 current_index = cur_byte_idx + (x * bytes_per_sample * state->wav_spec.channels);

        float sample = GetSampleNormalized(state, current_index);

        int y = center_y - (int)(sample * amplitude);

        SDL_RenderPoint(state->renderer, x, y);
    }
}
