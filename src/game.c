#include "spacerocks/game.h"

enum {
    ONE_PLAYER_GAME_COINS = 1,
    TWO_PLAYER_GAME_COINS = 2,
    MAX_COINS = TWO_PLAYER_GAME_COINS * 5,
};

static const char *const initial_initials = "A__";
static const char *const savefile_filename = "savefile";

static void init_player_game(PlayerGame *game, Vec2D logical_size,
                             Controller controller);
static void start_next_round(PlayerGame *game);
static InitialsInput make_initials_input();
static void start_game_attraction(Game *game);
static void set_attraction_change_time(Game *game);
static void enable_attract_state_touch_buttons(TouchInput *input);
static void disable_touch_input_buttons(TouchInput *input);
static void update_ui_game_state(Game *game, UIGameState *state);
static int game_event_watch(void *user_data, SDL_Event *event);
static void toggle_fullscreen(Game *game);
static void start_game_with_player(Game *game, bool two_players);
static void start_player_turn(Game *game, Player player);
static void enable_fullscreen(Game *game);
static void disable_fullscreen(Game *game);
static void toggle_audio_muted(Game *game);
static void cycle_through_languages(Game *game, int step);
static void check_player_game_input(PlayerGame *game, Input *input);
static void check_initials_input(Game *game);
static void start_scoreboard_attraction(Game *game);
static void cycle_through_initials_chars(InitialsInput *input, int step);
static void submit_initials_char(InitialsInput *input);
static void set_player_done_entering_initials(InitialsInput *input);
static void skip_initials_input(InitialsInput *input);
static void check_attraction_change_time(Game *game);
static void check_play_time(Game *game);
static void enable_play_state_touch_buttons(TouchInput *input);
static void check_turn_change(Game *game);
static void check_game_over(Game *game);
static void check_game_over_timeout(Game *Game);
static void enable_initials_input_touch_buttons(TouchInput *input);
static void check_player_done_entering_initials(Game *game);
static void update_player_game(PlayerGame *game, Audio *audio);
static void check_round_over(PlayerGame *game);
static void check_should_ship_spawn(PlayerGame *game);
static void check_ship_hyperspace_end_time(PlayerGame *game);
static void check_collisions(PlayerGame *game, Audio *audio);
static void destroy_ship(PlayerGame *game);
static void destroy_saucer(PlayerGame *game, bool add_score);
static void add_player_game_score(PlayerGame *game, int score);
static void play_explosion_sound(Audio *audio, AudioBuffer buffer);
static void play_rock_explosion_sound(Audio *audio, RockSize size);
static void destroy_and_split_rock(PlayerGame *game, Rock *rock,
                                   bool add_score);
static void update_pulse_sound(Game *game);
static void play_pulse_sound(PulseSound *pulse, Audio *audio);
static double get_pulse_silence_duration(uint32_t time_since_pulse_start);
static void set_pulse_silence_buffer(ALuint buffer, double t);
static void render_player_game(PlayerGame *game, RendererWrapper *renderer);

bool init_game(Game *game, SDL_Window *window, SDL_Renderer *renderer) {
    game->window = window;

    Vec2D logical_size = {960, 720};
    init_renderer_wrapper(&game->renderer, renderer, logical_size);

    init_audio(&game->audio);
    mute_audio(&game->audio);

    Scoreboard scoreboard = {0};
    if (read_savefile(&scoreboard, savefile_filename)) {
        game->scoreboard = scoreboard;
    }

    game->input = make_input();
    game->lang = get_user_language(LANGUAGE_ENGLISH);
    init_player_game(&game->players_game[PLAYER_1], logical_size, CONTROLLER_1);
    init_player_game(&game->players_game[PLAYER_2], logical_size, CONTROLLER_2);
    game->players_game[PLAYER_2].paused = true;
    game->initials_input = make_initials_input();

    if (!init_ui(&game->ui, &game->renderer, game->lang, &game->input.touch)) {
        return false;
    }

    start_game_attraction(game);
    update_ui_game_state(game, &game->ui.game_state);

    SDL_AddEventWatch(game_event_watch, game);

    return true;
}

static void init_player_game(PlayerGame *game, Vec2D logical_size,
                             Controller controller) {
    game->ship = make_ship(logical_size);
    game->saucer = make_saucer(logical_size, 0.0), game->ships_remaining = 3;
    game->controller = controller;
    game->current_time = SDL_GetPerformanceCounter();

    for (int i = 0; i < MAX_BULLETS; i++) {
        game->bullets[i] = make_bullet(logical_size);
    }

    for (int i = 0; i < MAX_ROCKS; i++) {
        game->rocks[i] = make_rock(logical_size);
    }

    for (int i = 0; i < MAX_DEBRIS; i++) {
        game->debris[i] = make_debris(logical_size);
    }

    start_next_round(game);
}

