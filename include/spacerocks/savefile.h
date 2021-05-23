#pragma once

#include <physfs.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "spacerocks/scoreboard.h"

bool read_savefile(Scoreboard *scoreboard, const char *path);
bool write_savefile(Scoreboard *scoreboard, const char *path);
