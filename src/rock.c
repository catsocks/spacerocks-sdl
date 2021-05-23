#include "spacerocks/rock.h"

enum {
    NUM_ROCK_FORMS = 4,
};

static const Lines rock_1_lines = {
    {
        {0.0f, -0.8f},  {0.5f, -1.0f},  {0.5f, -1.0f},  {1.0f, -0.5f},
        {1.0f, -0.5f},  {0.5f, -0.2f},  {0.5f, -0.2f},  {1.0f, 0.2f},
        {1.0f, 0.2f},   {0.5f, 1.0f},   {0.5f, 1.0f},   {-0.2f, 0.7f},
        {-0.2f, 0.7f},  {-0.5f, 1.0f},  {-0.5f, 1.0f},  {-1.0f, 0.5f},
        {-1.0f, 0.5f},  {-0.7f, 0.0f},  {-0.7f, 0.0f},  {-1.0f, -0.5f},
        {-1.0f, -0.5f}, {-0.5f, -1.0f}, {-0.5f, -1.0f}, {0.0f, -0.8f},
    },
    24,
    24,
};

static const Lines rock_2_lines = {
    {
        {0.2f, -1.0f},  {1.0f, -0.5f},  {1.0f, -0.5f},  {1.0f, -0.2f},
        {1.0f, -0.2f},  {0.3f, 0.0f},   {0.3f, 0.0f},   {1.0f, 0.5f},
        {1.0f, 0.5f},   {0.4f, 1.0f},   {0.4f, 1.0f},   {0.2f, 0.7f},
        {0.2f, 0.7f},   {-0.5f, 1.0f},  {-0.5f, 1.0f},  {-1.0f, 0.3f},
        {-1.0f, 0.3f},  {-1.0f, -0.5f}, {-1.0f, -0.5f}, {-0.3f, -0.5f},
        {-0.3f, -0.5f}, {-0.5f, -1.0f}, {-0.5f, -1.0f}, {0.2f, -1.0f},
    },
    24,
    24,
};

static const Lines rock_3_lines = {
    {
        {0.0f, -0.5f},  {0.5f, -1.0f},  {0.5f, -1.0f},  {1.0f, -0.5f},
        {1.0f, -0.5f},  {0.7f, 0.0f},   {0.7f, 0.0f},   {1.0f, 0.5f},
        {1.0f, 0.5f},   {0.3f, 1.0f},   {0.3f, 1.0f},   {-0.5f, 1.0f},
        {-0.5f, 1.0f},  {-1.0f, 0.5f},  {-1.0f, 0.5f},  {-1.0f, -0.5f},
        {-1.0f, -0.5f}, {-0.5f, -1.0f}, {-0.5f, -1.0f}, {0.0f, -0.5f},
    },
    20,
    20,
};

static const Lines rock_4_lines = {
    {
        {0.5f, -1.0f},  {1.0f, -0.3f},  {1.0f, -0.3f},  {1.0f, 0.3f},
        {1.0f, 0.3f},   {0.5f, 1.0f},   {0.5f, 1.0f},   {0.0f, 1.0f},
        {0.0f, 1.0f},   {0.0f, 0.3f},   {0.0f, 0.3f},   {-0.5f, 1.0f},
        {-0.5f, 1.0f},  {-1.0f, 0.2f},  {-1.0f, 0.2f},  {-0.5f, 0.0f},
        {-0.5f, 0.0f},  {-1.0f, -0.3f}, {-1.0f, -0.3f}, {-0.2f, -1.0f},
        {-0.2f, -1.0f}, {0.5f, -1.0f},
    },
    22,
    22,
};

static bool find_unspawned_rock(size_t *idx, Rock rocks[]);
static void spawn_rock(Rock *rock, RockSize size, Vec2Df position,
                       Vec2Df velocity);
static Graphics make_rock_graphics(RockSize size);

Rock make_rock(Vec2D wrap_around) {
    return (Rock){
        .physics = make_physics(wrap_around),
    };
}

void despawn_rock(Rock *rock) {
    rock->graphics.visible = false;
}

