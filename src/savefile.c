#include "spacerocks/savefile.h"

bool read_savefile(Scoreboard *scoreboard, const char *path) {
    bool ok = false;
    char *str = NULL;

    PHYSFS_file *file = PHYSFS_openRead(path);
    if (file == NULL) {
        PHYSFS_ErrorCode err = PHYSFS_getLastErrorCode();
        if (err == PHYSFS_ERR_NOT_FOUND) {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Savefile not found");
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Couldn't open file \"%s\": %s", path,
                         PHYSFS_getErrorByCode(err));
        }
        goto done;
    }

    int64_t file_length = PHYSFS_fileLength(file);
    if (file_length == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't determine length of the savefile.");
        goto done;
    }

    str = calloc(file_length + 1, sizeof(char)); // + null term.
    if (str == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't allocate memory for reading the savefile.");
        goto done;
    }

    if (PHYSFS_readBytes(file, str, file_length) == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't read the contents of the savefile: %s",
                     PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        goto done;
    }

    if (!parse_scoreboard(scoreboard, str)) {
        goto done;
    }

    ok = true;
done:
    if (file != NULL && PHYSFS_close(file) == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't close PhysicsFS file: %s",
                     PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    }
    free(str);

    return ok;
}

bool write_savefile(Scoreboard *scoreboard, const char *path) {
    bool ok = false;

    PHYSFS_file *file = PHYSFS_openWrite(path);
    if (file == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't open file \"%s\": %s", path,
                     PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        goto done;
    }

    char str[SCOREBOARD_STR_SIZE] = {0};
    size_t str_length = 0;
    if (!stringify_scoreboard(scoreboard, str, sizeof(str), &str_length)) {
        goto done;
    }

    int64_t bytes_written = PHYSFS_writeBytes(file, str, str_length);
    if (bytes_written == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't write the contents of a file: %s",
                     PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        goto done;
    } else if ((size_t)bytes_written < str_length) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't completely write the contents of a file: %s",
                     PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    }

    ok = true;
done:
    if (file != NULL && PHYSFS_close(file) == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't close PhysicsFS file: %s",
                     PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    }

    // NOTE: It's not guaranteed that the filesystem will be synced before the
    // browser tab is closed.
#ifdef __EMSCRIPTEN__
    if (ok) {
        // clang-format off
        EM_ASM(
            FS.syncfs((err) => {
                if (err) {
                    console.error(err);
                } else {
                    console.debug('Persistent filesystem syncing is done');
                }
            });
        );
        // clang-format on
    }
#endif

    return ok;
}
