#ifndef PLAYER_H
#define PLAYER_H

#include "app_state.h"

void Player_Init(AppState* state);

void Player_TogglePause(AppState* state);

// Seek by the specified number of seconds (can be negative)
void Player_SeekSeconds(AppState* state, float seconds);

// Toggle the looping mode
void Player_ToggleLoop(AppState* state);

// Toggle the song
void Player_NextSong(AppState* state);
void Player_PrevSong(AppState* state);

// Load the song from the playlist
SDL_AppResult Player_LoadSongIdx(AppState* state, int idx);

#endif  // PLAYER_H
