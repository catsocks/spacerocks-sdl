#include <AL/al.h>
#include <AL/alc.h>
#include <SDL.h>
#include <physfs.h>
#include <stdbool.h>
#include <time.h>

#ifdef _WIN32
#include <shellscalingapi.h>
#include <windows.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "spacerocks/assets.h"
#include "spacerocks/debug.h"
#include "spacerocks/game.h"

struct Context {
    Game game;
    bool quit_requested;
};

typedef struct Context Context;

void main_loop(void *arg);
void setup_dpi_awareness(void);

int main(int argc, char *argv[]) {
    // Suppress unused variable warnings, SDL requires that main accept argc and
    // argv when using MSVC or MinGW.
    (void)argc;
    (void)argv;

    if (DEBUG_SPACEROCKS) {
        SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION,
                           SDL_LOG_PRIORITY_DEBUG);
    }

    srand(time(NULL));

    if (argc > 1) {
        PHYSFS_init(argv[0]);
    } else {
        PHYSFS_init(NULL);
    }

    const char *savefolder = ".";
#ifdef __EMSCRIPTEN__
    savefolder = "savefolder";
#endif

    if (PHYSFS_mount("assets", "", 0) == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't mount assets: %s",
                     PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    }

    if (PHYSFS_mount(savefolder, "", 0) == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't mount directory for savefile: %s",
                     PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    }

    if (PHYSFS_setWriteDir(savefolder) == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't set directory for writing savefile: %s",
                     PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    }

    ALCdevice *device = alcOpenDevice(NULL); // open the preferred device
    ALCcontext *context = NULL;
    if (device == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't open an audio device: %s",
                     alGetString(alGetError()));
    } else {
        if ((context = alcCreateContext(device, NULL)) == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Couldn't create an OpenAL context: %s",
                         alGetString(alGetError()));
        } else {
            alcMakeContextCurrent(context);
            SDL_assert(alGetError() == AL_NO_ERROR);
        }
    }

    setup_dpi_awareness();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't initialize SDL: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    // A safe low resolution to use should SDL_GetDisplayUsableBounds fail.
    int window_width = 400;
    int window_height = 300;
    SDL_Rect usable_bounds = {0};
    // Assume SDL_WINDOWPOS_UNDEFINED will result in display 0 being used in
    // SDL_CreateWindow.
    // TODO: Either ensure the window will be created in the display 0 or
    // adjust the window size after the window has been created.
    if (SDL_GetDisplayUsableBounds(0, &usable_bounds) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't get the usable bounds of a display: %s",
                     SDL_GetError());
    } else {
        float scale = fminf((usable_bounds.w * 0.75f) / (float)window_width,
                            (usable_bounds.h * 0.75f) / (float)window_height);
        window_width *= scale;
        window_height *= scale;
    }

    // Create a hidden window so it may only be shown after the game is done
    // initializing.
    SDL_Window *window = SDL_CreateWindow(
        "Spacerocks", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        window_width, window_height,
        SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s",
                     SDL_GetError());
        return EXIT_FAILURE;
    }

    Asset window_icon = {0};
    if (load_asset(&window_icon, "images/window-icon.bmp")) {
        SDL_Surface *surf = SDL_LoadBMP_RW(window_icon.rw, 0);
        if (surf == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Couldn't load window icon: %s", SDL_GetError());
        } else {
            SDL_SetWindowIcon(window, surf);
            SDL_FreeSurface(surf);
        }
        free_asset(&window_icon);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    SDL_Renderer *renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't create renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    Context ctx = {0};
    bool ok = init_game(&ctx.game, window, renderer);
    if (!ok) {
        goto done;
    }

    SDL_ShowWindow(window);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(main_loop, &ctx, 0, 1);
#else
    while (!ctx.quit_requested) {
        main_loop(&ctx);
    }
#endif

done:
    deinit_game(&ctx.game);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    alcDestroyContext(context);
    alcCloseDevice(device);

    PHYSFS_deinit();

    if (ok) {
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

void main_loop(void *arg) {
    Context *ctx = arg;

    Game *game = &ctx->game;

    SDL_Event event = {0};
    while (SDL_PollEvent(&event) == 1) {
        switch (event.type) {
        case SDL_QUIT:
            ctx->quit_requested = true;
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            check_keyboard_event(game, &event);
            break;
        case SDL_FINGERDOWN:
        case SDL_FINGERUP:
        case SDL_FINGERMOTION:
            check_finger_event(game, &event);
            break;
        case SDL_CONTROLLERDEVICEADDED:
        case SDL_CONTROLLERDEVICEREMOVED:
            check_controller_device_event(game, &event);
            break;
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            check_controller_button_event(game, &event);
            break;
        }
    }

    check_game_input(game);
    update_game(game);
    render_game(game);
}

// TODO: Replace this function with a Windows application manifest containing
// the dpiAware and dpiAwareness settings.
// NOTE: I couldn't get an application manifest to work with MinGW. It was
// embedded in the executable but ignored.
void setup_dpi_awareness() {
#ifdef _WIN32
    void *lib = SDL_LoadObject("shcore.dll");
    if (lib == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't load shared library shcore.dll: %s",
                     SDL_GetError());
        return;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    HRESULT(WINAPI * fn)
    (PROCESS_DPI_AWARENESS dpiAwareness) = (HRESULT(WINAPI *)(
        PROCESS_DPI_AWARENESS))SDL_LoadFunction(lib, "SetProcessDpiAwareness");
#pragma GCC diagnostic pop

    if (fn == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't load the address of SetProcessDpiAwareness in "
                     "shcore.dll: %s",
                     SDL_GetError());
        return;
    }

    if (fn(PROCESS_PER_MONITOR_DPI_AWARE) != S_OK) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't set DPI awareness");
    }
#endif
}
