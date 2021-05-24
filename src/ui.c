#include "spacerocks/ui.h"

// Just for the aesthetics of it, feel free to change to whatever you'd like.
static const char *const copyright_str = "©2021 CATSOCKS";

static const SDL_Color normal_color = {150, 150, 150, 255};
static const SDL_Color highlight_color = {210, 210, 210, 255};

static void init_ui_static_texts(UI *ui);
static void init_ui_localized_texts(UI *ui);
static void init_ui_dynamic_texts(UI *ui);
static void set_ui_dynamic_texts(UI *ui);
static void check_snprintf_return_value(int rv, size_t output_str_size);
static void init_ui_touch_buttons(UI *ui, RendererWrapper *renderer,
                                  TouchInput *touch_input);
static UIText make_ui_text(float height, const char *str);
static UITouchButton make_ui_touch_button(RendererWrapper *renderer,
                                          TouchInput *input, Images *images,
                                          Button button);
static UITouchButton update_ui_touch_button(UITouchButton touch_button,
                                            TouchInput *input,
                                            Transition trans);
static void init_attract_state(UI *ui);
static void render_ui_text(RendererWrapper *renderer, UIText text);
static void render_ui_touch_button(RendererWrapper *renderer,
                                   UITouchButton button);
static void render_players_ships(RendererWrapper *renderer, UI *ui);
static void render_ships(RendererWrapper *renderer, Vec2Df position,
                         bool grows_right, int ships);

bool init_ui(UI *ui, RendererWrapper *renderer, Language lang,
             TouchInput *touch_input) {
    ui->game_state.lang = lang;
    ui->size = renderer->logical_size;
    ui->padding = (Vec2Df){0.0f, 10.0f};

    if (!init_images(&ui->images)) {
        return false;
    }

    ui->touch_buttons_transition = make_transition(0.1f);

    init_ui_static_texts(ui);
    init_ui_localized_texts(ui);
    init_ui_dynamic_texts(ui);

    init_ui_touch_buttons(ui, renderer, touch_input);

    if (DEBUG_TOUCH_BUTTONS) {
        show_ui_touch_controls(ui);
    }

    return true;
}

static void init_ui_static_texts(UI *ui) {
    // Will always be visible.
    UIText text = make_ui_text(10, copyright_str);
    text.position.x = (ui->size.x - text.size.x) / 2.0f;
    text.position.y = ui->size.y - text.size.y - ui->padding.y;
    text.visible = true;
    ui->texts[UI_TEXT_COPYRIGHT] = text;
}

static UIText make_ui_text(float height, const char *str) {
    return (UIText){
        .height = height,
        .str = str,
        .size = get_rendered_text_size(height, str),
        .color = normal_color,
    };
}

static void init_ui_localized_texts(UI *ui) {
    ui->texts[UI_TEXT_COIN_PLAY] = make_ui_text(21, NULL);
    ui->texts[UI_TEXT_PUSH_START] = make_ui_text(21, NULL);
    ui->texts[UI_TEXT_PUSH_START].color.a = 0;
    ui->texts[UI_TEXT_GAME_OVER] = make_ui_text(21, NULL);
    ui->texts[UI_TEXT_ENTER_INITIALS] = make_ui_text(21, NULL);
    ui->texts[UI_TEXT_HIGHSCORES] = make_ui_text(21, NULL);
    ui->texts[UI_TEXT_LANGUAGE] = make_ui_text(12, NULL);

    set_ui_localized_texts(ui);
}

