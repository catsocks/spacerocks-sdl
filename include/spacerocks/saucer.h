#pragma once

#include "spacerocks/assets.h"
#include "spacerocks/audio.h"
#include "spacerocks/bullet.h"
#include "spacerocks/graphics.h"
#include "spacerocks/math.h"
#include "spacerocks/physics.h"

struct SaucerTarget {
    Vec2Df position;
    Vec2Df velocity;
    bool active;
};

typedef struct SaucerTarget SaucerTarget;

struct Saucer {
    Physics physics;
    Graphics graphics;
    bool big;
    double spawn_time;
    SaucerTarget target;
    double firing_cooldown;
    double start_firing_time;
    double direction_change_time;
    float accuracy;
    bool on_edge;
    double random_hiding_time;
};

typedef struct Saucer Saucer;

extern const Lines saucer_lines;

Saucer make_saucer(Vec2D wrap_around, double t);
void set_saucer_target(Saucer *saucer, Physics physics, bool active);
void fire_saucer_bullets(Saucer *saucer, Bullet bullets[], Audio *audio,
                         double t);
void despawn_saucer(Saucer *saucer, bool destroyed, double t);
void update_saucer(Saucer *saucer, int player_score, double t, double dt);
void play_saucer_warning_sound(Saucer *saucer, Audio *audio);