static void start_next_round(PlayerGame *game) {
    game->round++;

    int initial_large_rocks = 4;
    int large_rocks = initial_large_rocks * (1 + ((game->round - 1) * 0.5));
    if (large_rocks > MAX_BIG_ROCKS) {
        large_rocks = MAX_BIG_ROCKS;
    }
    spawn_large_rocks(game->rocks, large_rocks);
    game->small_rocks_remaining = large_rocks * 4;
}

static InitialsInput make_initials_input() {
    InitialsInput input = {
        .valid_initials_chars_length = strlen(valid_initials_chars),
    };
    strcpy(input.initials, initial_initials);
    return input;
}

static void start_game_attraction(Game *game) {
    game->state = STATE_GAME_ATTRACT;
    game->render_player_game = true;
    game->players_game[game->current_player].paused = false;
    set_attraction_change_time(game);
    enable_attract_state_touch_buttons(&game->input.touch);
    init_ui_game_attract_state(&game->ui);
}

static void set_attraction_change_time(Game *game) {
    if (game->scoreboard.num_scores > 0) {
        game->attraction_change_time = SDL_GetTicks() + 16000; // in ms
    }
}

static void enable_attract_state_touch_buttons(TouchInput *input) {
    disable_touch_input_buttons(input);
    input->buttons[BUTTON_COIN].enabled = true;
    input->buttons[BUTTON_ONE_PLAYER].enabled = true;
    input->buttons[BUTTON_TWO_PLAYERS].enabled = true;
}

static void disable_touch_input_buttons(TouchInput *input) {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        input->buttons[i].enabled = false;
    }
}

static void update_ui_game_state(Game *game, UIGameState *state) {
    state->lang = game->lang;
    state->two_players = game->two_players;
    state->current_player_no = game->current_player + 1;
    state->coins = game->coins;
    state->p1_ships = game->players_game[PLAYER_1].ships_remaining;
    state->p2_ships = game->players_game[PLAYER_2].ships_remaining;
    state->p1_score = game->players_game[PLAYER_1].score;
    state->p2_score = game->players_game[PLAYER_2].score;
    state->scoreboard = game->scoreboard;
    strcpy(state->initials, game->initials_input.initials);
    state->last_touch_input = game->last_touch_input;
}

// NOTE: Necessary so the SDL_WINDOWEVENT_SHOWN and SDL_WINDOWEVENT_HIDDEN
// events may be handled when running when using Emscripten.
static int game_event_watch(void *user_data, SDL_Event *event) {
    Game *game = user_data;

    if (event->type == SDL_WINDOWEVENT) {
        // SDL_WINDOWEVENT_SHOWN and SDL_WINDOWEVENT_HIDDEN will be added to the
        // event queue when the user switches to and from the browser in which
        // the game is running, respectively.
        switch (event->window.event) {
        case SDL_WINDOWEVENT_RESTORED:
        case SDL_WINDOWEVENT_SHOWN:
            unpause_audio_device(&game->audio);
            if (game->audio_muted_by_event) {
                unmute_audio(&game->audio);
                game->audio_muted_by_event = false;
            }
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
        case SDL_WINDOWEVENT_HIDDEN:
            if (!pause_audio_device(&game->audio)) {
                mute_audio(&game->audio);
                game->audio_muted_by_event = true;
            }
            break;
        }
    }
    return 0;
}

void check_keyboard_event(Game *game, SDL_Event *event) {
    input_check_keyboard_event(&game->input, event);
}

void check_finger_event(Game *game, SDL_Event *event) {
    game->last_touch_input = SDL_GetTicks();
    show_ui_touch_controls(&game->ui);
    input_check_finger_event(&game->input, game->renderer.logical_size, event);
}

void check_controller_device_event(Game *game, SDL_Event *event) {
    input_check_controller_device_event(&game->input, event);
}

void check_controller_button_event(Game *game, SDL_Event *event) {
    input_check_controller_button_event(&game->input, event);
}

