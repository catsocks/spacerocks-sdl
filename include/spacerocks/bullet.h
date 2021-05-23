#pragma once

#include "spacerocks/graphics.h"
#include "spacerocks/math.h"
#include "spacerocks/physics.h"

enum {
    SHIP_BULLETS = 15,
    SAUCER_BULLETS = 15,
    MAX_BULLETS = SHIP_BULLETS + SAUCER_BULLETS,
};

struct Bullet {
    Physics physics;
    Graphics graphics;
    double despawn_time;
};

typedef struct Bullet Bullet;

Bullet make_bullet(Vec2D wrap_around);
void spawn_bullet(Bullet *bullet, Vec2Df position, Vec2Df velocity, double t);
void despawn_bullet(Bullet *bullet);
bool find_unspawned_bullet(size_t *idx, Bullet bullets[], size_t num_bullets);
void update_bullet(Bullet *bullet, double t, double dt);
