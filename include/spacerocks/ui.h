#pragma once

#include <stdbool.h>

#include "spacerocks/assets.h"
#include "spacerocks/debug.h"
#include "spacerocks/font.h"
#include "spacerocks/input.h"
#include "spacerocks/language.h"
#include "spacerocks/math.h"
#include "spacerocks/renderer.h"
#include "spacerocks/scoreboard.h"
#include "spacerocks/ship.h"
#include "spacerocks/transition.h"
#include "spacerocks/util.h"

struct UIGameState {
    Language lang;
    bool two_players;
    int current_player_no;
    int coins;
    int p1_ships;
    int p2_ships;
    int p1_score;
    int p2_score;
    char initials[INITIALS_STR_SIZE];
    Scoreboard scoreboard;
    uint32_t last_touch_input;
};

typedef struct UIGameState UIGameState;

struct UIText {
    Vec2Df position;
    Vec2Df size;
    float height;
    const char *str;
    bool visible;
    SDL_Color color;
};

typedef struct UIText UIText;

struct UITouchButton {
    Button button;
    Rect2D rect;
    SDL_Texture *texture;
    bool visible;
    float transparency;
};

typedef struct UITouchButton UITouchButton;

enum UITextEnum {
    UI_TEXT_COPYRIGHT,
    UI_TEXT_P1_SCORE,
    UI_TEXT_P2_SCORE,
    UI_TEXT_HIGHEST_SCORE,
    UI_TEXT_COIN_PLAY,
    UI_TEXT_PUSH_START,
    UI_TEXT_PLAYER_NO_NO,
    UI_TEXT_PLAYER_NO,
    UI_TEXT_GAME_OVER,
    UI_TEXT_ENTER_INITIALS,
    UI_TEXT_INITIALS,
    UI_TEXT_HIGHSCORES,
    UI_TEXT_SCOREBOARD,
    UI_TEXT_LANGUAGE,
    NUM_UI_TEXTS,
};

typedef enum UITextEnum UITextEnum;

enum UITouchButtonEnum {
    UI_TOUCH_BUTTON_COIN,
    UI_TOUCH_BUTTON_ONE_PLAYER,
    UI_TOUCH_BUTTON_TWO_PLAYERS,
    UI_TOUCH_BUTTON_ROTATE_LEFT,
    UI_TOUCH_BUTTON_ROTATE_RIGHT,
    UI_TOUCH_BUTTON_THRUST,
    UI_TOUCH_BUTTON_FIRE,
    UI_TOUCH_BUTTON_HYPERSPACE,
    NUM_UI_TOUCH_BUTTONS,
};

typedef enum UITouchButtonEnum UITouchButtonEnum;

struct UI {
    UIGameState game_state;
    Vec2D size;
    Vec2Df padding;
    Images images;
    Transition touch_buttons_transition;
    UITouchButton touch_buttons[NUM_UI_TOUCH_BUTTONS];
    UIText texts[NUM_UI_TEXTS];
    bool touch_buttons_shown;
    char p1_score_str[SCORE_STR_SIZE];
    char p2_score_str[SCORE_STR_SIZE];
    char highest_score_str[SCORE_STR_SIZE];
    char scoreboard_str[SCOREBOARD_STR_SIZE];
    char player_str[20];
    bool render_p1_ships;
    bool render_p2_ships;
    bool flash_player_score;
    uint32_t push_start_text_timer;
    uint32_t score_no_text_timer;
    uint32_t hide_language_text_time;
};

typedef struct UI UI;

bool init_ui(UI *ui, RendererWrapper *renderer, Language lang,
             TouchInput *touch_input);
void set_ui_localized_texts(UI *ui);
void update_ui(UI *ui, TouchInput *touch_input);
void show_current_language_name(UI *ui);
void show_ui_touch_controls(UI *ui);
void init_ui_game_attract_state(UI *ui);
void init_ui_scoreboard_attract_state(UI *ui);
void init_ui_turn_start_state(UI *ui);
void init_ui_play_state(UI *ui);
void init_ui_game_over_state(UI *ui);
void init_ui_enter_initials_state(UI *ui);
void render_ui(UI *ui, RendererWrapper *renderer);
void free_ui(UI *ui);
