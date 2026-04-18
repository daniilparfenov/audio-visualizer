#ifndef GUI_H
#define GUI_H

#include <SDL3/SDL.h>

// Nuklear settings
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_COMMAND_USERDATA

#include "nuklear.h"
#include "nuklear_sdl3_renderer.h"

// Forward declaration of Appstate
typedef struct AppState AppState;

// Draws the interface and returns Rect (x, y, w, h) in which the visualization can be drawn
void GUI_Draw(AppState* state, float* out_vis_x, float* out_vis_y, float* out_vis_w, float* out_vis_h);

#endif  // GUI_H
