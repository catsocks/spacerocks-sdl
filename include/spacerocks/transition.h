#pragma once

#include <SDL.h>
#include <stdbool.h>

#include "spacerocks/math.h"

struct Transition {
    float duration;
    bool started;
    bool reverse;
    uint32_t start_time;
    uint32_t end_time;
    float progress;
    bool done;
};

typedef struct Transition Transition;

Transition make_transition(float duration);
Transition start_transition(Transition trans, bool reverse);
Transition update_transition(Transition trans);