void check_game_input(Game *game) {
    if (is_button_just_released(&game->input, BUTTON_COIN)) {
        if (game->coins < MAX_COINS) {
            game->coins++;
        }
    }

    if (is_button_just_released(&game->input, BUTTON_ONE_PLAYER)) {
        if (game->coins >= ONE_PLAYER_GAME_COINS) {
            start_game_with_player(game, false);
            game->coins -= ONE_PLAYER_GAME_COINS;
        }
    }

    if (is_button_just_released(&game->input, BUTTON_TWO_PLAYERS)) {
        if (game->coins >= TWO_PLAYER_GAME_COINS) {
            start_game_with_player(game, true);
            game->coins -= TWO_PLAYER_GAME_COINS;
        }
    }

    if (is_button_just_pressed(&game->input, BUTTON_TOGGLE_FULLSCREEN)) {
        toggle_fullscreen(game);
    }

    if (is_button_just_pressed(&game->input, BUTTON_ENABLE_FULLSCREEN)) {
        enable_fullscreen(game);
    }

    if (is_button_just_pressed(&game->input, BUTTON_DISABLE_FULLSCREEN)) {
        disable_fullscreen(game);
    }

    if (is_button_just_pressed(&game->input, BUTTON_TOGGLE_MUTE)) {
        toggle_audio_muted(game);
    }

    if (is_button_just_pressed(&game->input, BUTTON_NEXT_LANG)) {
        cycle_through_languages(game, 1);
    }

    if (is_button_just_pressed(&game->input, BUTTON_PREV_LANG)) {
        cycle_through_languages(game, -1);
    }

    check_player_game_input(&game->players_game[game->current_player],
                            &game->input);

    check_initials_input(game);

    update_input(&game->input);
}

static void start_game_with_player(Game *game, bool two_players) {
    if (game->state != STATE_GAME_ATTRACT &&
        game->state != STATE_SCOREBOARD_ATTRACT) {
        return;
    }

    if (!game->audio_muted_by_player) {
        stop_all_audio_sources(&game->audio);
        unmute_audio(&game->audio);
    }

    game->players_game[PLAYER_1] = (PlayerGame){0};
    game->players_game[PLAYER_2] = (PlayerGame){0};
    init_player_game(&game->players_game[PLAYER_1], game->renderer.logical_size,
                     CONTROLLER_1);
    init_player_game(&game->players_game[PLAYER_2], game->renderer.logical_size,
                     CONTROLLER_2);

    game->two_players = two_players;
    game->players_game[PLAYER_1].single_player = !game->two_players;
    game->players_game[PLAYER_2].single_player = !game->two_players;

    game->players_game[PLAYER_1].paused = true;
    game->players_game[PLAYER_2].paused = true;
    game->render_player_game = false;
    start_player_turn(game, PLAYER_1);
}

static void start_player_turn(Game *game, Player player) {
    if (game->state != STATE_GAME_ATTRACT &&
        game->state != STATE_SCOREBOARD_ATTRACT &&
        game->state != STATE_GAME_OVER && game->state != STATE_PLAY) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                     "start_player_turn called in the wrong game state");
        return;
    }

    game->state = STATE_TURN_START;
    game->current_player = player;
    game->play_time = SDL_GetTicks() + 2000; // in ms
    disable_touch_input_buttons(&game->input.touch);
    update_ui_game_state(game, &game->ui.game_state);
    init_ui_turn_start_state(&game->ui);
}

static void toggle_fullscreen(Game *game) {
    if (SDL_GetWindowFlags(game->window) & SDL_WINDOW_FULLSCREEN_DESKTOP) {
        disable_fullscreen(game);
    } else {
        enable_fullscreen(game);
    }
}

static void enable_fullscreen(Game *game) {
#ifdef __EMSCRIPTEN__
    // Enabling fullscreen when fullscreenEnabled is undefined, on Safari 14 for
    // example, can result in a buggy fullscreen state, so it's best not to call
    // SDL_SetWindowFullscreen.
    // TODO: Remove when unnecessary. As of 2021-05-17 Safari doesn't fully
    // support the Full Screen API yet: https://caniuse.com/fullscreen.
    // clang-format off
    int fullscreen_disabled = EM_ASM_INT(
        if (navigator.userAgent.includes('AppleWebKit')) {
            return 'fullscreenEnabled' in document === false;
        }
        return false;
    );
    // clang-format on
    if (fullscreen_disabled == 1) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "Prevented fullscreen from being enabled due to "
                    "document.fullscreenEnabled being undefined");
        return;
    }
#endif
    if (SDL_SetWindowFullscreen(game->window, SDL_WINDOW_FULLSCREEN_DESKTOP) <
        0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't enable desktop fullscreen: %s", SDL_GetError());
    }
}

static void disable_fullscreen(Game *game) {
    if (SDL_SetWindowFullscreen(game->window, 0) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't disable fullscreen: %s", SDL_GetError());
    }
}

static void toggle_audio_muted(Game *game) {
    if (game->audio.muted) {
        if (game->audio_muted_by_player) {
            unmute_audio(&game->audio);
            game->audio_muted_by_player = false;
        }
    } else {
        mute_audio(&game->audio);
        game->audio_muted_by_player = true;
    }
}

static void cycle_through_languages(Game *game, int step) {
    game->lang = wrap_around(game->lang + step, NUM_LANGUAGES);
    update_ui_game_state(game, &game->ui.game_state);
    set_ui_localized_texts(&game->ui);
    show_current_language_name(&game->ui);
}

