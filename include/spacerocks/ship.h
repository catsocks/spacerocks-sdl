#pragma once

#include "spacerocks/audio.h"
#include "spacerocks/bullet.h"
#include "spacerocks/graphics.h"
#include "spacerocks/input.h"
#include "spacerocks/math.h"
#include "spacerocks/physics.h"
#include "spacerocks/rock.h"
#include "spacerocks/saucer.h"

struct Ship {
    Physics physics;
    Graphics graphics;
    bool thrusting;
    float thrusting_flame_timer;
    bool firing;
    double firing_cooldown;
    double sustained_fire;
    bool firing_overheat;
    bool hyperspace_enabled;
    double hyperspace_end_time;
    bool spawned;
    double respawn_cooldown;
};

typedef struct Ship Ship;

extern const Lines idling_ship_lines;

Ship make_ship(Vec2D wrap_around);
void spawn_ship(Ship *ship);
void despawn_ship(Ship *ship, double t);
void check_ship_input(Ship *ship, Input *input, Controller ctrl, double t);
void fire_ship_bullets(Ship *ship, Bullet bullets[], Audio *audio, double t);
void update_ship(Ship *ship, double t, double dt);
void play_ship_thrusting_sound(Ship *ship, Audio *audio);
void disable_hyperspace(Ship *ship);
bool can_ship_spawn_safely(Ship *ship, Saucer *saucer, Rock rocks[]);
Vec2Df get_ship_spawn_position(Vec2D wrap_around);
