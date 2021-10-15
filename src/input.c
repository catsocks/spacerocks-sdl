#include "spacerocks/input.h"

static TouchButton make_touch_button(int x, int y, float width);

static const Button SCANCODE_BUTTON_MAP[SDL_NUM_SCANCODES] = {
    [SDL_SCANCODE_C] = BUTTON_COIN,
    [SDL_SCANCODE_1] = BUTTON_ONE_PLAYER,
    [SDL_SCANCODE_2] = BUTTON_TWO_PLAYERS,
    [SDL_SCANCODE_W] = BUTTON_THRUST,
    [SDL_SCANCODE_A] = BUTTON_ROTATE_LEFT,
    [SDL_SCANCODE_D] = BUTTON_ROTATE_RIGHT,
    [SDL_SCANCODE_J] = BUTTON_FIRE,
    [SDL_SCANCODE_K] = BUTTON_HYPERSPACE,
    [SDL_SCANCODE_F11] = BUTTON_TOGGLE_FULLSCREEN,
    [SDL_SCANCODE_M] = BUTTON_TOGGLE_MUTE,
    [SDL_SCANCODE_F10] = BUTTON_NEXT_LANG,
    [SDL_SCANCODE_F9] = BUTTON_PREV_LANG,
    [SDL_SCANCODE_3] = BUTTON_ADD_SHIP,
    [SDL_SCANCODE_4] = BUTTON_ADD_SCORE,
    [SDL_SCANCODE_5] = BUTTON_DESTROY_SHIP,
    [SDL_SCANCODE_6] = BUTTON_DESTROY_ROCKS,
    [SDL_SCANCODE_7] = BUTTON_DESTROY_SAUCER,
    [SDL_SCANCODE_8] = BUTTON_GAME_OVER,
};

