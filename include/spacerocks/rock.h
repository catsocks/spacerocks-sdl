#pragma once

#include "spacerocks/graphics.h"
#include "spacerocks/math.h"
#include "spacerocks/physics.h"
#include "spacerocks/util.h"

enum {
    MAX_BIG_ROCKS = 12,
    MAX_MEDIUM_ROCKS = MAX_BIG_ROCKS * 2,
    MAX_SMALL_ROCKS = MAX_MEDIUM_ROCKS * 2,
    MAX_ROCKS = MAX_SMALL_ROCKS,
};

enum RockSize {
    ROCK_SIZE_SMALL,
    ROCK_SIZE_MEDIUM,
    ROCK_SIZE_BIG,
    NUM_ROCK_SIZES,
};

typedef enum RockSize RockSize;

struct Rock {
    Physics physics;
    Graphics graphics;
    RockSize size;
};

typedef struct Rock Rock;

Rock make_rock(Vec2D wrap_around);
void despawn_rock(Rock *rock);
void spawn_large_rocks(Rock rocks[], int num);
void update_rock(Rock *rock, double dt);
void split_rock(Rock rocks[], Rock *rock);
