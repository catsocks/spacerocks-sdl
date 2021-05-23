#pragma once

#include "spacerocks/graphics.h"
#include "spacerocks/math.h"

// Very basic rigid body physics.
struct Physics {
    Vec2Df position;
    float angle; // in radians
    Vec2Df linear_velocity;
    Vec2Df linear_accel;
    float linear_damping;
    float angular_velocity;
    float angular_damping;
    float angular_accel;
    Vec2D wrap_around;
};

typedef struct Physics Physics;

Physics make_physics(Vec2D wrap_around);
Physics update_physics(Physics physics, double dt);
void physics_transform_graphics(Physics physics, Graphics *gfx);
