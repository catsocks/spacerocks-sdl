#include "spacerocks/renderer.h"

static void update_renderer_wrapper(RendererWrapper *wrapper);
static int renderer_wrapper_event_watch(void *user_data, SDL_Event *event);

void init_renderer_wrapper(RendererWrapper *wrapper, SDL_Renderer *renderer,
                           Vec2D logical_size) {
    wrapper->renderer = renderer;
    wrapper->logical_size = logical_size;

    update_renderer_wrapper(wrapper);

    SDL_AddEventWatch(renderer_wrapper_event_watch, wrapper);
}

static void update_renderer_wrapper(RendererWrapper *wrapper) {
    SDL_GetRendererOutputSize(wrapper->renderer, &wrapper->output_size.x,
                              &wrapper->output_size.y);

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Renderer output size: %dx%d",
                 wrapper->output_size.x, wrapper->output_size.y);

    wrapper->scale =
        fminf(wrapper->output_size.y / (float)wrapper->logical_size.y,
              wrapper->output_size.x / (float)wrapper->logical_size.x);

    wrapper->viewport.w = wrapper->scale * wrapper->logical_size.x;
    wrapper->viewport.h = wrapper->scale * wrapper->logical_size.y;
    wrapper->viewport.x =
        (wrapper->output_size.x - (wrapper->viewport.w)) / 2.0f;
    wrapper->viewport.y =
        (wrapper->output_size.y - (wrapper->viewport.h)) / 2.0f;
}

static int renderer_wrapper_event_watch(void *user_data, SDL_Event *event) {
    RendererWrapper *wrapper = user_data;

    // The code below has been adapted from SDL_RendererEventWatch.
    if (event->type == SDL_WINDOWEVENT) {
        if (event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            update_renderer_wrapper(wrapper);
        }
    } else if (event->type == SDL_FINGERDOWN || event->type == SDL_FINGERUP ||
               event->type == SDL_FINGERMOTION) {
        if (wrapper->output_size.x == 0.0f) {
            event->tfinger.x = 0.5f;
        } else {
            float normalized_viewport_y =
                wrapper->viewport.x / (float)wrapper->output_size.x;
            float normalized_viewport_h =
                wrapper->viewport.w / (float)wrapper->output_size.x;
            if (event->tfinger.x <= normalized_viewport_y) {
                event->tfinger.x = 0.0f;
            } else if (event->tfinger.x >=
                       (normalized_viewport_y + normalized_viewport_h)) {
                event->tfinger.x = 1.0f;
            } else {
                event->tfinger.x = (event->tfinger.x - normalized_viewport_y) /
                                   normalized_viewport_h;
            }
        }

        if (wrapper->output_size.y == 0.0f) {
            event->tfinger.y = 0.5f;
        } else {
            float normalized_viewport_y =
                wrapper->viewport.y / (float)wrapper->output_size.y;
            float normalized_viewport_h =
                wrapper->viewport.h / (float)wrapper->output_size.y;
            if (event->tfinger.y <= normalized_viewport_y) {
                event->tfinger.y = 0.0f;
            } else if (event->tfinger.y >=
                       (normalized_viewport_y + normalized_viewport_h)) {
                event->tfinger.y = 1.0f;
            } else {
                event->tfinger.y = (event->tfinger.y - normalized_viewport_y) /
                                   normalized_viewport_h;
            }
        }
    }

    return 0;
}

Vec2Df renderer_wrapper_scale_vec2df(RendererWrapper *wrapper, Vec2Df vector) {
    vector.x *= wrapper->scale;
    vector.y *= wrapper->scale;
    vector.x += wrapper->viewport.x;
    vector.y += wrapper->viewport.y;
    return vector;
}

Rect2D renderer_wrapper_scale_rect2d(RendererWrapper *wrapper, Rect2D rect) {
    rect.x *= wrapper->scale;
    rect.y *= wrapper->scale;
    rect.x += wrapper->viewport.x;
    rect.y += wrapper->viewport.y;
    rect.w *= wrapper->scale;
    rect.h *= wrapper->scale;
    return rect;
}

Rect2Df renderer_wrapper_scale_rect2df(RendererWrapper *wrapper, Rect2Df rect) {
    rect.x *= wrapper->scale;
    rect.y *= wrapper->scale;
    rect.x += wrapper->viewport.x;
    rect.y += wrapper->viewport.y;
    rect.w *= wrapper->scale;
    rect.h *= wrapper->scale;
    return rect;
}
