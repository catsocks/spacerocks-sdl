#pragma once

#include "spacerocks/graphics.h"
#include "spacerocks/math.h"
#include "spacerocks/physics.h"
#include "spacerocks/rock.h"

enum {
    NUM_SHIP_DEBRIS = 5,
    MAX_SAUCER_DEBRIS = 4,
    MAX_ROCK_DEBRIS = 8,
    MAX_DEBRIS =
        (MAX_ROCKS * MAX_ROCK_DEBRIS) + MAX_SAUCER_DEBRIS + NUM_SHIP_DEBRIS,
};

enum DebrisKind {
    DEBRIS_KIND_SHIP,
    DEBRIS_KIND_BIG_SAUCER,
    DEBRIS_KIND_SMALL_SAUCER,
    DEBRIS_KIND_BIG_ROCK,
    DEBRIS_KIND_MEDIUM_ROCK,
    DEBRIS_KIND_SMALL_ROCK,
    NUM_DEBRIS_KINDS,
};

typedef enum DebrisKind DebrisKind;

struct Debris {
    Physics physics;
    Graphics graphics;
    Vec2Df spawn;
};

typedef struct Debris Debris;

Debris make_debris(Vec2D wrap_around);
void update_debris(Debris *debris, double dt);
void spawn_debris_from_unspawned(Debris pile[MAX_DEBRIS], DebrisKind kind,
                                 Vec2Df position);
