#pragma once

#include <SDL.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

enum {
    NUM_INITIALS = 3,
    INITIALS_STR_SIZE = 4, // equal to NUM_INITIALS + null term.
    MAX_SCORES = 10,
    MAX_SCORE = 99999999, // the Asteroids world record seems to be 41,838,740
    SCORE_STR_SIZE = 9, // equal to the num. of digits in MAX_SCORE + null term.
    SCOREBOARD_STR_SIZE = 151,
};

struct Score {
    char initials[NUM_INITIALS + 1];
    int points;
};

typedef struct Score Score;

struct Scoreboard {
    int highest_score;
    Score scores[MAX_SCORES];
    int num_scores;
};

typedef struct Scoreboard Scoreboard;

extern const char *const valid_initials_chars;

bool is_high_score(Scoreboard *board, int score);
bool insert_scoreboard(Scoreboard *board, int score, const char *initials);
bool parse_scoreboard(Scoreboard *board, const char *str);
bool stringify_scoreboard(Scoreboard *board, char *str, size_t str_size,
                          size_t *str_length);
