#pragma once

#include <SDL.h>

#include "spacerocks/debug.h"
#include "spacerocks/lines.h"
#include "spacerocks/math.h"
#include "spacerocks/renderer.h"

void render_text(RendererWrapper *renderer, Vec2Df position, float height,
                 const char *str, SDL_Color color);
Vec2Df get_rendered_text_size(float height, const char *str);
