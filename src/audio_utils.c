#include "audio_utils.h"

void FeedAudio(AppState* state) {
    const int MIN_BUFFERED_BYTES = 32768;
    int queued_bytes = SDL_GetAudioStreamAvailable(state->stream);

    // If there are more bytes in the stream than we need, we don't need to feed the stream
    if (queued_bytes >= MIN_BUFFERED_BYTES) {
        return;
    }

    // Calc how much samples we have to add
    int bytes_to_add = MIN_BUFFERED_BYTES - queued_bytes;
    int samples_to_add = bytes_to_add / sizeof(float);
    // avoiding channel spliiting
    if (state->channels > 0)
        samples_to_add = (samples_to_add / state->channels) * state->channels;

    if (samples_to_add == 0)
        return;

    for (int samples_fed = 0; samples_fed < samples_to_add;) {
        int remaining_samples_in_file = state->samples_count - state->cur_sample_idx;
        int chunk_size = samples_to_add - samples_fed;  // The samples count we should add on cur iteration
        if (chunk_size > remaining_samples_in_file) {
            chunk_size = remaining_samples_in_file;
        }

        // If the file is ended, loop the audio
        if (chunk_size == 0) {
            state->cur_sample_idx = 0;
            continue;
        }

        // Put audio into the stream
        SDL_PutAudioStreamData(state->stream, &state->samples[state->cur_sample_idx], chunk_size * sizeof(float));

        // Put audio into the ring buffer (so far, only the left channel)
        int frames = chunk_size / state->channels;
        for (int i = 0; i < frames; i++) {
            float sample_val = state->samples[state->cur_sample_idx + i * state->channels];

            state->ring_buffer[state->ring_buffer_idx] = sample_val;
            state->ring_buffer_idx = (state->ring_buffer_idx + 1) % state->ring_buffer_len;
        }

        state->cur_sample_idx += chunk_size;
        samples_fed += chunk_size;
    }
}

SDL_AppResult Audio_LoadAndSetup(AppState* state, const char* filepath) {
    SDL_AudioSpec wav_spec;
    Uint8* wav_data = NULL;
    Uint32 wav_data_len = 0;

    // Load .wav file
    if (!SDL_LoadWAV(filepath, &wav_spec, &wav_data, &wav_data_len)) {
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

void Audio_Cleanup(AppState* state) {
    if (!state)
        return;

    if (state->stream) {
        SDL_DestroyAudioStream(state->stream);
        state->stream = NULL;
    }
    if (state->samples) {
        SDL_free(state->samples);
        state->samples = NULL;
    }
    if (state->ring_buffer) {
        SDL_free(state->ring_buffer);
        state->ring_buffer = NULL;
    }
}
