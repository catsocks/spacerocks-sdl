#pragma once

#include <SDL.h>
#include <physfs.h>
#include <stdbool.h>
#include <stdlib.h>

enum Image {
    IMAGE_COIN_BUTTON,
    IMAGE_ONE_PLAYER_BUTTON,
    IMAGE_TWO_PLAYER_BUTTON,
    IMAGE_THRUST_BUTTON,
    IMAGE_ROTATE_LEFT_BUTTON,
    IMAGE_ROTATE_RIGHT_BUTTON,
    IMAGE_FIRE_BUTTON,
    IMAGE_HYPERSPACE_BUTTON,
    NUM_IMAGES,
};

typedef enum Image Image;

enum Sound {
    SOUND_EXPLOSION_BIG,
    SOUND_EXPLOSION_MEDIUM,
    SOUND_EXPLOSION_SMALL,
    SOUND_PULSE_HIGH,
    SOUND_PULSE_LOW,
    SOUND_SAUCER_FIRE,
    SOUND_SHIP_FIRE,
    SOUND_SHIP_THRUST,
    SOUND_WARNING_BIG_SAUCER,
    SOUND_WARNING_SMALL_SAUCER,
    NUM_SOUNDS,
};

typedef enum Sound Sound;

struct Images {
    SDL_Surface *surfaces[NUM_IMAGES];
};

typedef struct Images Images;

struct SoundData {
    SDL_AudioSpec spec;
    uint8_t *buffer;
    uint32_t buffer_size;
};

typedef struct SoundData SoundData;

struct Sounds {
    SoundData data[NUM_SOUNDS];
};

typedef struct Sounds Sounds;

struct Asset {
    PHYSFS_file *physfs_file;
    uint8_t *buf;
    SDL_RWops *rw;
};

typedef struct Asset Asset;

bool init_images(Images *images);
bool init_sounds(Sounds *sounds);
void free_images(Images *images);
void free_sounds(Sounds *sounds);
bool load_asset(Asset *asset, const char *path);
void free_asset(Asset *asset);
