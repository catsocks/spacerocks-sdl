#pragma once

#include <SDL.h>

#include "spacerocks/math.h"
#include "spacerocks/renderer.h"

enum {
    MAX_LINES_POINTS = 24,
};

// Lines represents vector graphics that may optionally used in collision
// detection.
// The lines MUST be stored in pairs of points and the graphics they form should
// be upside down. If the graphics is to be used in collision detection, the
// first lines MUST form a simple polygon that may be concave, its size MUST
// be normalized to 1, and the number of points used to form it MUST be set as
// the value of polygon_length.
struct Lines {
    Vec2Df points[MAX_LINES_POINTS];
    int points_length;
    int polygon_length;
};

typedef struct Lines Lines;

void transform_lines(Lines *lines, Vec2Df translation, float rotation,
                     float scale);
bool polygons_intersect(Lines *lines_1, Lines *lines_2);
void render_lines(RendererWrapper *renderer, Lines *lines, SDL_Color color);