void set_ui_localized_texts(UI *ui) {
    UIText *text = &ui->texts[UI_TEXT_COIN_PLAY];
    text->str = langs_strings[ui->game_state.lang][STRING_COIN_PLAY];
    text->size = get_rendered_text_size(text->height, text->str);
    text->position.x = (ui->size.x - text->size.x) / 2.0f;
    text->position.y = ui->size.y - 130.0f;

    text = &ui->texts[UI_TEXT_PUSH_START];
    text->str = langs_strings[ui->game_state.lang][STRING_PUSH_START];
    text->size = get_rendered_text_size(text->height, text->str);
    text->position.x = (ui->size.x - text->size.x) / 2.0f;
    text->position.y = 80.0f;

    text = &ui->texts[UI_TEXT_GAME_OVER];
    text->str = langs_strings[ui->game_state.lang][STRING_GAME_OVER];
    text->size = get_rendered_text_size(text->height, text->str);
    text->position.x = (ui->size.x - text->size.x) / 2.0f;
    text->position.y = 180.0f;

    text = &ui->texts[UI_TEXT_ENTER_INITIALS];
    text->str = langs_strings[ui->game_state.lang][STRING_ENTER_INITIALS];
    text->size = get_rendered_text_size(text->height, text->str);
    text->position.x = (ui->size.x - text->size.x) / 2.0f;
    text->position.y = 180.0f;

    text = &ui->texts[UI_TEXT_HIGHSCORES];
    text->str = langs_strings[ui->game_state.lang][STRING_HIGHSCORES];
    text->size = get_rendered_text_size(text->height, text->str);
    text->position.x = (ui->size.x - text->size.x) / 2.0f;
    text->position.y = 130.0f;

    text = &ui->texts[UI_TEXT_LANGUAGE];
    text->str = langs_strings[ui->game_state.lang][STRING_LANGUAGE];
    text->size = get_rendered_text_size(text->height, text->str);
    text->position.x = (ui->size.x - text->size.x) / 2.0f;
    text->position.y = ui->size.y - 180.0f;
}

static void init_ui_dynamic_texts(UI *ui) {
    ui->texts[UI_TEXT_P1_SCORE] = make_ui_text(20, ui->p1_score_str);
    ui->texts[UI_TEXT_P2_SCORE] = make_ui_text(20, ui->p2_score_str);
    ui->texts[UI_TEXT_HIGHEST_SCORE] = make_ui_text(12, ui->highest_score_str);
    ui->texts[UI_TEXT_SCOREBOARD] = make_ui_text(21, ui->scoreboard_str);
    ui->texts[UI_TEXT_PLAYER_NO] = make_ui_text(21, ui->player_str);
    ui->texts[UI_TEXT_INITIALS] = make_ui_text(40, ui->game_state.initials);

    // These will always be visible.
    ui->texts[UI_TEXT_P1_SCORE].visible = true;
    ui->texts[UI_TEXT_HIGHEST_SCORE].visible = true;

    set_ui_dynamic_texts(ui);
}

static void set_ui_dynamic_texts(UI *ui) {
    if (!stringify_scoreboard(&ui->game_state.scoreboard, ui->scoreboard_str,
                              sizeof(ui->scoreboard_str), NULL)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't convert the scoreboard to a string.");
    }

    int rv = snprintf(ui->p1_score_str, sizeof(ui->p1_score_str), "%02d",
                      ui->game_state.p1_score);
    check_snprintf_return_value(rv, sizeof(ui->p1_score_str));
    rv = snprintf(ui->p2_score_str, sizeof(ui->p2_score_str), "%02d",
                  ui->game_state.p2_score);
    check_snprintf_return_value(rv, sizeof(ui->p2_score_str));
    rv = snprintf(ui->highest_score_str, sizeof(ui->highest_score_str), "%02d",
                  ui->game_state.scoreboard.highest_score);
    check_snprintf_return_value(rv, sizeof(ui->highest_score_str));
    rv = snprintf(ui->player_str, sizeof(ui->player_str), "%s %d",
                  langs_strings[ui->game_state.lang][STRING_PLAYER],
                  ui->game_state.current_player_no);
    check_snprintf_return_value(rv, sizeof(ui->player_str));

    UIText *text = &ui->texts[UI_TEXT_P1_SCORE];
    text->size = get_rendered_text_size(text->height, text->str);
    text->position.x = 160.0f;
    text->position.y = ui->padding.y;

    text = &ui->texts[UI_TEXT_P2_SCORE];
    text->size = get_rendered_text_size(text->height, text->str);
    text->position.x = ui->size.x - text->size.x - 160.0f;
    text->position.y = ui->padding.y;

    text = &ui->texts[UI_TEXT_HIGHEST_SCORE];
    text->size = get_rendered_text_size(text->height, text->str);
    text->position.x = (ui->size.x - text->size.x) / 2.0f;
    text->position.y =
        ui->padding.y + ui->texts[UI_TEXT_P1_SCORE].size.y - text->size.y;

    text = &ui->texts[UI_TEXT_SCOREBOARD];
    text->size = get_rendered_text_size(text->height, text->str);
    text->position.x = (ui->size.x - text->size.x) / 2.0f;
    text->position.y = 200.0f;

    text = &ui->texts[UI_TEXT_PLAYER_NO];
    text->size = get_rendered_text_size(text->height, text->str);
    text->position.x = (ui->size.x - text->size.x) / 2.0f;
    text->position.y = 130.0f;

    text = &ui->texts[UI_TEXT_INITIALS];
    text->size = get_rendered_text_size(text->height, text->str);
    text->position.x = (ui->size.x - text->size.x) / 2.0f;
    text->position.y = ui->size.y - 140.0f;
}

