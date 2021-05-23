#include "spacerocks/bullet.h"

static const Lines bullet_lines = {
    {
        {1.0f, 0.5f},
        {1.0f, -0.5f},
        {1.0f, -0.5f},
        {-1.0f, -0.5f},
        {-1.0f, -0.5f},
        {-1.0f, 0.5f},
        {-1.0f, 0.5f},
        {1.0f, 0.5f},
    },
    8,
    8,
};

Bullet make_bullet(Vec2D wrap_around) {
    SDL_Color color = {150, 150, 150, 255};
    return (Bullet){
        .physics = make_physics(wrap_around),
        .graphics = make_graphics(&bullet_lines, 1.0f, color),
    };
}

void spawn_bullet(Bullet *bullet, Vec2Df position, Vec2Df velocity, double t) {
    bullet->despawn_time = t + 1.6;

    bullet->graphics.visible = true;

    bullet->physics.position = position;
    bullet->physics.angle = atan2_vec2df(velocity);
    bullet->physics.linear_velocity = velocity;

    physics_transform_graphics(bullet->physics, &bullet->graphics);
}

void despawn_bullet(Bullet *bullet) {
    bullet->graphics.visible = false;
}

bool find_unspawned_bullet(size_t *idx, Bullet bullets[], size_t num_bullets) {
    for (size_t i = 0; i < num_bullets; i++) {
        if (bullets[i].graphics.visible) {
            continue;
        }
        *idx = i;
        return true;
    }
    return false;
}

void update_bullet(Bullet *bullet, double t, double dt) {
    if (!bullet->graphics.visible) {
        return;
    }

    if (bullet->despawn_time <= t) {
        bullet->graphics.visible = false;
        return;
    }

    bullet->physics = update_physics(bullet->physics, dt);

    physics_transform_graphics(bullet->physics, &bullet->graphics);
}
