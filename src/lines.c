#include "spacerocks/lines.h"

static bool point_in_polygon(Lines *lines, Vec2Df point);

void transform_lines(Lines *lines, Vec2Df translation, float rotation,
                     float scale) {
    for (int i = 0; i < lines->points_length; i++) {
        // Assumes the origin of the lines is (0,0).
        lines->points[i] = rotate_vec2df(lines->points[i], rotation);

        lines->points[i].x *= scale;
        lines->points[i].y *= scale;

        lines->points[i].x += translation.x;
        lines->points[i].y += translation.y;
    }
}

bool polygons_intersect(Lines *lines_1, Lines *lines_2) {
    for (int i = 0; i < lines_1->polygon_length; i += 2) {
        if (point_in_polygon(lines_2, lines_1->points[i])) {
            return true;
        }
    }
    return false;
}

// Implements the crossing number test.
static bool point_in_polygon(Lines *lines, Vec2Df point) {
    int crossing_counter = 0;
    for (int i = 0; i < lines->polygon_length; i += 2) {
        Vec2Df line_start = lines->points[i];
        Vec2Df line_end = lines->points[(i + 1) % lines->polygon_length];

        // Test for an upward or downward crossing.
        if (((line_start.y <= point.y) && (line_end.y > point.y)) ||
            ((line_start.y > point.y) && (line_end.y <= point.y))) {
            // Compute the actual edge-ray intersect x-coordinate.
            float vt = (point.y - line_start.y) / (line_end.y - line_start.y);
            // Tesr if point.x < intersect.
            if (point.x < line_start.x + (vt * (line_end.x - line_start.x))) {
                // A valid crossing of y=point.y right of point.x.
                crossing_counter++;
            }
        }
    }
    return crossing_counter % 2 != 0;
}

void render_lines(RendererWrapper *renderer, Lines *lines, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer->renderer, color.r, color.b, color.b,
                           color.a);

    for (int i = 0; i < lines->points_length; i += 2) {
        Vec2Df line_start = lines->points[i];
        Vec2Df line_end = lines->points[(i + 1) % lines->points_length];
        line_start = renderer_wrapper_scale_vec2df(renderer, line_start);
        line_end = renderer_wrapper_scale_vec2df(renderer, line_end);
        SDL_RenderDrawLineF(renderer->renderer, line_start.x, line_start.y,
                            line_end.x, line_end.y);
    }
}
