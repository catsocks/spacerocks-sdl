#include "spacerocks/scoreboard.h"

const char *const valid_initials_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_";

static int get_score_index(Scoreboard *board, int score);
static void shift_by_one(Scoreboard *board, int idx);
static void underscores_to_spaces(char *str);

// Return whether the given score is high enough to be in the scoreboard.
bool is_high_score(Scoreboard *board, int score) {
    return get_score_index(board, score) >= 0;
}

// Return the index where the given score would be placed in the scoreboard and
// -1 if it's too low to be included in it.
static int get_score_index(Scoreboard *board, int score) {
    for (int i = 0; i < MAX_SCORES; i++) {
        if (score > board->scores[i].points) {
            return i;
        }
    }
    return -1;
}

// Insert a score in the scoreboard and return true on success.
bool insert_scoreboard(Scoreboard *board, int score, const char *initials) {
    int idx = get_score_index(board, score);
    if (idx == -1) {
        return false;
    }

    if (score > board->highest_score) {
        board->highest_score = score;
    }

    shift_by_one(board, idx);

    strncpy(board->scores[idx].initials, initials,
            sizeof(board->scores[idx].initials));
    board->scores[idx].initials[sizeof(board->scores[idx].initials) - 1] = '\0';
    underscores_to_spaces(board->scores[idx].initials);

    board->scores[idx].points = score;

    if (board->num_scores < MAX_SCORES) {
        board->num_scores += 1;
    }

    return true;
}

static void shift_by_one(Scoreboard *board, int idx) {
    int num_scores = board->num_scores - idx;
    if (board->num_scores == MAX_SCORES) {
        num_scores--;
    }
    memmove(board->scores + idx + 1, board->scores + idx,
            sizeof(Score) * num_scores);
}

static void underscores_to_spaces(char *str) {
    for (size_t i = 0; i < strlen(str); i++) {
        if (str[i] == '_') {
            str[i] = ' ';
        }
    }
}

// Convert a string to a scoreboard and return true on success. On failure the
// scoreboard will not be cleaned up.
bool parse_scoreboard(Scoreboard *board, const char *str) {
    int str_idx = 0;
    for (int i = 0; i < MAX_SCORES; i++) {
        if (str[str_idx] == '\0') {
            return true;
        }

        str_idx += strcspn(&str[str_idx], "."); // ship score position number
        if (str[++str_idx] == '\0') {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Couldn't deserialize scoreboard: malformed score");
            return false;
        }
        str_idx += strspn(&str[str_idx], " "); // skip score padding

        // First read the points.
        char *last_read_char = NULL;
        long int points = strtol(&str[str_idx], &last_read_char, 10);
        if (points == LONG_MIN || points == LONG_MAX) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Couldn't deserialize scoreboard: invalid points");
            return false;
        }

        if (*last_read_char == '\0') {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Couldn't deserialize scoreboard: malformed score");
            return false;
        }
        str_idx += last_read_char - &str[str_idx];
        str_idx++; // ship space between points and initials

        // Then read the initials.
        char initials[NUM_INITIALS + 1] = {0};
        strncpy(initials, &str[str_idx], sizeof(initials));
        initials[sizeof(initials) - 1] = '\0';

        if (strlen(initials) != 3) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Couldn't deserialize scoreboard: invalid initials");
            return false;
        }

        // Skip to the next line.
        str_idx += strcspn(&str[str_idx], "\n") + 1;

        insert_scoreboard(board, points, initials);
    }

    return true;
}

// Convert a scoreboard to a string and return true on success. On failure
// str_size will be equal to zero and str will be empty.
bool stringify_scoreboard(Scoreboard *board, char *str, size_t str_size,
                          size_t *str_length) {
    bool ok = false;
    size_t chars_written = 0; // excl. null term.
    for (int i = 0; i < board->num_scores; i++) {
        if (str_size <= 1) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Couldn't serialize scoreboard: string is full");
            goto err;
        }

        int rv = snprintf(&str[chars_written], str_size, "%2d. %6d %s\n", i + 1,
                          board->scores[i].points, board->scores[i].initials);
        if (rv < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Couldn't serialize scoreboard: %s", strerror(errno));
            goto err;
        } else if ((size_t)rv >= str_size) { // it was truncated
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Couldn't serialize scoreboard: string is full");
            goto err;
        }
        chars_written += rv;
        str_size -= rv;
    }
    ok = true;
    if (str_length != NULL) {
        *str_length = chars_written;
    }
err:
    if (!ok) {
        str[0] = '\0';
    }
    return ok;
}
