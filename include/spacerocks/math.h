#pragma once

#include <SDL.h>
#include <stdbool.h>

typedef SDL_Point Vec2D;
typedef SDL_FPoint Vec2Df;
typedef SDL_Rect Rect2D;
typedef SDL_FRect Rect2Df;

extern const double pi;

int wrap_around(int val, int max);
float fwrap_around(float v, float max);
int rand_range(int min, int max);
float frand_range(float min, float max);
bool rand_bool();
bool percent_chance(int percent);
float clamp(float v, float min, float max);
float normalize(float v, float min, float max);
float length_vec2df(Vec2Df v);
float atan2_vec2df(Vec2Df v);
float distance_vec2df(Vec2Df v1, Vec2Df v2);
Vec2Df rotate_vec2df(Vec2Df v, float angle);
Vec2Df wrap_around_vec2df(Vec2Df v, Vec2D max);