static void check_player_game_input(PlayerGame *game, Input *input) {
    if (DEBUG_SPACEROCKS) {
        if (is_button_just_pressed(input, BUTTON_ADD_SHIP)) {
            game->ships_remaining++;
        }

        if (is_button_just_pressed(input, BUTTON_ADD_SCORE)) {
            game->score += 5000;
        }

        if (is_button_just_pressed(input, BUTTON_DESTROY_SHIP)) {
            destroy_ship(game);
        }

        if (is_button_just_pressed(input, BUTTON_DESTROY_ROCKS)) {
            for (int i = 0; i < MAX_ROCKS; i++) {
                destroy_and_split_rock(game, &game->rocks[i], true);
            }
        }

        if (is_button_just_pressed(input, BUTTON_DESTROY_SAUCER)) {
            destroy_saucer(game, true);
        }

        if (is_button_just_pressed(input, BUTTON_GAME_OVER)) {
            if (game->ship.spawned) {
                game->ships_remaining = 0;
                destroy_ship(game);
            }
        }
    }

    check_ship_input(&game->ship, input, game->controller, game->time);
}

static void check_initials_input(Game *game) {
    if (game->state != STATE_ENTER_INITIALS) {
        return;
    }

    Controller ctrl = game->players_game[game->current_player].controller;
    if (is_button_just_pressed_controller(&game->input, BUTTON_ROTATE_LEFT,
                                          ctrl)) {
        cycle_through_initials_chars(&game->initials_input, -1);
    } else if (is_button_just_pressed_controller(&game->input,
                                                 BUTTON_ROTATE_RIGHT, ctrl)) {
        cycle_through_initials_chars(&game->initials_input, 1);
    } else if (is_button_just_pressed_controller(&game->input,
                                                 BUTTON_HYPERSPACE, ctrl)) {
        submit_initials_char(&game->initials_input);
    } else if (is_button_just_pressed_controller(&game->input, BUTTON_FIRE,
                                                 ctrl)) {
        skip_initials_input(&game->initials_input);
    }
}

// Start the scoreboard attraction if there are scores to be shown, otherwise
// start the game attraction.
static void start_scoreboard_attraction(Game *game) {
    if (game->state != STATE_GAME_ATTRACT && game->state != STATE_GAME_OVER &&
        game->state != STATE_ENTER_INITIALS) {
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "start_scoreboard_attraction called in the wrong game state");
        return;
    }

    if (game->scoreboard.num_scores == 0) {
        start_game_attraction(game);
        return;
    }

    game->state = STATE_SCOREBOARD_ATTRACT;
    game->render_player_game = false;
    game->players_game[game->current_player].paused = true;
    set_attraction_change_time(game);
    enable_attract_state_touch_buttons(&game->input.touch);
    init_ui_scoreboard_attract_state(&game->ui);
}

static void cycle_through_initials_chars(InitialsInput *input, int step) {
    input->chars_idx = wrap_around(input->chars_idx + step,
                                   input->valid_initials_chars_length);
    input->initials[input->cursor] = valid_initials_chars[input->chars_idx];
}

static void submit_initials_char(InitialsInput *input) {
    if (input->cursor == NUM_INITIALS - 1) {
        set_player_done_entering_initials(input);
        return;
    }
    input->cursor++;
    input->chars_idx = 0;
    input->initials[input->cursor] = valid_initials_chars[input->chars_idx];
}

static void set_player_done_entering_initials(InitialsInput *input) {
    input->cursor = 0;
    input->chars_idx = 0;
    input->player_done = true;
}

static void skip_initials_input(InitialsInput *input) {
    for (int i = 0; i < NUM_INITIALS; i++) {
        input->initials[i] = ' ';
    }
    set_player_done_entering_initials(input);
}

void update_game(Game *game) {
    check_attraction_change_time(game);
    check_play_time(game);
    check_turn_change(game);
    check_game_over(game);
    check_game_over_timeout(game);
    check_player_done_entering_initials(game);
    update_player_game(&game->players_game[PLAYER_1], &game->audio);
    update_player_game(&game->players_game[PLAYER_2], &game->audio);
    update_pulse_sound(game);
    play_pulse_sound(&game->pulse, &game->audio);
    update_ui_game_state(game, &game->ui.game_state);
    update_ui(&game->ui, &game->input.touch);
}

static void check_attraction_change_time(Game *game) {
    if (game->state >= STATE_TURN_START) {
        return;
    }

    if (game->attraction_change_time == 0 ||
        game->attraction_change_time > SDL_GetTicks()) {
        return;
    }

    if (game->state == STATE_GAME_ATTRACT) {
        start_scoreboard_attraction(game);
        return;
    }
    start_game_attraction(game);
}

