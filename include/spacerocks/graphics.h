#pragma once

#include <SDL.h>
#include <stdbool.h>

#include "spacerocks/debug.h"
#include "spacerocks/lines.h"
#include "spacerocks/math.h"
#include "spacerocks/renderer.h"

struct Graphics {
    const Lines *source_lines;
    Lines lines;
    float scale;
    SDL_Color color;
    bool visible;
    Rect2D bounding_box;
};

typedef struct Graphics Graphics;

Graphics make_graphics(const Lines *source_lines, float scale, SDL_Color color);
void transform_graphics(Graphics *gfx, Vec2Df position, float rotation,
                        float scale);
bool graphics_intersect(Graphics *gfx_1, Graphics *gfx_2);
void render_graphics(RendererWrapper *renderer, Graphics *gfx);