static void check_snprintf_return_value(int rv, size_t output_str_size) {
    SDL_assert(rv > 0); // there was no error
    if (rv > 0) {
        SDL_assert((size_t)rv < output_str_size); // there was no truncation
    }
}

static void init_ui_touch_buttons(UI *ui, RendererWrapper *renderer,
                                  TouchInput *touch_input) {
    const Button touch_buttons_button[NUM_UI_TOUCH_BUTTONS] = {
        [UI_TOUCH_BUTTON_COIN] = BUTTON_COIN,
        [UI_TOUCH_BUTTON_ONE_PLAYER] = BUTTON_ONE_PLAYER,
        [UI_TOUCH_BUTTON_TWO_PLAYERS] = BUTTON_TWO_PLAYERS,
        [UI_TOUCH_BUTTON_ROTATE_LEFT] = BUTTON_ROTATE_LEFT,
        [UI_TOUCH_BUTTON_ROTATE_RIGHT] = BUTTON_ROTATE_RIGHT,
        [UI_TOUCH_BUTTON_THRUST] = BUTTON_THRUST,
        [UI_TOUCH_BUTTON_FIRE] = BUTTON_FIRE,
        [UI_TOUCH_BUTTON_HYPERSPACE] = BUTTON_HYPERSPACE,
    };

    for (int i = 0; i < NUM_UI_TOUCH_BUTTONS; i++) {
        ui->touch_buttons[i] = make_ui_touch_button(
            renderer, touch_input, &ui->images, touch_buttons_button[i]);
    }
}

static UITouchButton make_ui_touch_button(RendererWrapper *renderer,
                                          TouchInput *input, Images *images,
                                          Button button) {
    const Image buttons_image[NUM_BUTTONS] = {
        [BUTTON_COIN] = IMAGE_COIN_BUTTON,
        [BUTTON_ONE_PLAYER] = IMAGE_ONE_PLAYER_BUTTON,
        [BUTTON_TWO_PLAYERS] = IMAGE_TWO_PLAYER_BUTTON,
        [BUTTON_THRUST] = IMAGE_THRUST_BUTTON,
        [BUTTON_ROTATE_LEFT] = IMAGE_ROTATE_LEFT_BUTTON,
        [BUTTON_ROTATE_RIGHT] = IMAGE_ROTATE_RIGHT_BUTTON,
        [BUTTON_FIRE] = IMAGE_FIRE_BUTTON,
        [BUTTON_HYPERSPACE] = IMAGE_HYPERSPACE_BUTTON,
    };

    UITouchButton touch_button = {
        .button = button,
        .rect = input->buttons[button].rect,
        .texture = SDL_CreateTextureFromSurface(
            renderer->renderer, images->surfaces[buttons_image[button]]),
        .transparency = 0.4f,
        .visible = input->buttons[button].enabled,
    };

    if (touch_button.texture == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't create texture out of image surface: %s",
                     SDL_GetError());
        // NOTE: Would it be worth returning an error from this function
        // instead?
        exit(EXIT_FAILURE);
    }

    return touch_button;
}

