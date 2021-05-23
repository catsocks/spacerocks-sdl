#include "spacerocks/graphics.h"

static void set_bounding_box(Graphics *gfx, Vec2Df position);
static void debug_render_bounding_box(RendererWrapper *renderer, Rect2D box);

Graphics make_graphics(const Lines *source_lines, float scale,
                       SDL_Color color) {
    SDL_assert(source_lines != NULL);
    Graphics gfx = {
        .source_lines = source_lines,
        .lines = *source_lines,
        .scale = scale,
        .color = color,
    };
    transform_lines(&gfx.lines, (Vec2Df){0}, 0.0f, scale);
    set_bounding_box(&gfx, (Vec2Df){0});
    return gfx;
}

static void set_bounding_box(Graphics *gfx, Vec2Df position) {
    float padding = 15.0f;
    gfx->bounding_box.x = position.x - gfx->scale - (padding / 2.0f);
    gfx->bounding_box.y = position.y - gfx->scale - (padding / 2.0f);
    gfx->bounding_box.w = (gfx->scale * 2.0f) + padding;
    gfx->bounding_box.h = (gfx->scale * 2.0f) + padding;
}

void transform_graphics(Graphics *gfx, Vec2Df position, float rotation,
                        float scale) {
    gfx->scale = scale;
    gfx->lines = *gfx->source_lines;
    transform_lines(&gfx->lines, position, rotation, gfx->scale);
    set_bounding_box(gfx, position);
}

bool graphics_intersect(Graphics *gfx_1, Graphics *gfx_2) {
    if (gfx_1->visible && gfx_2->visible) {
        if (SDL_HasIntersection(&gfx_1->bounding_box, &gfx_2->bounding_box)) {
            return polygons_intersect(&gfx_1->lines, &gfx_2->lines);
        }
    }
    return false;
}

void render_graphics(RendererWrapper *renderer, Graphics *gfx) {
    if (gfx->visible) {
        render_lines(renderer, &gfx->lines, gfx->color);

        if (DEBUG_BOUNDING_BOXES) {
            debug_render_bounding_box(renderer, gfx->bounding_box);
        }
    }
}

static void debug_render_bounding_box(RendererWrapper *renderer, Rect2D box) {
    uint8_t c[4] = {0};
    SDL_GetRenderDrawColor(renderer->renderer, &c[0], &c[1], &c[2], &c[3]);
    SDL_SetRenderDrawColor(renderer->renderer, 0, 255, 0, 255);
    Rect2D rect = renderer_wrapper_scale_rect2d(renderer, box);
    SDL_RenderDrawRect(renderer->renderer, &rect);
    SDL_SetRenderDrawColor(renderer->renderer, c[0], c[1], c[2], c[3]);
}