// Transition the game from STATE_TURN_START to STATE_PLAY.
static void check_play_time(Game *game) {
    if (game->state != STATE_TURN_START) {
        return;
    }

    if (game->play_time == 0.0 || game->play_time > SDL_GetTicks()) {
        return;
    }
    game->play_time = 0.0;
    game->state = STATE_PLAY;

    // Undo the hiding and pausing that happens when a player starts the game
    // and the first turn begins.
    game->render_player_game = true;
    game->players_game[game->current_player].paused = false;

    game->players_game[game->current_player].should_ship_spawn = true;

    enable_play_state_touch_buttons(&game->input.touch);
    init_ui_play_state(&game->ui);
}

static void enable_play_state_touch_buttons(TouchInput *input) {
    disable_touch_input_buttons(input);
    input->buttons[BUTTON_ROTATE_LEFT].enabled = true;
    input->buttons[BUTTON_ROTATE_RIGHT].enabled = true;
    input->buttons[BUTTON_THRUST].enabled = true;
    input->buttons[BUTTON_FIRE].enabled = true;
    input->buttons[BUTTON_HYPERSPACE].enabled = true;
}

// Transition the game from STATE_GAME_OVER to STATE_TURN_PLAY in a 2 player
// game.
static void check_turn_change(Game *game) {
    if (!game->two_players) {
        return;
    }

    if (game->state != STATE_PLAY && game->state != STATE_GAME_OVER) {
        return;
    }

    // A player turn to play ends when their ship is destroyed and their ship
    // respawn cooldown expires.
    PlayerGame *current_game = &game->players_game[game->current_player];
    if (current_game->ship.respawn_cooldown == 0.0 ||
        current_game->ship.respawn_cooldown > current_game->time) {
        return;
    }
    if (current_game->should_ship_spawn) {
        return;
    }

    Player other_player = (game->current_player + 1) % NUM_PLAYERS;
    PlayerGame *other_game = &game->players_game[other_player];
    if (other_game->ships_remaining > 0) {
        stop_all_audio_sources(&game->audio);
        current_game->paused = true;
        other_game->paused = false;
        start_player_turn(game, other_player);
    } else if (current_game->ships_remaining > 0) {
        current_game->should_ship_spawn = true;
    }
}

// Transition the game from STATE_PLAY to STATE_GAME_OVER and conditionally set
// the time for the transition to STATE_ENTER_INITIALS.
static void check_game_over(Game *game) {
    if (game->state != STATE_PLAY) {
        return;
    }

    if (game->players_game[game->current_player].ships_remaining > 0) {
        return;
    }

    game->state = STATE_GAME_OVER;
    disable_touch_input_buttons(&game->input.touch);
    init_ui_game_over_state(&game->ui);

    Player other_player = (game->current_player + 1) % NUM_PLAYERS;
    if (!game->two_players ||
        game->players_game[other_player].ships_remaining == 0) {
        game->game_over_timeout = SDL_GetTicks() + 2000; // in ms
    }
}

static void check_game_over_timeout(Game *game) {
    if (game->game_over_timeout == 0) {
        return;
    }

    if (game->game_over_timeout > SDL_GetTicks()) {
        return;
    }
    game->game_over_timeout = 0;

    Player current_player = game->current_player;
    Player other_player = (game->current_player + 1) % NUM_PLAYERS;
    if (is_high_score(&game->scoreboard,
                      game->players_game[current_player].score)) {
        game->initials_input.players_remaining++;
    }
    if (is_high_score(&game->scoreboard,
                      game->players_game[other_player].score)) {
        game->initials_input.players_remaining++;
    }

    stop_all_audio_sources(&game->audio);
    mute_audio(&game->audio);

    if (game->initials_input.players_remaining == 0) {
        if (game->players_game[current_player].score == 0 &&
            game->players_game[other_player].score == 0) {
            start_game_attraction(game);
        } else {
            start_scoreboard_attraction(game);
        }
        return;
    }

    game->state = STATE_ENTER_INITIALS;
    game->players_game[current_player].paused = true;
    game->render_player_game = false;

    enable_initials_input_touch_buttons(&game->input.touch);
    init_ui_enter_initials_state(&game->ui);
}

static void enable_initials_input_touch_buttons(TouchInput *input) {
    disable_touch_input_buttons(input);
    input->buttons[BUTTON_ROTATE_LEFT].enabled = true;
    input->buttons[BUTTON_ROTATE_RIGHT].enabled = true;
    input->buttons[BUTTON_FIRE].enabled = true;
    input->buttons[BUTTON_HYPERSPACE].enabled = true;
}