void update_ui(UI *ui, TouchInput *touch_input) {
    ui->touch_buttons_transition =
        update_transition(ui->touch_buttons_transition);

    if (ui->hide_language_text_time <= SDL_GetTicks()) {
        ui->texts[UI_TEXT_LANGUAGE].visible = false;
    }

    // Hide the UI touch controls after a period of touch input inactivity.
    if (ui->touch_buttons_shown && !DEBUG_TOUCH_BUTTONS) {
        // Time is specified in ms.
        if (ui->game_state.last_touch_input != 0 &&
            SDL_GetTicks() - ui->game_state.last_touch_input >= 20000) {
            ui->touch_buttons_shown = false;
            ui->touch_buttons_transition =
                start_transition(ui->touch_buttons_transition, true);
        }
    }

    set_ui_dynamic_texts(ui);

    for (int i = 0; i < NUM_UI_TOUCH_BUTTONS; i++) {
        ui->touch_buttons[i] = update_ui_touch_button(
            ui->touch_buttons[i], touch_input, ui->touch_buttons_transition);
    }

    if (ui->game_state.coins > 0) {
        if (ui->push_start_text_timer <= SDL_GetTicks()) {
            ui->push_start_text_timer = SDL_GetTicks() + 500; // in ms
            if (ui->texts[UI_TEXT_PUSH_START].color.a == 0) {
                ui->texts[UI_TEXT_PUSH_START].color.a = 255;
            } else {
                ui->texts[UI_TEXT_PUSH_START].color.a = 0;
            }
        }
    } else {
        ui->push_start_text_timer = 0;
        ui->texts[UI_TEXT_PUSH_START].color.a = 0;
    }

    if (ui->flash_player_score) {
        if (ui->score_no_text_timer <= SDL_GetTicks()) {
            ui->score_no_text_timer = SDL_GetTicks() + 250; // in ms
            UITextEnum text = (ui->game_state.current_player_no == 1)
                                  ? UI_TEXT_P1_SCORE
                                  : UI_TEXT_P2_SCORE;
            if (ui->texts[text].color.a == 0) {
                ui->texts[text].color.a = 255;
            } else {
                ui->texts[text].color.a = 0;
            }
        }
    } else {
        ui->score_no_text_timer = 0;
        ui->texts[UI_TEXT_P1_SCORE].color.a = 255;
        ui->texts[UI_TEXT_P2_SCORE].color.a = 255;
    }
}

static UITouchButton update_ui_touch_button(UITouchButton touch_button,
                                            TouchInput *input,
                                            Transition trans) {
    touch_button.visible = input->buttons[touch_button.button].enabled;
    if (!touch_button.visible) {
        return touch_button;
    }

    if (input->buttons[touch_button.button].state != BUTTON_STATE_UP &&
        trans.done) {
        SDL_SetTextureAlphaMod(touch_button.texture,
                               fminf(touch_button.transparency + 0.2f, 1.0f) *
                                   255);
    } else {
        SDL_SetTextureAlphaMod(touch_button.texture,
                               trans.progress * touch_button.transparency *
                                   255);
    }

    return touch_button;
}

void show_current_language_name(UI *ui) {
    ui->hide_language_text_time = SDL_GetTicks() + 1500; // in ms
    ui->texts[UI_TEXT_LANGUAGE].visible = true;
}

void show_ui_touch_controls(UI *ui) {
    if (ui->touch_buttons_shown) {
        return;
    }
    ui->touch_buttons_shown = true;
    ui->touch_buttons_transition =
        start_transition(ui->touch_buttons_transition, false);
}

void init_ui_game_attract_state(UI *ui) {
    ui->texts[UI_TEXT_HIGHSCORES].visible = false;
    ui->texts[UI_TEXT_SCOREBOARD].visible = false;

    ui->texts[UI_TEXT_PLAYER_NO].visible = false;

    init_attract_state(ui);
}

static void init_attract_state(UI *ui) {
    ui->texts[UI_TEXT_P2_SCORE].visible = true;
    ui->texts[UI_TEXT_PUSH_START].visible = true;
    ui->texts[UI_TEXT_COIN_PLAY].visible = true;

    ui->texts[UI_TEXT_GAME_OVER].visible = false;

    ui->texts[UI_TEXT_P1_SCORE].color = normal_color;
    ui->texts[UI_TEXT_P2_SCORE].color = normal_color;
}

void init_ui_scoreboard_attract_state(UI *ui) {
    ui->texts[UI_TEXT_HIGHSCORES].visible = true;
    ui->texts[UI_TEXT_SCOREBOARD].visible = true;

    ui->texts[UI_TEXT_PLAYER_NO].visible = false;
    ui->texts[UI_TEXT_ENTER_INITIALS].visible = false;
    ui->texts[UI_TEXT_INITIALS].visible = false;

    init_attract_state(ui);
}

