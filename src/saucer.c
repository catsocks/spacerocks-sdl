#include "spacerocks/saucer.h"

static const float big_saucer_scale = 20.0f;
static const float small_saucer_scale = 10.0f;

const Lines saucer_lines = {
    {
        {0.2f, -0.6f},
        {0.4f, -0.2f},
        {0.4f, -0.2f},
        {1.0f, 0.2f},
        {1.0f, 0.2f},
        {0.4f, 0.6f},
        {0.4f, 0.6f},
        {-0.4f, 0.6f},
        {-0.4f, 0.6f},
        {-1.0f, 0.2f},
        {-1.0f, 0.2f},
        {-0.4f, -0.2f},
        {-0.4f, -0.2f},
        {-0.2f, -0.6f},
        {-0.2f, -0.6f},
        {0.2f, -0.6f},
        // 2nd horizontal line.
        {0.4f, -0.2f},
        {-0.4f, -0.2f},
        // 3rd horizontal line.
        {-1.0f, 0.2f},
        {1.0f, 0.2f},
    },
    20,
    16,
};

static void spawn_saucer(Saucer *saucer, int player_score, double t);
static void set_saucer_direction_change_time(Saucer *saucer, double t);
static Physics set_saucer_velocity(Physics physics, bool initial_direction);

Saucer make_saucer(Vec2D wrap_around, double t) {
    SDL_Color color = {255, 255, 255, 255};
    return (Saucer){
        .physics = make_physics(wrap_around),
        .graphics = make_graphics(&saucer_lines, big_saucer_scale, color),
        .big = true,
        .spawn_time = t + 8.0,
    };
}

void set_saucer_target(Saucer *saucer, Physics physics, bool active) {
    saucer->target.position = physics.position;
    saucer->target.velocity = physics.linear_velocity;
    saucer->target.active = active;
}

void fire_saucer_bullets(Saucer *saucer, Bullet bullets[], Audio *audio,
                         double t) {
    if (!saucer->graphics.visible || saucer->firing_cooldown > t ||
        saucer->start_firing_time > t) {
        return;
    }

    float direction = frand_range(0.0f, pi * 2.0f);
    if (saucer->target.active) {
        // TODO: Take the ship's velocity into account.
        Vec2Df position = saucer->target.position;
        position.x -= saucer->physics.position.x;
        position.y -= saucer->physics.position.y;
        direction = atan2_vec2df(position);

        float error_factor = 1.0f - saucer->accuracy;
        direction += frand_range(error_factor * -pi, error_factor * pi);
    }

    float distance = saucer->graphics.scale + 5.0f;
    Vec2Df position = {
        .x = saucer->physics.position.x + (cosf(direction) * distance),
        .y = saucer->physics.position.y + (sinf(direction) * distance),
    };
    float speed = length_vec2df(saucer->physics.linear_velocity) + 400.0f;
    Vec2Df velocity = (Vec2Df){
        .x = cosf(direction) * speed,
        .y = sinf(direction) * speed,
    };

    size_t i = 0;
    if (find_unspawned_bullet(&i, bullets, SAUCER_BULLETS)) {
        spawn_bullet(&bullets[i], position, velocity, t);
    }

    saucer->firing_cooldown = t + 0.4;

    ALuint source = audio->sources[AUDIO_SOURCE_SAUCER_FIRE];
    ALuint buffer = audio->buffers[AUDIO_BUFFER_SAUCER_FIRE];
    ALint current_buffer = 0;
    alGetSourcei(source, AL_BUFFER, &current_buffer);
    if (current_buffer == 0) {
        alSourcei(source, AL_BUFFER, buffer);
    }
    alSourcePlay(source);
}

void despawn_saucer(Saucer *saucer, bool destroyed, double t) {
    saucer->random_hiding_time = 0.0;
    saucer->spawn_time = t + ((destroyed) ? 16.0 : 8.0);
    saucer->graphics.visible = false;
}

