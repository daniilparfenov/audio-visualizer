#include "audio_utils.h"
#include <SDL3_mixer/SDL_mixer.h>

// This callback is called by the mixer every time it sends a chunk of audio to the sound card.
// So we use it to take a copy of the audio for visualization.
static void SDLCALL AudioPostMixCallback(void* udata, MIX_Mixer* mixer, const SDL_AudioSpec* spec, float* pcm,
                                         int samples) {
    AppState* state = (AppState*)udata;

    int frames = samples / state->channels;
    for (int i = 0; i < frames; ++i) {
        // Take a sample (for now we simply use the left channel)
        float sample_val = pcm[i * state->channels];

        state->ring_buffer[state->ring_buffer_idx] = sample_val;
        state->ring_buffer_idx = (state->ring_buffer_idx + 1) % state->ring_buffer_len;
    }
}

void FeedAudio(AppState* state) {
    if (state->track && !MIX_TrackPlaying(state->track) && state->player.is_playing) {
        if (state->player.is_looping) {
            MIX_PlayTrack(state->track, 0);  // Restart the track
        } else if (state->player.playlist_count > 1) {
            state->player.wants_next_song = 1;
        } else {
            state->player.is_playing = 0;
            SDL_Log("Player: Reached end of playlist, playback stopped.");
        }
    }
}

SDL_AppResult Audio_Init(AppState* state) {
    // Initialize SDL_mixer
    if (!MIX_Init()) {
        SDL_Log("Couldn't initialize SDL_mixer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Force F32 format and 2 channels so we are guaranteed to receive floats in PostMixCallback
    SDL_AudioSpec spec;
    spec.freq = 44100;
    spec.format = SDL_AUDIO_F32;
    spec.channels = 2;

    state->mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
    if (!state->mixer) {
        SDL_Log("MIX_CreateMixerDevice failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    state->sample_rate = spec.freq;
    state->channels = spec.channels;

    // Set callback to feed data into the ring buffer for visualization
    MIX_SetPostMixCallback(state->mixer, AudioPostMixCallback, state);

    // Set up the Ring Buffer
    state->ring_buffer_len = RING_BUFFER_SIZE;
    state->ring_buffer = SDL_calloc(state->ring_buffer_len, sizeof(float));

    return SDL_APP_CONTINUE;
}

SDL_AppResult Audio_LoadAndSetup(AppState* state, const char* filepath) {
    // Reset track and unbind previous audio
    if (state->track) {
        MIX_StopTrack(state->track, 0);  // 0 frames for fade-out
        MIX_SetTrackAudio(state->track, NULL);
    } else {
        state->track = MIX_CreateTrack(state->mixer);
    }

    // Free the previous audio object
    if (state->audio) {
        MIX_DestroyAudio(state->audio);
        state->audio = NULL;
    }

    // Reset ring buffer position
    state->ring_buffer_idx = 0;
    if (state->ring_buffer) {
        SDL_memset(state->ring_buffer, 0, state->ring_buffer_len * sizeof(float));
    }

    // Load new audio file (with predecode flag set to true)
    state->audio = MIX_LoadAudio(state->mixer, filepath, true);
    if (!state->audio) {
        SDL_Log("Couldn't load music %s: %s", filepath, SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Bind new audio to our track
    MIX_SetTrackAudio(state->track, state->audio);

    // Start playback
    if (!MIX_PlayTrack(state->track, 0)) {
        SDL_Log("MIX_PlayTrack: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    state->player.is_playing = 1;

    return SDL_APP_CONTINUE;
}

void Audio_Cleanup(AppState* state) {
    if (!state)
        return;

    if (state->track) {
        MIX_DestroyTrack(state->track);
        state->track = NULL;
    }
    if (state->audio) {
        MIX_DestroyAudio(state->audio);
        state->audio = NULL;
    }
    if (state->ring_buffer) {
        SDL_free(state->ring_buffer);
        state->ring_buffer = NULL;
    }

    if (state->mixer) {
        MIX_DestroyMixer(state->mixer);
        state->mixer = NULL;
    }

    MIX_Quit();
}
