#include "spacerocks/math.h"

const double pi = 3.14159265358979323846;

int wrap_around(int v, int max) {
    return (max + (v % max)) % max;
}

float fwrap_around(float v, float max) {
    return fmodf(max + fmodf(v, max), max);
}

// Return a random int between min and max (inclusive).
int rand_range(int min, int max) {
    return min + (rand() / ((RAND_MAX / (max - min + 1)) + 1));
}

// Return a random float between min and max (inclusive).
float frand_range(float min, float max) {
    return min + (rand() / ((double)RAND_MAX / (max - min)));
}

bool rand_bool() {
    return rand_range(0, 1) == 1;
}

bool percent_chance(int percent) {
    return rand_range(0, 100) <= percent;
}

float clamp(float v, float min, float max) {
    return fmaxf(min, fminf(max, v));
}

float normalize(float v, float min, float max) {
    if (min - max == 0.0f) {
        return 0.0f;
    }
    return (v - min) / (max - min);
}

float length_vec2df(Vec2Df v) {
    return sqrtf((v.y * v.y) + (v.x * v.x));
}

float atan2_vec2df(Vec2Df v) {
    return atan2f(v.y, v.x);
}

float distance_vec2df(Vec2Df v1, Vec2Df v2) {
    v1.x -= v2.x;
    v1.y -= v2.y;
    return length_vec2df(v1);
}

Vec2Df rotate_vec2df(Vec2Df v, float angle) {
    float s = sinf(angle);
    float c = cosf(angle);

    float new_x = (v.x * c) - (v.y * s);
    float new_y = (v.x * s) + (v.y * c);

    v.x = new_x;
    v.y = new_y;

    return v;
}

Vec2Df wrap_around_vec2df(Vec2Df v, Vec2D max) {
    v.x = fwrap_around(v.x, max.x);
    v.y = fwrap_around(v.y, max.y);
    return v;
}