static void check_player_done_entering_initials(Game *game) {
    if (game->state != STATE_ENTER_INITIALS) {
        return;
    }

    if (!game->initials_input.player_done) {
        return;
    }
    game->initials_input.player_done = false;

    int score = game->players_game[game->current_player].score;
    insert_scoreboard(&game->scoreboard, score, game->initials_input.initials);

    write_savefile(&game->scoreboard, savefile_filename);

    strcpy(game->initials_input.initials, initial_initials);
    if (--game->initials_input.players_remaining == 1) {
        game->current_player = (game->current_player + 1) % NUM_PLAYERS;
        return;
    }
    game->initials_input.players_remaining = 0;

    start_scoreboard_attraction(game);
}

static void update_player_game(PlayerGame *game, Audio *audio) {
    uint64_t previous_time = game->current_time;
    game->current_time = SDL_GetPerformanceCounter();
    double frame_time = (game->current_time - previous_time) /
                        (double)SDL_GetPerformanceFrequency();

    // Slow the game down if the time since the last frame is over a second.
    frame_time = fmin(frame_time, 1.0);

    while (!game->paused && frame_time > 0.0) {
        double max_frame_time = 1.0 / 60.0;
        double delta_time = fmin(frame_time, max_frame_time);

        check_round_over(game);
        check_should_ship_spawn(game);
        check_ship_hyperspace_end_time(game);

        fire_ship_bullets(&game->ship, game->bullets, audio, game->time);
        update_ship(&game->ship, game->time, delta_time);
        play_ship_thrusting_sound(&game->ship, audio);

        set_saucer_target(&game->saucer, game->ship.physics,
                          game->ship.graphics.visible);
        fire_saucer_bullets(&game->saucer, &game->bullets[SHIP_BULLETS - 1],
                            audio, game->time);
        update_saucer(&game->saucer, game->score, game->time, delta_time);
        play_saucer_warning_sound(&game->saucer, audio);

        for (int i = 0; i < MAX_BULLETS; ++i) {
            update_bullet(&game->bullets[i], game->time, delta_time);
        }

        for (int i = 0; i < MAX_ROCKS; ++i) {
            update_rock(&game->rocks[i], delta_time);
        }

        for (int i = 0; i < MAX_DEBRIS; ++i) {
            update_debris(&game->debris[i], delta_time);
        }

        check_collisions(game, audio);

        frame_time -= delta_time;
        game->time += delta_time;
    }
}

static void check_round_over(PlayerGame *game) {
    if (game->small_rocks_remaining > 0 || game->saucer.graphics.visible) {
        return;
    }

    if (game->next_round_time == 0.0) {
        game->next_round_time = game->time + 1.0;
        return;
    }

    if (game->time >= game->next_round_time) {
        game->next_round_time = 0.0;
        start_next_round(game);
    }
}

static void check_should_ship_spawn(PlayerGame *game) {
    if (game->ship.spawned || !game->should_ship_spawn) {
        return;
    }

    if (game->ships_remaining == 0 ||
        game->ship.respawn_cooldown > game->time) {
        return;
    }

    if (!can_ship_spawn_safely(&game->ship, &game->saucer, game->rocks)) {
        return;
    }

    spawn_ship(&game->ship);

    if (!game->single_player) {
        game->should_ship_spawn = false;
    }
}

static void check_ship_hyperspace_end_time(PlayerGame *game) {
    if (!game->ship.hyperspace_enabled ||
        game->ship.hyperspace_end_time > game->time) {
        return;
    }

    disable_hyperspace(&game->ship);
    if (percent_chance(25)) {
        destroy_ship(game);
    }
}

static void check_collisions(PlayerGame *game, Audio *audio) {
    if (graphics_intersect(&game->ship.graphics, &game->saucer.graphics)) {
        destroy_ship(game);
        destroy_saucer(game, true);
        play_explosion_sound(audio, AUDIO_BUFFER_EXPLOSION_BIG);
    }

    for (int r = 0; r < MAX_ROCKS; r++) {
        Rock *rock = &game->rocks[r];

        if (graphics_intersect(&game->ship.graphics, &rock->graphics)) {
            destroy_and_split_rock(game, rock, true);
            destroy_ship(game);
            play_explosion_sound(audio, AUDIO_BUFFER_EXPLOSION_BIG);
            continue;
        }

        if (graphics_intersect(&game->saucer.graphics, &rock->graphics)) {
            destroy_and_split_rock(game, rock, false);
            destroy_saucer(game, false);
            play_explosion_sound(audio, AUDIO_BUFFER_EXPLOSION_BIG);
            continue;
        }

        for (int b = 0; b < MAX_BULLETS; b++) {
            Bullet *bullet = &game->bullets[b];
            if (graphics_intersect(&bullet->graphics, &rock->graphics)) {
                play_rock_explosion_sound(audio, rock->size);
                destroy_and_split_rock(game, rock, b < SHIP_BULLETS - 1);
                despawn_bullet(bullet);
                break;
            }
        }
    }

    for (int i = 0; i < MAX_BULLETS; i++) {
        Bullet *bullet = &game->bullets[i];

        if (graphics_intersect(&bullet->graphics, &game->saucer.graphics)) {
            if (i >= SHIP_BULLETS - 1) {
                continue;
            }
            despawn_bullet(bullet);
            destroy_saucer(game, true);
            play_explosion_sound(audio, AUDIO_BUFFER_EXPLOSION_BIG);
            continue;
        }

        if (graphics_intersect(&bullet->graphics, &game->ship.graphics)) {
            if (i < SHIP_BULLETS - 1) {
                continue;
            }
            despawn_bullet(bullet);
            destroy_ship(game);
            play_explosion_sound(audio, AUDIO_BUFFER_EXPLOSION_BIG);
        }
    }
}