void init_ui_turn_start_state(UI *ui) {
    if (ui->game_state.two_players) {
        ui->render_p1_ships = true;
        ui->render_p2_ships = true;
        ui->flash_player_score = true;

        if (ui->game_state.current_player_no == 1) {
            ui->texts[UI_TEXT_P1_SCORE].color = highlight_color;
            ui->texts[UI_TEXT_P2_SCORE].color = normal_color;
        } else {
            ui->texts[UI_TEXT_P1_SCORE].color = normal_color;
            ui->texts[UI_TEXT_P2_SCORE].color = highlight_color;
        }
    } else {
        ui->render_p1_ships = true;
        ui->render_p2_ships = false;
        ui->texts[UI_TEXT_P2_SCORE].visible = false;
    }

    ui->texts[UI_TEXT_COIN_PLAY].visible = false;
    ui->texts[UI_TEXT_PUSH_START].visible = false;
    ui->texts[UI_TEXT_PLAYER_NO].visible = true;

    ui->texts[UI_TEXT_HIGHSCORES].visible = false;
    ui->texts[UI_TEXT_SCOREBOARD].visible = false;

    ui->texts[UI_TEXT_GAME_OVER].visible = false;
}

void init_ui_play_state(UI *ui) {
    ui->flash_player_score = false;

    ui->texts[UI_TEXT_PLAYER_NO].visible = false;
}

void init_ui_game_over_state(UI *ui) {
    ui->texts[UI_TEXT_GAME_OVER].visible = true;
    ui->texts[UI_TEXT_PLAYER_NO].visible = ui->game_state.two_players;
}

void init_ui_enter_initials_state(UI *ui) {
    ui->texts[UI_TEXT_GAME_OVER].visible = false;

    if (ui->game_state.two_players) {
        ui->texts[UI_TEXT_PLAYER_NO].visible = true;
    }

    ui->texts[UI_TEXT_P2_SCORE].visible = true;
    ui->texts[UI_TEXT_ENTER_INITIALS].visible = true;
    ui->texts[UI_TEXT_INITIALS].visible = true;

    ui->texts[UI_TEXT_P2_SCORE].color = normal_color;
    ui->texts[UI_TEXT_P1_SCORE].color = normal_color;
}

void render_ui(UI *ui, RendererWrapper *renderer) {
    for (int i = 0; i < NUM_UI_TEXTS; i++) {
        render_ui_text(renderer, ui->texts[i]);
    }

    for (int i = 0; i < NUM_UI_TOUCH_BUTTONS; i++) {
        render_ui_touch_button(renderer, ui->touch_buttons[i]);
    }

    render_players_ships(renderer, ui);
}

static void render_ui_text(RendererWrapper *renderer, UIText text) {
    if (text.visible) {
        render_text(renderer, text.position, text.height, text.str, text.color);
    }
}

static void render_ui_touch_button(RendererWrapper *renderer,
                                   UITouchButton button) {
    if (button.visible) {
        Rect2D rect = renderer_wrapper_scale_rect2d(renderer, button.rect);
        SDL_RenderCopy(renderer->renderer, button.texture, NULL, &rect);
    }
}

static void render_players_ships(RendererWrapper *renderer, UI *ui) {
    if (!ui->render_p1_ships) {
        return;
    }
    Vec2Df position = {130.0f, 55.0f};
    render_ships(renderer, position, true, ui->game_state.p1_ships);
    if (ui->render_p2_ships) {
        position.x = 829.0f;
        render_ships(renderer, position, false, ui->game_state.p2_ships);
    }
}

static void render_ships(RendererWrapper *renderer, Vec2Df position,
                         bool grows_right, int ships) {
    // There doesn't appear to be a limit in the number of ships that can appear
    // in Asteroids, but they will be limited to 16 so that ships from different
    // players do not overlap when in 2 player mode.
    ships = MIN(ships, 16);

    float scale = 10.0f;
    float rotation = (3.0f * pi) / 2.0f;
    Lines lines = idling_ship_lines;
    transform_lines(&lines, position, rotation, scale);

    // The player 2 ships need to grow to the left.
    Vec2Df offset = {scale * ((grows_right) ? 2.0f : -2.0f), 0.0f};
    while (ships-- > 0) {
        transform_lines(&lines, offset, 0.0f, 1.0f);
        render_lines(renderer, &lines, normal_color);
    }
}

void free_ui(UI *ui) {
    for (int i = 0; i < NUM_UI_TOUCH_BUTTONS; i++) {
        SDL_DestroyTexture(ui->touch_buttons[i].texture);
    }

    free_images(&ui->images);
}
