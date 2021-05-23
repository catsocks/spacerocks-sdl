#pragma once

#include <SDL.h>

#include "spacerocks/math.h"

// RendererWrapper allows for a fixed logical resolution to be used for
// rendering, mimicking the behaviour of SDL_RenderSetLogicalSize.
// Two issues as of SDL v2.0.13 prevent SDL_RenderSetLogicalSize from being used
// in the place of RendererWrapper:
//     1) SDL_RenderSetLogicalSize causes the game to be scaled incorrectly when
//        fullscreen is toggled with Emscripten.
//     2) Diagonal lines appear thinner than straight lines when scaling is
//        applied.
struct RendererWrapper {
    SDL_Renderer *renderer;
    Vec2D output_size;
    Vec2D logical_size;
    Rect2D viewport;
    float scale;
};

typedef struct RendererWrapper RendererWrapper;

void init_renderer_wrapper(RendererWrapper *wrapper, SDL_Renderer *renderer,
                           Vec2D logical_size);
Vec2Df renderer_wrapper_scale_vec2df(RendererWrapper *wrapper, Vec2Df vector);
Rect2D renderer_wrapper_scale_rect2d(RendererWrapper *wrapper, Rect2D rect);
Rect2Df renderer_wrapper_scale_rect2df(RendererWrapper *wrapper, Rect2Df rect);