static void destroy_ship(PlayerGame *game) {
    // In case the function is called by BUTTON_DESTROY_SHIP.
    if (!game->ship.spawned) {
        return;
    }

    if (game->ships_remaining > 0) {
        game->ships_remaining--;
    }

    despawn_ship(&game->ship, game->time);
    spawn_debris_from_unspawned(game->debris, DEBRIS_KIND_SHIP,
                                game->ship.physics.position);
}

static void destroy_saucer(PlayerGame *game, bool add_score) {
    // In case the function is called by BUTTON_DESTROY_SAUCER.
    if (!game->saucer.graphics.visible) {
        return;
    }

    if (add_score) {
        add_player_game_score(game, (game->saucer.big) ? 200 : 1000);
    }
    despawn_saucer(&game->saucer, true, game->time);
    spawn_debris_from_unspawned(game->debris,
                                (game->saucer.big) ? DEBRIS_KIND_BIG_SAUCER
                                                   : DEBRIS_KIND_SMALL_SAUCER,
                                game->saucer.physics.position);
}

static void add_player_game_score(PlayerGame *game, int score) {
    int old_score = game->score;

    // Stop increasing the score once it reaches a limit unlike the original
    // Asteroids which turns over at 99,990.
    game->score = MIN(game->score + score, MAX_SCORE);

    // Award the player with an extra ship when the score is equal to or goes
    // over a multiple of 10,000.
    if (game->score > old_score) {
        int multiple = 10000;
        if ((game->score - old_score) / multiple >= 1) {
            game->ships_remaining += (game->score - old_score) / multiple;
        } else if (old_score % multiple > game->score % multiple) {
            game->ships_remaining++;
        }
    }
}

static void play_explosion_sound(Audio *audio, AudioBuffer buffer) {
    ALuint source = audio->sources[AUDIO_SOURCE_EXPLOSION];
    ALuint al_buffer = audio->buffers[buffer];
    ALint current_al_buffer = 0;
    alGetSourcei(source, AL_BUFFER, &current_al_buffer);
    if (current_al_buffer != (ALint)al_buffer) {
        alSourceStop(source);
        alSourcei(source, AL_BUFFER, al_buffer);
    }
    alSourcePlay(source);
}

static void play_rock_explosion_sound(Audio *audio, RockSize size) {
    AudioBuffer sizes_buffer[NUM_ROCK_SIZES] = {
        [ROCK_SIZE_SMALL] = AUDIO_BUFFER_EXPLOSION_SMALL,
        [ROCK_SIZE_MEDIUM] = AUDIO_BUFFER_EXPLOSION_MEDIUM,
        [ROCK_SIZE_BIG] = AUDIO_BUFFER_EXPLOSION_BIG,
    };
    play_explosion_sound(audio, sizes_buffer[size]);
}

static void destroy_and_split_rock(PlayerGame *game, Rock *rock,
                                   bool add_score) {
    // In case the function is called by BUTTON_DESTROY_ROCKS.
    if (!rock->graphics.visible) {
        return;
    }

    int sizes_score[NUM_ROCK_SIZES] = {
        [ROCK_SIZE_SMALL] = 100,
        [ROCK_SIZE_MEDIUM] = 50,
        [ROCK_SIZE_BIG] = 20,
    };
    if (add_score) {
        add_player_game_score(game, sizes_score[rock->size]);
    }

    DebrisKind sizes_debris[NUM_ROCK_SIZES] = {
        [ROCK_SIZE_BIG] = DEBRIS_KIND_BIG_ROCK,
        [ROCK_SIZE_MEDIUM] = DEBRIS_KIND_MEDIUM_ROCK,
        [ROCK_SIZE_SMALL] = DEBRIS_KIND_SMALL_ROCK,
    };
    despawn_rock(rock);
    spawn_debris_from_unspawned(game->debris, sizes_debris[rock->size],
                                rock->physics.position);

    if (rock->size == ROCK_SIZE_SMALL) {
        game->small_rocks_remaining--;
        SDL_assert(game->small_rocks_remaining >= 0);
        return;
    }
    split_rock(game->rocks, rock);
}

