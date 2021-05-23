#include "spacerocks/game.h"

const Lines idling_ship_lines = {
    {
        {1.0f, 0.0f},
        {-1.0f, -1.0f},
        {-1.0f, -1.0f},
        {-0.75f, -0.5f},
        {-0.75f, -0.5f},
        {-0.6f, -0.0f},
        {-0.6f, -0.0f},
        {-0.75f, 0.5f},
        {-0.75f, 0.5f},
        {-1.0f, 1.0f},
        {-1.0f, 1.0f},
        {1.0f, 0.0f},
    },
    12,
    12,
};

static const Lines thrusting_ship_lines = {
    {
        {1.0f, 0.0f},
        {-1.0f, -1.0f},
        {-1.0f, -1.0f},
        {-0.75f, -0.5f},
        {-0.75f, -0.5f},
        {-0.6f, -0.0f},
        {-0.6f, -0.0f},
        {-0.75f, 0.5f},
        {-0.75f, 0.5f},
        {-1.0f, 1.0f},
        {-1.0f, 1.0f},
        {1.0f, 0.0f},
        // Rocket flame.
        {-0.75f, -0.5f},
        {-1.3f, -0.2f},
        {-1.3f, -0.2f},
        {-1.5f, 0.0f},
        {-1.5f, 0.0f},
        {-1.3f, 0.2f},
        {-1.3f, 0.2f},
        {-0.75f, 0.5f},
    },
    20,
    12,
};

static Physics make_ship_physics(Vec2D wrap_around);
static void enable_hyperspace(Ship *ship, double t);

Ship make_ship(Vec2D wrap_around) {
    SDL_Color color = {255, 255, 255, 255};
    return (Ship){
        .physics = make_ship_physics(wrap_around),
        .graphics = make_graphics(&idling_ship_lines, 10.0f, color),
    };
}

static Physics make_ship_physics(Vec2D wrap_around) {
    Physics physics = make_physics(wrap_around);
    physics.position = get_ship_spawn_position(wrap_around);
    physics.linear_damping = 0.5f;
    physics.angular_damping = 0.99f;
    return physics;
}

void spawn_ship(Ship *ship) {
    ship->spawned = true;
    ship->graphics.visible = true;
    ship->respawn_cooldown = 0.0;
    ship->physics = make_ship_physics(ship->physics.wrap_around);
    physics_transform_graphics(ship->physics, &ship->graphics);
}

void despawn_ship(Ship *ship, double t) {
    ship->spawned = false;
    ship->graphics.visible = false;
    ship->graphics.source_lines = &idling_ship_lines;
    ship->respawn_cooldown = t + 2.0;
    ship->sustained_fire = 0.0;
    ship->firing_overheat = false;
}

void check_ship_input(Ship *ship, Input *input, Controller ctrl, double t) {
    ship->thrusting = false;
    ship->firing = false;

    if (!ship->spawned) {
        return;
    }

    ship->physics.linear_accel = (Vec2Df){0};
    float linear_speed = 500.0f;
    if (controller_button_down(input, BUTTON_THRUST, ctrl)) {
        ship->thrusting = true;
        ship->physics.linear_accel.x = cosf(ship->physics.angle) * linear_speed;
        ship->physics.linear_accel.y = sinf(ship->physics.angle) * linear_speed;
    }

    ship->physics.angular_accel = 0.0f;
    float angular_speed = pi * 10.0f;
    if (controller_button_down(input, BUTTON_ROTATE_LEFT, ctrl)) {
        ship->physics.angular_accel = -angular_speed;
    } else if (controller_button_down(input, BUTTON_ROTATE_RIGHT, ctrl)) {
        ship->physics.angular_accel = angular_speed;
    }

    if (controller_button_down(input, BUTTON_FIRE, ctrl)) {
        if (!ship->hyperspace_enabled) {
            ship->firing = true;
        }
    }

    if (controller_button_down_not_held(input, BUTTON_HYPERSPACE, ctrl)) {
        enable_hyperspace(ship, t);
    }
}

