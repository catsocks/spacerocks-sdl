#include "spacerocks/debris.h"

static const Lines ship_debris_lines = {
    {
        {0.0f, 1.0f},
        {0.5f, -1.0f},
        {0.5f, -1.0f},
        {-1.0f, -1.0f},
        {-1.0f, -1.0f},
        {0.0f, 1.0f},
    },
    6,
    6,
};

static const Lines rock_debris_lines = {
    {
        {1.0f, 0.0f},
        {0.31f, 0.95f},
        {0.31f, 0.95f},
        {-0.81f, 0.59f},
        {-0.81f, 0.59f},
        {-0.81f, -0.59f},
        {-0.81f, -0.59f},
        {0.31f, -0.95f},
        {0.31f, -0.95f},
        {1.0f, 0.0f},
    },
    10,
    10,
};

static const Lines saucer_debris_lines = {
    {
        {0.5f, 0.0f},     {0.87f, 0.4f},   {0.87f, 0.4f},   {0.25f, 0.34f},
        {0.25f, 0.34f},   {-0.0f, 0.8f},   {-0.0f, 0.8f},   {-0.25f, 0.34f},
        {-0.25f, 0.34f},  {-0.87f, 0.4f},  {-0.87f, 0.4f},  {-0.5f, -0.0f},
        {-0.5f, -0.0f},   {-0.87f, -0.4f}, {-0.87f, -0.4f}, {-0.25f, -0.34f},
        {-0.25f, -0.34f}, {0.0f, -0.8f},   {0.0f, -0.8f},   {0.25f, -0.34f},
        {0.25f, -0.34f},  {0.87f, -0.4f},  {0.87f, -0.4f},  {0.5f, 0.0f},
    },
    24,
    24,
};

static void spawn_debris(Debris *debris, DebrisKind kind, Vec2Df position);

Debris make_debris(Vec2D wrap_around) {
    return (Debris){
        .physics = make_physics(wrap_around),
    };
}

void update_debris(Debris *debris, double dt) {
    if (!debris->graphics.visible) {
        return;
    }

    if (distance_vec2df(debris->spawn, debris->physics.position) > 70.0f) {
        debris->graphics.visible = false;
        return;
    }

    debris->physics = update_physics(debris->physics, dt);
    physics_transform_graphics(debris->physics, &debris->graphics);
}

void spawn_debris_from_unspawned(Debris pile[MAX_DEBRIS], DebrisKind kind,
                                 Vec2Df position) {
    int pile_size[NUM_DEBRIS_KINDS] = {
        [DEBRIS_KIND_SHIP] = NUM_SHIP_DEBRIS,
        [DEBRIS_KIND_BIG_SAUCER] = MAX_SAUCER_DEBRIS,
        [DEBRIS_KIND_SMALL_SAUCER] = MAX_SAUCER_DEBRIS / 1.2,
        [DEBRIS_KIND_BIG_ROCK] = MAX_ROCK_DEBRIS,
        [DEBRIS_KIND_MEDIUM_ROCK] = MAX_ROCK_DEBRIS / 1.5,
        [DEBRIS_KIND_SMALL_ROCK] = MAX_ROCK_DEBRIS / 2,
    };

    int size = pile_size[kind];
    for (int i = 0; i < MAX_DEBRIS; i++) {
        if (pile[i].graphics.visible) {
            continue;
        }
        spawn_debris(&pile[i], kind, position);
        if (--size == 0) {
            return;
        }
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                 "No unspawned debris left to spawn.");
}

static void spawn_debris(Debris *debris, DebrisKind kind, Vec2Df position) {
    debris->spawn = position;

    float direction = frand_range(0.0f, pi * 2.0f);
    float speed = 50.0f + frand_range(-10.0f, 10.0f);
    debris->physics.linear_velocity.x = cosf(direction) * speed;
    debris->physics.linear_velocity.y = sinf(direction) * speed;
    debris->physics.angular_velocity = frand_range(-pi, pi);

    float distance = frand_range(5.0f, 20.0f);
    debris->physics.position.x = position.x + (cosf(direction) * distance);
    debris->physics.position.y = position.y + (sinf(direction) * distance);

    float rotation = frand_range(0.0f, pi / 2.0f);
    debris->physics.angle = rotation;

    const Lines *kinds_line[NUM_DEBRIS_KINDS] = {
        [DEBRIS_KIND_SHIP] = &ship_debris_lines,
        [DEBRIS_KIND_BIG_SAUCER] = &saucer_debris_lines,
        [DEBRIS_KIND_SMALL_SAUCER] = &saucer_debris_lines,
        [DEBRIS_KIND_BIG_ROCK] = &rock_debris_lines,
        [DEBRIS_KIND_MEDIUM_ROCK] = &rock_debris_lines,
        [DEBRIS_KIND_SMALL_ROCK] = &rock_debris_lines,
    };
    float kinds_scale[NUM_DEBRIS_KINDS] = {
        [DEBRIS_KIND_SHIP] = 5.0f,         [DEBRIS_KIND_BIG_SAUCER] = 6.0f,
        [DEBRIS_KIND_SMALL_SAUCER] = 5.0f, [DEBRIS_KIND_BIG_ROCK] = 5.0f,
        [DEBRIS_KIND_MEDIUM_ROCK] = 3.0f,  [DEBRIS_KIND_SMALL_ROCK] = 2.5f,
    };

    SDL_Color color = {150, 150, 150, 255};
    debris->graphics =
        make_graphics(kinds_line[kind], kinds_scale[kind], color);
    physics_transform_graphics(debris->physics, &debris->graphics);
    debris->graphics.visible = true;
}