void spawn_large_rocks(Rock rocks[], int num) {
    SDL_assert(num <= MAX_ROCKS);

    for (int i = 0; i < MIN(num, MAX_ROCKS); i++) {
        Vec2Df position = {0};
        // Rocks spawn on one of the edges of the screen.
        if (rand_bool()) {
            position.x = frand_range(0.0f, rocks[i].physics.wrap_around.x);
        } else {
            position.y = frand_range(0.0f, rocks[i].physics.wrap_around.y);
        }

        float direction = frand_range(0.0f, pi * 2.0f);
        float speed = 70.0f;
        Vec2Df velocity = {
            cosf(direction) * speed,
            sinf(direction) * speed,
        };

        // Assumes this function will only be called when there are no spawned
        // rocks left.
        spawn_rock(&rocks[i], ROCK_SIZE_BIG, position, velocity);
    }
}

static void spawn_rock(Rock *rock, RockSize size, Vec2Df position,
                       Vec2Df velocity) {
    rock->size = size;

    float sizes_max_speed[NUM_ROCK_SIZES] = {
        [ROCK_SIZE_SMALL] = pi / 5.0f,
        [ROCK_SIZE_MEDIUM] = pi / 9.0f,
        [ROCK_SIZE_BIG] = pi / 14.0f,
    };
    float max_speed = sizes_max_speed[rock->size];

    rock->physics.position = position;
    rock->physics.linear_velocity = velocity;
    rock->physics.angular_velocity = frand_range(-max_speed, max_speed);

    rock->graphics = make_rock_graphics(rock->size);
    physics_transform_graphics(rock->physics, &rock->graphics);
    rock->graphics.visible = true;
}

static bool find_unspawned_rock(size_t *idx, Rock rocks[]) {
    for (size_t i = 0; i < MAX_ROCKS; i++) {
        if (rocks[i].graphics.visible) {
            continue;
        }
        *idx = i;
        return true;
    }
    return false;
}

static Graphics make_rock_graphics(RockSize size) {
    const Lines *forms_lines[NUM_ROCK_FORMS] = {
        &rock_1_lines,
        &rock_2_lines,
        &rock_3_lines,
        &rock_4_lines,
    };
    float sizes_scale[NUM_ROCK_SIZES] = {
        [ROCK_SIZE_SMALL] = 8.0f,
        [ROCK_SIZE_MEDIUM] = 15.0f,
        [ROCK_SIZE_BIG] = 30.0f,
    };
    SDL_Color color = {150, 150, 150, 255};
    int form = rand_range(0, NUM_ROCK_FORMS - 1);
    return make_graphics(forms_lines[form], sizes_scale[size], color);
}

void update_rock(Rock *rock, double dt) {
    if (!rock->graphics.visible) {
        return;
    }
    rock->physics = update_physics(rock->physics, dt);
    physics_transform_graphics(rock->physics, &rock->graphics);
}

void split_rock(Rock rocks[], Rock *rock) {
    SDL_assert(rock->size != ROCK_SIZE_SMALL);
    if (rock->size == ROCK_SIZE_SMALL) {
        return;
    }

    float original_direction = atan2_vec2df(rock->physics.linear_velocity);

    Vec2Df position = rock->physics.position;
    RockSize size = rock->size - 1;
    // NOTE: Consider implementing mass in Physics so that the mass of the
    // rock may be changed instead of its velocity.
    float direction = original_direction + frand_range(pi / -2.0f, pi / 2.0f);
    float speed = length_vec2df(rock->physics.linear_velocity) * 1.5f;
    Vec2Df velocity = {
        .x = cosf(direction) * speed,
        .y = sinf(direction) * speed,
    };

    size_t i = 0;
    bool found = find_unspawned_rock(&i, rocks);
    SDL_assert(found);
    if (found) {
        spawn_rock(&rocks[i], size, position, velocity);
    }

    float other_direction = 0;
    do {
        other_direction =
            original_direction + frand_range(pi / -2.0f, pi / 2.0f);
    } while (fabsf(direction - other_direction) < pi / 6.0f);

    velocity = (Vec2Df){
        .x = cosf(other_direction) * speed,
        .y = sinf(other_direction) * speed,
    };

    if ((found = find_unspawned_rock(&i, rocks))) {
        spawn_rock(&rocks[i], size, position, velocity);
    }
    SDL_assert(found);
}