void update_saucer(Saucer *saucer, int player_score, double t, double dt) {
    if (saucer->spawn_time != 0.0 && saucer->spawn_time <= t) {
        spawn_saucer(saucer, player_score, t);
    }

    if (!saucer->graphics.visible) {
        return;
    }

    if (saucer->big) {
        // (0.00002 * x) = 0.6 where x is 30000.
        saucer->accuracy = fminf((0.00002f * player_score), 0.6f);
    } else {
        // (0.00002 * x) + 0.4 = 1.0 where x is 30000.
        saucer->accuracy = fminf((0.00002f * player_score) + 0.4f, 1.0f);
    }

    // There is a random chance that the saucer will disappear when it's over an
    // horizontal edge and respawn a few seconds later.
    if (saucer->physics.position.x + saucer->graphics.scale < 5.0f ||
        saucer->physics.position.x + saucer->graphics.scale >
            saucer->physics.wrap_around.x - 5.0f) {
        if (!saucer->on_edge) {
            if (percent_chance(25)) {
                despawn_saucer(saucer, false, t);
                return;
            }
        }
        saucer->on_edge = true;
    } else {
        saucer->on_edge = false;
    }

    // The small saucer will disappear at random times.
    if (!saucer->big) {
        if (saucer->random_hiding_time == 0.0) {
            saucer->random_hiding_time = t + frand_range(6.0f, 16.0f);
        } else if (saucer->random_hiding_time < t) {
            saucer->random_hiding_time = 0.0;
            despawn_saucer(saucer, false, t);
            return;
        }
    }

    if (saucer->direction_change_time <= t) {
        saucer->physics = set_saucer_velocity(saucer->physics, false);
        set_saucer_direction_change_time(saucer, t);
    }

    saucer->physics = update_physics(saucer->physics, dt);
    physics_transform_graphics(saucer->physics, &saucer->graphics);
}

static void spawn_saucer(Saucer *saucer, int player_score, double t) {
    saucer->spawn_time = 0.0;
    saucer->start_firing_time = t + 0.5;
    saucer->on_edge = true;

    // player_score * 0.0025 = 100 where player_score is 40000.
    // TODO: It'd be nice to increase the chance of small saucers appearing in
    // the second or third round.
    saucer->big = percent_chance(fmax(100.0 - (player_score * 0.0025), 0.0));

    if (rand_bool()) {
        saucer->physics.position.x = 0.0f;
    } else {
        saucer->physics.position.x = saucer->physics.wrap_around.x;
    }

    saucer->physics.position.y =
        frand_range(saucer->graphics.scale,
                    saucer->physics.wrap_around.y - saucer->graphics.scale);

    saucer->physics = set_saucer_velocity(saucer->physics, true);

    saucer->graphics = make_graphics(
        &saucer_lines, (saucer->big) ? big_saucer_scale : small_saucer_scale,
        saucer->graphics.color);

    physics_transform_graphics(saucer->physics, &saucer->graphics);
    saucer->graphics.visible = true;

    set_saucer_direction_change_time(saucer, t);
}

static void set_saucer_direction_change_time(Saucer *saucer, double t) {
    saucer->direction_change_time = t + rand_range(2, 10);
}

static Physics set_saucer_velocity(Physics physics, bool initial_direction) {
    float direction = 0.0f;
    if (initial_direction) {
        direction = rand_range(1, 2) * pi;
    } else {
        direction = frand_range(0.0f, 2.0f * pi);
    };
    float speed = 100.0f;
    physics.linear_velocity.x = cosf(direction) * speed;
    physics.linear_velocity.y = sinf(direction) * speed;
    return physics;
}

void play_saucer_warning_sound(Saucer *saucer, Audio *audio) {
    ALuint source = audio->sources[AUDIO_SOURCE_SAUCER_WARNING];
    if (!saucer->graphics.visible) {
        alSourcef(source, AL_GAIN, 0.0);
        return;
    }

    ALint state = 0;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING) {
        alSourcef(source, AL_GAIN, 1.0);
        return;
    }

    ALuint buffer = audio->buffers[AUDIO_BUFFER_WARNING_BIG_SAUCER];
    if (!saucer->big) {
        buffer = audio->buffers[AUDIO_BUFFER_WARNING_SMALL_SAUCER];
    }

    alSourceStop(source);
    alSourcei(source, AL_BUFFER, buffer);
    alSourcei(source, AL_LOOPING, AL_TRUE);
    alSourcef(source, AL_GAIN, 1.0);
    alSourcePlay(source);
}
