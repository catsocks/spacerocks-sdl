#pragma once

#include <SDL.h>
#include <stdbool.h>

#include "spacerocks/math.h"

enum ButtonState {
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_JUST_PRESSED,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_JUST_RELEASED,
};

typedef enum ButtonState ButtonState;

// NOTE: It'd be nice to have BUTTON_PAUSE, BUTTON_UNPAUSE and BUTTON_RESET.
enum Button {
    BUTTON_NONE,
    BUTTON_COIN,
    BUTTON_ONE_PLAYER,
    BUTTON_TWO_PLAYERS,
    BUTTON_THRUST,
    BUTTON_ROTATE_LEFT,
    BUTTON_ROTATE_RIGHT,
    BUTTON_FIRE,
    BUTTON_HYPERSPACE,
    BUTTON_TOGGLE_FULLSCREEN,
    BUTTON_ENABLE_FULLSCREEN,
    BUTTON_DISABLE_FULLSCREEN,
    BUTTON_TOGGLE_MUTE,
    BUTTON_NEXT_LANG,
    BUTTON_PREV_LANG,
    // The following are used for debugging.
    BUTTON_ADD_SHIP,
    BUTTON_ADD_SCORE,
    BUTTON_DESTROY_SHIP,
    BUTTON_DESTROY_ROCKS,
    BUTTON_DESTROY_SAUCER,
    BUTTON_GAME_OVER,
    NUM_BUTTONS,
};

typedef enum Button Button;

enum Controller {
    CONTROLLER_1,
    CONTROLLER_2,
    NUM_CONTROLLERS,
};

typedef enum Controller Controller;

struct ControllerInput {
    SDL_GameController *devices[NUM_CONTROLLERS];
    ButtonState buttons[NUM_CONTROLLERS][NUM_BUTTONS];
};

typedef struct ControllerInput ControllerInput;

struct TouchButton {
    Rect2D rect;
    ButtonState state;
    SDL_FingerID finger_id;
    bool enabled;
};

typedef struct TouchButton TouchButton;

struct TouchInput {
    TouchButton buttons[NUM_BUTTONS];
};

typedef struct TouchInput TouchInput;

struct Input {
    ButtonState keyboard_state[NUM_BUTTONS];
    ControllerInput controller;
    TouchInput touch;
};

typedef struct Input Input;

Input make_input();
void input_down_to_held(Input *input);
void input_released_to_up(Input *input);
void input_check_keyboard_event(Input *input, SDL_Event *event);
void input_check_finger_event(Input *input, Vec2D logical_size,
                              SDL_Event *event);
void input_check_controller_device_event(Input *input, SDL_Event *event);
void input_check_controller_button_event(Input *input, SDL_Event *event);
bool is_button_just_pressed(Input *input, Button button);
bool is_button_pressed_controller(Input *input, Button button,
                                  Controller preferred_ctrl);
bool is_button_just_pressed_controller(Input *input, Button button,
                                       Controller preferred_ctrl);
bool is_button_just_released(Input *input, Button button);
