#include "spacerocks/assets.h"

bool init_images(Images *images) {
    const char *images_path[NUM_IMAGES] = {
        [IMAGE_COIN_BUTTON] = "images/coin-button.bmp",
        [IMAGE_ONE_PLAYER_BUTTON] = "images/1p-button.bmp",
        [IMAGE_TWO_PLAYER_BUTTON] = "images/2p-button.bmp",
        [IMAGE_THRUST_BUTTON] = "images/thrust-button.bmp",
        [IMAGE_ROTATE_LEFT_BUTTON] = "images/rotate-left-button.bmp",
        [IMAGE_ROTATE_RIGHT_BUTTON] = "images/rotate-right-button.bmp",
        [IMAGE_FIRE_BUTTON] = "images/fire-button.bmp",
        [IMAGE_HYPERSPACE_BUTTON] = "images/hyperspace-button.bmp",
    };

    for (int i = 0; i < NUM_IMAGES; i++) {
        Asset asset = {0};
        if (!load_asset(&asset, images_path[i])) {
            return false;
        }

        if ((images->surfaces[i] = SDL_LoadBMP_RW(asset.rw, 0)) == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Couldn't load image: %s", SDL_GetError());
            free_asset(&asset);
            return false;
        }
        free_asset(&asset);
    }
    return true;
}

bool init_sounds(Sounds *sounds) {
    const char *sounds_path[NUM_SOUNDS] = {
        [SOUND_EXPLOSION_BIG] = "audio/explosion-big.wav",
        [SOUND_EXPLOSION_MEDIUM] = "audio/explosion-medium.wav",
        [SOUND_EXPLOSION_SMALL] = "audio/explosion-small.wav",
        [SOUND_PULSE_HIGH] = "audio/pulse-high.wav",
        [SOUND_PULSE_LOW] = "audio/pulse-low.wav",
        [SOUND_SAUCER_FIRE] = "audio/saucer-fire.wav",
        [SOUND_WARNING_BIG_SAUCER] = "audio/warning-big-saucer.wav",
        [SOUND_WARNING_SMALL_SAUCER] = "audio/warning-small-saucer.wav",
        [SOUND_SHIP_FIRE] = "audio/ship-fire.wav",
        [SOUND_SHIP_THRUST] = "audio/ship-thrust.wav",
    };

    for (int i = 0; i < NUM_SOUNDS; i++) {
        Asset asset = {0};
        if (!load_asset(&asset, sounds_path[i])) {
            continue;
        }

        if (SDL_LoadWAV_RW(asset.rw, 0, &sounds->data[i].spec,
                           &sounds->data[i].buffer,
                           &sounds->data[i].buffer_size) == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Couldn't load sound: %s", SDL_GetError());
            free_asset(&asset);
            return false;
        }

        free_asset(&asset);
    }
    return true;
}

void free_images(Images *images) {
    for (int i = 0; i < NUM_IMAGES; i++) {
        SDL_FreeSurface(images->surfaces[i]);
    }
}

void free_sounds(Sounds *sounds) {
    for (int i = 0; i < NUM_SOUNDS; i++) {
        SDL_FreeWAV(sounds->data[i].buffer);
    }
}

bool load_asset(Asset *asset, const char *path) {
    asset->physfs_file = PHYSFS_openRead(path);
    if (asset->physfs_file == NULL) {
        PHYSFS_ErrorCode code = PHYSFS_getLastErrorCode();
        if (code == PHYSFS_ERR_NOT_FOUND) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Couldn't open \"%s\", were the assets deleted?",
                         path);
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Couldn't open \"%s\": %s", path,
                         PHYSFS_getErrorByCode(code));
        }
        free_asset(asset);
        return false;
    }

    int64_t buf_length = PHYSFS_fileLength(asset->physfs_file);
    if (buf_length == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't determine length of a file.");
        free_asset(asset);
        return false;
    }

    asset->buf = calloc(buf_length, sizeof(uint8_t));
    if (asset->buf == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't allocate memory for reading a file.");
        free_asset(asset);
        return false;
    }

    // NOTE: Is it worth handling when the return value is a positive integer
    // but it less less than buf_length?
    if (PHYSFS_readBytes(asset->physfs_file, asset->buf, buf_length) == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't read the contents of a file: %s",
                     PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        free_asset(asset);
        return false;
    }

    asset->rw = SDL_RWFromMem(asset->buf, buf_length);
    if (asset->rw == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't read a file from memory: %s", SDL_GetError());
        free_asset(asset);
        return false;
    }

    return true;
}

void free_asset(Asset *asset) {
    if (asset->rw != NULL && SDL_RWclose(asset->rw) == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't close RWops file: %s", SDL_GetError());
    }
    free(asset->buf);
    if (asset->physfs_file != NULL && PHYSFS_close(asset->physfs_file) == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't close PhysicsFS file: %s",
                     PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    }
}