static const Button CONTROLLER_BUTTON_BUTTON_MAP[SDL_CONTROLLER_BUTTON_MAX] = {
    [SDL_CONTROLLER_BUTTON_Y] = BUTTON_COIN,
    [SDL_CONTROLLER_BUTTON_LEFTSHOULDER] = BUTTON_ONE_PLAYER,
    [SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = BUTTON_TWO_PLAYERS,
    [SDL_CONTROLLER_BUTTON_DPAD_UP] = BUTTON_THRUST,
    [SDL_CONTROLLER_BUTTON_DPAD_LEFT] = BUTTON_ROTATE_LEFT,
    [SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = BUTTON_ROTATE_RIGHT,
    [SDL_CONTROLLER_BUTTON_A] = BUTTON_FIRE,
    [SDL_CONTROLLER_BUTTON_B] = BUTTON_HYPERSPACE,
};

// NOTE: Maybe store touch button positions as floats normalized to [0, 1]?
Input make_input() {
    TouchInput touch = {
        .buttons =
            {
                [BUTTON_COIN] = make_touch_button(54, 561, 120),
                [BUTTON_ONE_PLAYER] = make_touch_button(800, 420, 120),
                [BUTTON_TWO_PLAYERS] = make_touch_button(800, 561, 120),
                [BUTTON_ROTATE_LEFT] = make_touch_button(54, 561, 120),
                [BUTTON_ROTATE_RIGHT] = make_touch_button(220, 561, 120),
                [BUTTON_THRUST] = make_touch_button(137, 420, 120),
                [BUTTON_FIRE] = make_touch_button(800, 561, 120),
                [BUTTON_HYPERSPACE] = make_touch_button(800, 420, 120),
            },
    };
    return (Input){
        .touch = touch,
    };
}

static TouchButton make_touch_button(int x, int y, float width) {
    return (TouchButton){
        .rect =
            {
                .x = x,
                .y = y,
                .w = width,
                .h = width,
            },
        .finger_id = -1,
    };
}

// Should be called at the end of every game frame.
void update_input_buttons_state(Input *input) {
    // Change buttons in the state BUTTON_STATE_JUST_PRESSED to
    // BUTTON_STATE_PRESSED.
    for (int b = 0; b < NUM_BUTTONS; b++) {
        if (input->keyboard_state[b] == BUTTON_STATE_JUST_PRESSED) {
            input->keyboard_state[b] = BUTTON_STATE_PRESSED;
        }

        for (int c = 0; c < NUM_CONTROLLERS; c++) {
            if (input->controller.buttons[c][b] == BUTTON_STATE_JUST_PRESSED) {
                input->controller.buttons[c][b] = BUTTON_STATE_PRESSED;
            }
        }

        if (input->touch.buttons[b].state == BUTTON_STATE_JUST_PRESSED) {
            input->touch.buttons[b].state = BUTTON_STATE_PRESSED;
        }
    }

    // Change buttons in the state BUTTON_STATE_JUST_RELEASED to
    // BUTTON_STATE_RELEASED.
    for (int b = 0; b < NUM_BUTTONS; b++) {
        if (input->keyboard_state[b] == BUTTON_STATE_JUST_RELEASED) {
            input->keyboard_state[b] = BUTTON_STATE_RELEASED;
        }

        for (int c = 0; c < NUM_CONTROLLERS; c++) {
            if (input->controller.buttons[c][b] == BUTTON_STATE_JUST_RELEASED) {
                input->controller.buttons[c][b] = BUTTON_STATE_RELEASED;
            }
        }

        if (input->touch.buttons[b].state == BUTTON_STATE_JUST_RELEASED) {
            input->touch.buttons[b].state = BUTTON_STATE_RELEASED;
        }
    }
}

void input_check_keyboard_event(Input *input, SDL_Event *event) {
    Button b = SCANCODE_BUTTON_MAP[event->key.keysym.scancode];
    if (b == BUTTON_NONE) {
        return;
    }
    if (event->type == SDL_KEYDOWN) {
        // Prevent switching a button from BUTTON_STATE_PRESSED to
        // BUTTON_STATE_JUST_PRESSED on repeated SDL_KEYDOWN events.
        if (input->keyboard_state[b] != BUTTON_STATE_PRESSED) {
            input->keyboard_state[b] = BUTTON_STATE_JUST_PRESSED;
        }
    } else {
        input->keyboard_state[b] = BUTTON_STATE_JUST_RELEASED;
    }
}

void input_check_controller_device_event(Input *input, SDL_Event *event) {
    if (event->type == SDL_CONTROLLERDEVICEADDED) {
        if (input->controller.devices[CONTROLLER_1] == NULL) {
            input->controller.devices[CONTROLLER_1] =
                SDL_GameControllerOpen(event->cdevice.which);
        } else if (input->controller.devices[CONTROLLER_2] == NULL) {
            input->controller.devices[CONTROLLER_2] =
                SDL_GameControllerOpen(event->cdevice.which);
        }
    } else {
        SDL_GameController *ctrl =
            SDL_GameControllerFromInstanceID(event->cdevice.which);
        if (input->controller.devices[CONTROLLER_1] == ctrl) {
            input->controller.devices[CONTROLLER_1] = NULL;
        } else if (input->controller.devices[CONTROLLER_2] == ctrl) {
            input->controller.devices[CONTROLLER_2] = NULL;
        }
        SDL_GameControllerClose(ctrl);
    }
}

void input_check_controller_button_event(Input *input, SDL_Event *event) {
    SDL_GameController *device =
        SDL_GameControllerFromInstanceID(event->cbutton.which);

    Controller c = CONTROLLER_1;
    if (device == input->controller.devices[CONTROLLER_2]) {
        c = CONTROLLER_2;
    } else if (device != input->controller.devices[CONTROLLER_1]) {
        return;
    }

    Button b = CONTROLLER_BUTTON_BUTTON_MAP[event->cbutton.button];
    if (b == BUTTON_NONE) {
        return;
    }
    if (event->cbutton.state == SDL_PRESSED) {
        input->controller.buttons[c][b] = BUTTON_STATE_JUST_PRESSED;
    } else {
        input->controller.buttons[c][b] = BUTTON_STATE_JUST_RELEASED;
    }
}

void input_check_finger_event(Input *input, Vec2D logical_size,
                              SDL_Event *event) {
    // First check the virtual touch buttons.
    Vec2D point = {event->tfinger.x * logical_size.x,
                   event->tfinger.y * logical_size.y};
    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (!input->touch.buttons[i].enabled) {
            input->touch.buttons[i].state = BUTTON_STATE_JUST_RELEASED;
            continue;
        }

        if (event->type == SDL_FINGERUP) {
            if (input->touch.buttons[i].finger_id != event->tfinger.fingerId) {
                continue;
            }
            input->touch.buttons[i].state = BUTTON_STATE_JUST_RELEASED;
            input->touch.buttons[i].finger_id = -1;
            continue;
        }

        if (SDL_PointInRect(&point, &input->touch.buttons[i].rect)) {
            if (event->type == SDL_FINGERDOWN) {
                input->touch.buttons[i].finger_id = event->tfinger.fingerId;
                input->touch.buttons[i].state = BUTTON_STATE_JUST_PRESSED;
            }
        } else {
            if (input->touch.buttons[i].finger_id != event->tfinger.fingerId) {
                continue;
            }
            input->touch.buttons[i].state = BUTTON_STATE_RELEASED;
            input->touch.buttons[i].finger_id = -1;
        }
    }

    // Then check the touch gestures, which are limited to the states
    // BUTTON_STATE_JUST_PRESSED and BUTTON_STATE_RELEASED.
    if (event->type != SDL_FINGERUP) {
        if (event->tfinger.dy > 0.1f) {
            input->touch.buttons[BUTTON_DISABLE_FULLSCREEN].state =
                BUTTON_STATE_JUST_PRESSED;
        } else if (event->tfinger.dy < -0.1f) {
            input->touch.buttons[BUTTON_ENABLE_FULLSCREEN].state =
                BUTTON_STATE_JUST_PRESSED;
        }
    } else {
        input->touch.buttons[BUTTON_ENABLE_FULLSCREEN].state =
            BUTTON_STATE_RELEASED;
        input->touch.buttons[BUTTON_DISABLE_FULLSCREEN].state =
            BUTTON_STATE_RELEASED;
    }
}

// Return true if the given button is pressed.
// NOTE: There are too many if statements here, what's a better way to do this?
bool is_button_pressed_controller(Input *input, Button button,
                                  Controller preferred_ctrl) {
    Controller ctrl = preferred_ctrl;
    if (input->controller.devices[preferred_ctrl] == NULL) {
        ctrl = (preferred_ctrl == CONTROLLER_1) ? CONTROLLER_2 : CONTROLLER_1;
    }

    return input->keyboard_state[button] == BUTTON_STATE_JUST_PRESSED ||
           input->keyboard_state[button] == BUTTON_STATE_PRESSED ||
           input->controller.buttons[ctrl][button] ==
               BUTTON_STATE_JUST_PRESSED ||
           input->controller.buttons[ctrl][button] == BUTTON_STATE_PRESSED ||
           input->touch.buttons[button].state == BUTTON_STATE_JUST_PRESSED ||
           input->touch.buttons[button].state == BUTTON_STATE_PRESSED;
}

// Return true the first frame the given button is registered as being pressed.
bool is_button_just_pressed(Input *input, Button button) {
    return input->keyboard_state[button] == BUTTON_STATE_JUST_PRESSED ||
           input->controller.buttons[CONTROLLER_1][button] ==
               BUTTON_STATE_JUST_PRESSED ||
           input->controller.buttons[CONTROLLER_2][button] ==
               BUTTON_STATE_JUST_PRESSED ||
           input->touch.buttons[button].state == BUTTON_STATE_JUST_PRESSED;
}

// Return true the first frame the given button, preferably in the given
// controller, is registered as being pressed.
bool is_button_just_pressed_controller(Input *input, Button button,
                                       Controller preferred_ctrl) {
    Controller ctrl = preferred_ctrl;
    if (input->controller.devices[preferred_ctrl] == NULL) {
        ctrl = (preferred_ctrl == CONTROLLER_1) ? CONTROLLER_2 : CONTROLLER_1;
    }

    return input->keyboard_state[button] == BUTTON_STATE_JUST_PRESSED ||
           input->controller.buttons[ctrl][button] ==
               BUTTON_STATE_JUST_PRESSED ||
           input->touch.buttons[button].state == BUTTON_STATE_JUST_PRESSED;
}

// Return true the first frame the given button is registered as released after
// having been pressed.
bool is_button_just_released(Input *input, Button button) {
    return input->keyboard_state[button] == BUTTON_STATE_JUST_RELEASED ||
           input->controller.buttons[CONTROLLER_1][button] ==
               BUTTON_STATE_JUST_RELEASED ||
           input->controller.buttons[CONTROLLER_2][button] ==
               BUTTON_STATE_JUST_RELEASED ||
           input->touch.buttons[button].state == BUTTON_STATE_JUST_RELEASED;
}
