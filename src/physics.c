#include "spacerocks/physics.h"

Physics make_physics(Vec2D wrap_around) {
    return (Physics){
        .wrap_around = wrap_around,
    };
}

Physics update_physics(Physics physics, double dt) {
    physics.linear_velocity.x += physics.linear_accel.x * dt;
    physics.linear_velocity.y += physics.linear_accel.y * dt;

    physics.linear_velocity.x *= powf(1.0f - physics.linear_damping, dt);
    physics.linear_velocity.y *= powf(1.0f - physics.linear_damping, dt);

    physics.position.x += physics.linear_velocity.x * dt;
    physics.position.y += physics.linear_velocity.y * dt;

    physics.position =
        wrap_around_vec2df(physics.position, physics.wrap_around);

    physics.angular_velocity += physics.angular_accel * dt;

    physics.angular_velocity *= powf(1.0f - physics.angular_damping, dt);

    physics.angle += physics.angular_velocity * dt;

    return physics;
}

void physics_transform_graphics(Physics physics, Graphics *gfx) {
    transform_graphics(gfx, physics.position, physics.angle, gfx->scale);
}
