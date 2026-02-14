#include "audio_utils.h"

void FeedAudio(AppState* state) {
    const int MIN_BUFFERED_BYTES = 8192;
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