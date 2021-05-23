#include <assert.h>

#include "spacerocks/lines.h"
#include "spacerocks/saucer.h"
#include "spacerocks/ship.h"
#include "spacerocks/util.h"

// TODO: Test collision detection more thoroughly.
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    float scale = 10.0f;
    Lines saucer = saucer_lines;
    transform_lines(&saucer, (Vec2Df){0}, 0.0f, scale);

    Vec2Df should_intersect[] = {
        {0.0f, 0.0f},
        {scale, scale},
    };
    for (size_t i = 0; i < ARRAY_SIZE(should_intersect); i++) {
        Lines ship = idling_ship_lines;
        transform_lines(&ship, should_intersect[i], 0.0f, scale);

        // polygon_intersects should tell when polygons in lines intersect.
        assert(polygons_intersect(&saucer, &ship));
    }

    Vec2Df should_not_intersect[] = {
        {0.0f, scale * 2.0f},
        {0.0f, -scale * 2.0f},
        {-scale * 2.0f, 0.0f},
        {scale * 2.0f, 0.0f},
    };
    for (size_t i = 0; i < ARRAY_SIZE(should_not_intersect); i++) {
        Lines ship = idling_ship_lines;
        transform_lines(&ship, should_not_intersect[i], 0.0f, scale);

        // polygon_intersects should tell when polygons in lines don't
        // intersect.
        assert(!polygons_intersect(&saucer, &ship));
    }

    return EXIT_SUCCESS;
}
