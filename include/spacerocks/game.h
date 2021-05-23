#pragma once

#include <SDL.h>

#include "spacerocks/assets.h"
#include "spacerocks/audio.h"
#include "spacerocks/bullet.h"
#include "spacerocks/debris.h"
#include "spacerocks/input.h"
#include "spacerocks/language.h"
#include "spacerocks/renderer.h"
#include "spacerocks/rock.h"
#include "spacerocks/saucer.h"
#include "spacerocks/savefile.h"
#include "spacerocks/scoreboard.h"
#include "spacerocks/ship.h"
#include "spacerocks/ui.h"
#include "spacerocks/util.h"

enum State {
    // Start the game without the ship.
    STATE_GAME_ATTRACT,
    // Show high scores if there are any to be shown.
    STATE_SCOREBOARD_ATTRACT,
    // Indicate whether it's the player 1 or 2's turn to play.
    STATE_TURN_START,
    // Let the player play the game.
    STATE_PLAY,
    // Indicate the game is over for a player.
    STATE_GAME_OVER,
    // Ask a player to enter their initials if their score is high enough for
    // the high scores scoreboard.
    STATE_ENTER_INITIALS,
};

typedef enum State State;

struct PlayerGame {
    bool single_player;
    Ship ship;
    Saucer saucer;
    Bullet bullets[MAX_BULLETS];
    Rock rocks[MAX_ROCKS];
    Debris debris[MAX_DEBRIS];
    int ships_remaining;
    Controller controller;
    int round;
    int small_rocks_remaining;
    bool should_ship_spawn;
    double next_round_time;
    double time;
    uint64_t current_time;
    bool paused;
    int score;
};

typedef struct PlayerGame PlayerGame;

enum Player {
    PLAYER_1,
    PLAYER_2,
    NUM_PLAYERS,
};

typedef enum Player Player;

struct InitialsInput {
    int valid_initials_chars_length;
    char initials[INITIALS_STR_SIZE];
    int cursor;
    int chars_idx;
    bool player_done;
    int players_remaining;
};

typedef struct InitialsInput InitialsInput;

struct PulseSound {
    bool enabled;
    uint32_t start_time;
};

typedef struct PulseSound PulseSound;

struct Game {
    State state;
    SDL_Window *window;
    RendererWrapper renderer;
    Audio audio;
    Scoreboard scoreboard;
    Input input;
    Language lang;
    PlayerGame players_game[NUM_PLAYERS];
    InitialsInput initials_input;
    UI ui;
    PulseSound pulse;
    bool audio_muted_by_player;
    bool audio_muted_by_event;
    int coins;
    bool two_players;
    bool render_player_game;
    Player current_player;
    uint32_t play_time;
    uint32_t attraction_change_time;
    uint32_t game_over_timeout;
    uint32_t last_touch_input;
};

typedef struct Game Game;

bool init_game(Game *game, SDL_Window *window, SDL_Renderer *renderer);
void check_game_input(Game *game);
void check_keyboard_event(Game *game, SDL_Event *event);
void check_finger_event(Game *game, SDL_Event *event);
void check_controller_device_event(Game *game, SDL_Event *event);
void check_controller_button_event(Game *game, SDL_Event *event);
void update_game(Game *game);
void render_game(Game *game);
void deinit_game(Game *game);
