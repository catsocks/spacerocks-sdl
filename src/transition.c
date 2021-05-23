#include "spacerocks/transition.h"

Transition make_transition(float duration) {
    return (Transition){
        .duration = duration,
    };
}

Transition start_transition(Transition trans, bool reverse) {
    trans.started = true;
    trans.reverse = reverse;
    trans.start_time = SDL_GetTicks();
    trans.end_time = SDL_GetTicks() + (trans.duration * 1000); // in ms
    trans.progress = (trans.reverse) ? 1.0f : 0.0f;
    trans.done = false;
    return trans;
}

Transition update_transition(Transition trans) {
    if (trans.done) {
        return trans;
    }

    trans.progress =
        normalize(SDL_GetTicks(), trans.start_time, trans.end_time);
    if (trans.reverse) {
        trans.progress = 1.0f - trans.progress;
    }
    trans.progress = clamp(trans.progress, 0.0f, 1.0f);

    if (trans.reverse) {
        trans.done = trans.progress == 0.0f;
    } else {
        trans.done = trans.progress == 1.0f;
    }
    return trans;
}