void fire_ship_bullets(Ship *ship, Bullet bullets[], Audio *audio, double t) {
    if (!ship->firing || ship->firing_cooldown > t || ship->firing_overheat) {
        return;
    }

    float direction = ship->physics.angle;
    float distance = 10.0f;
    Vec2Df position = {
        .x = ship->physics.position.x + (cosf(direction) * distance),
        .y = ship->physics.position.y + (sinf(direction) * distance),
    };
    float speed = length_vec2df(ship->physics.linear_velocity) + 400.0f;
    Vec2Df velocity = (Vec2Df){
        .x = cosf(direction) * speed,
        .y = sinf(direction) * speed,
    };

    size_t i = 0;
    if (find_unspawned_bullet(&i, bullets, SHIP_BULLETS)) {
        spawn_bullet(&bullets[i], position, velocity, t);
    }

    ship->firing_cooldown = t + 0.2;

    ALuint source = audio->sources[AUDIO_SOURCE_SHIP_FIRE];
    ALuint buffer = audio->buffers[AUDIO_BUFFER_SHIP_FIRE];
    ALint current_buffer = 0;
    alGetSourcei(source, AL_BUFFER, &current_buffer);
    if (current_buffer == 0) {
        alSourcei(source, AL_BUFFER, buffer);
    }
    alSourcePlay(source);
}

void update_ship(Ship *ship, double t, double dt) {
    if (!ship->spawned) {
        return;
    }

    if (ship->firing_cooldown >= t && !ship->firing_overheat) {
        ship->sustained_fire = fmin(ship->sustained_fire + dt, 1.0);
        if (ship->sustained_fire >= 1.0) {
            ship->firing_overheat = true;
        }
    } else {
        ship->sustained_fire = fmax(ship->sustained_fire - dt, 0.0);
        if (ship->firing_overheat && ship->sustained_fire <= 0.5) {
            ship->sustained_fire = 0.0;
            ship->firing_overheat = false;
        }
    }

    if (ship->thrusting) {
        ship->thrusting_flame_timer =
            fmodf(ship->thrusting_flame_timer + dt, 0.12f);
    } else {
        ship->thrusting_flame_timer = 0.06f;
    }

    if (ship->thrusting_flame_timer > 0.06f) {
        ship->graphics.source_lines = &thrusting_ship_lines;
    } else {
        ship->graphics.source_lines = &idling_ship_lines;
    }

    ship->physics = update_physics(ship->physics, dt);
    physics_transform_graphics(ship->physics, &ship->graphics);
}

void play_ship_thrusting_sound(Ship *ship, Audio *audio) {
    // TODO: Remove this this macro when it's possible to tweak the source's
    // gain without causing crackling with the Emscripten OpenAL implementation.
#ifdef __EMSCRIPTEN__
    (void)ship;
    (void)audio;
#else
    ALuint source = audio->sources[AUDIO_SOURCE_SHIP_THRUST];
    if (!ship->thrusting) {
        alSourcef(source, AL_GAIN, 0.0);
        return;
    }

    ALint state = 0;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING) {
        alSourcef(source, AL_GAIN, 1.0);
        return;
    }

    ALuint buffer = audio->buffers[AUDIO_BUFFER_SHIP_THRUST];
    alSourcei(source, AL_BUFFER, buffer);
    alSourcei(source, AL_LOOPING, AL_TRUE);
    alSourcePlay(source);
#endif
}

static void enable_hyperspace(Ship *ship, double t) {
    if (ship->hyperspace_enabled) {
        return;
    }
    ship->hyperspace_enabled = true;
    ship->hyperspace_end_time = t + 1.0;
    ship->graphics.visible = false;
}

void disable_hyperspace(Ship *ship) {
    ship->hyperspace_enabled = false;
    ship->hyperspace_end_time = 0.0;
    ship->graphics.visible = true;

    ship->physics = make_ship_physics(ship->physics.wrap_around);
    ship->physics.position.x = frand_range(0.0f, ship->physics.wrap_around.x);
    ship->physics.position.y = frand_range(0.0f, ship->physics.wrap_around.y);
}

bool can_ship_spawn_safely(Ship *ship, Saucer *saucer, Rock rocks[]) {
    Vec2Df spawn = get_ship_spawn_position(ship->physics.wrap_around);

    if (saucer->graphics.visible) {
        if (distance_vec2df(spawn, saucer->physics.position) <= 100.0f) {
            return false;
        }
    }

    for (int i = 0; i < MAX_ROCKS; i++) {
        if (rocks[i].graphics.visible) {
            if (distance_vec2df(spawn, rocks[i].physics.position) <= 100.0f) {
                return false;
            }
        }
    }

    return true;
}

Vec2Df get_ship_spawn_position(Vec2D wrap_around) {
    return (Vec2Df){
        wrap_around.x / 2.0f,
        wrap_around.y / 2.0f,
    };
}