static void update_pulse_sound(Game *game) {
    if (game->state != STATE_PLAY) {
        game->pulse.enabled = false;
        game->pulse.start_time = 0;
        return;
    }

    if (game->pulse.start_time == 0) {
        game->pulse.start_time = SDL_GetTicks() + 1000; // in ms
    }

    PlayerGame *player_game = &game->players_game[game->current_player];
    if (!player_game->ship.graphics.visible) {
        game->pulse.enabled = false;
        return;
    }
    if (player_game->small_rocks_remaining == 0) {
        game->pulse.enabled = false;
        game->pulse.start_time = 0;
        return;
    }

    game->pulse.enabled = SDL_GetTicks() > game->pulse.start_time;
}

static void play_pulse_sound(PulseSound *pulse, Audio *audio) {
    ALuint source = audio->sources[AUDIO_SOURCE_PULSE];
    if (!pulse->enabled) {
        alSourcef(source, AL_GAIN, 0.0);
        return;
    }

    alSourcef(source, AL_GAIN, 1.0);

    double silence_duration =
        get_pulse_silence_duration((SDL_GetTicks() - pulse->start_time));

    ALint state = 0;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING) {
        ALuint buffers[] = {
            audio->buffers[AUDIO_BUFFER_PULSE_LOW],
            audio->buffers[AUDIO_BUFFER_PULSE_SILENCE_1],
            audio->buffers[AUDIO_BUFFER_PULSE_HIGH],
            audio->buffers[AUDIO_BUFFER_PULSE_SILENCE_2],
        };

        ALint source_type = 0;
        alGetSourcei(source, AL_SOURCE_TYPE, &source_type);
        if (source_type == AL_STREAMING) {
            ALuint processed_buffers[ARRAY_SIZE(buffers)] = {0};
            alSourceUnqueueBuffers(source, ARRAY_SIZE(processed_buffers),
                                   processed_buffers);
        }

        set_pulse_silence_buffer(buffers[1], silence_duration);
        set_pulse_silence_buffer(buffers[3], silence_duration);
        alSourceQueueBuffers(source, ARRAY_SIZE(buffers), buffers);
        alSourcePlay(source);
        return;
    }

    ALint processed = 0;
    alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
    while (processed-- > 0) {
        ALuint buffer = 0;
        alSourceUnqueueBuffers(source, 1, &buffer);
        if (buffer == audio->buffers[AUDIO_BUFFER_PULSE_LOW] ||
            buffer == audio->buffers[AUDIO_BUFFER_PULSE_HIGH]) {
            alSourceQueueBuffers(source, 1, &buffer);
            continue;
        }

        set_pulse_silence_buffer(buffer, silence_duration);
        alSourceQueueBuffers(source, 1, &buffer);
    }
}

static double get_pulse_silence_duration(uint32_t time_since_pulse_start) {
    // trunc(1000 - (t * 0.01583)) = 50 where t equals 60000.
    return MAX(1000 - (time_since_pulse_start * 0.01583), 50) / 1000.0;
}

static void set_pulse_silence_buffer(ALuint buffer, double t) {
    // The type of the samples is determined by AUDIO_FORMAT.
    static int16_t samples[AUDIO_FREQUENCY];
    int sample_count =
        MIN((unsigned)(t * AUDIO_FREQUENCY), ARRAY_SIZE(samples));
    int num_bytes = sample_count * sizeof(int16_t);
    alBufferData(buffer, AUDIO_FORMAT, samples, num_bytes, AUDIO_FREQUENCY);
}

void render_game(Game *game) {
    SDL_SetRenderDrawColor(game->renderer.renderer, 0, 0, 0, 255);
    SDL_RenderClear(game->renderer.renderer);

    if (game->render_player_game) {
        render_player_game(&game->players_game[game->current_player],
                           &game->renderer);
    }

    render_ui(&game->ui, &game->renderer);

    SDL_RenderPresent(game->renderer.renderer);
}

static void render_player_game(PlayerGame *game, RendererWrapper *renderer) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        render_graphics(renderer, &game->bullets[i].graphics);
    }

    for (int i = 0; i < MAX_ROCKS; i++) {
        render_graphics(renderer, &game->rocks[i].graphics);
    }

    for (int i = 0; i < MAX_DEBRIS; i++) {
        render_graphics(renderer, &game->debris[i].graphics);
    }

    render_graphics(renderer, &game->ship.graphics);
    render_graphics(renderer, &game->saucer.graphics);
}

void deinit_game(Game *game) {
    deinit_audio(&game->audio);
    free_ui(&game->ui);
}
