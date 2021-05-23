#include <assert.h>
#include <string.h>

#include "spacerocks/scoreboard.h"
#include "spacerocks/util.h"

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    Scoreboard board = {0};

    // is_high_score should tell when a score is high enough for the scoreboard.
    assert(is_high_score(&board, 1));

    // insert_scoreboard should store scores.
    assert(insert_scoreboard(&board, 10, "JJJ"));
    assert(insert_scoreboard(&board, 9, "III"));
    assert(insert_scoreboard(&board, 8, "HHH"));
    assert(insert_scoreboard(&board, 7, "GGG"));
    assert(insert_scoreboard(&board, 6, "FFF"));
    assert(insert_scoreboard(&board, 1, "AAA"));

    // insert_scoreboard should store scores and move other scores if necessary.
    assert(insert_scoreboard(&board, 2, "B B"));
    assert(insert_scoreboard(&board, 3, "   "));
    assert(insert_scoreboard(&board, 4, "DDD"));
    assert(insert_scoreboard(&board, 5, "EEE"));

    // insert_scoreboard should store scores and discard other scores if
    // necessary.
    // KKK should take the place of AAA.
    assert(insert_scoreboard(&board, 2, "KKK"));

    // The scores should be in the correct order.
    assert(strcmp(board.scores[0].initials, "JJJ") == 0);
    assert(strcmp(board.scores[1].initials, "III") == 0);
    assert(strcmp(board.scores[2].initials, "HHH") == 0);
    assert(strcmp(board.scores[3].initials, "GGG") == 0);
    assert(strcmp(board.scores[4].initials, "FFF") == 0);
    assert(strcmp(board.scores[5].initials, "EEE") == 0);
    assert(strcmp(board.scores[6].initials, "DDD") == 0);
    assert(strcmp(board.scores[7].initials, "   ") == 0);
    assert(strcmp(board.scores[8].initials, "B B") == 0);
    assert(strcmp(board.scores[9].initials, "KKK") == 0);

    // is_high_score should tell when a score isn't high enough for the
    // scoreboard.
    assert(!is_high_score(&board, 1));

    // insert_scoreboard shouldn't add scores that are too low.
    assert(!insert_scoreboard(&board, 0, "LLL"));
    assert(!insert_scoreboard(&board, 1, "LLL"));

    // stringify_scoreboard should convert the scoreboard to a string.
    char str[SCOREBOARD_STR_SIZE] = {0};
    size_t str_length = 0;
    assert(stringify_scoreboard(&board, str, sizeof(str), &str_length));
    assert(str_length == SCOREBOARD_STR_SIZE - 1);

    // stringify_scoreboard should fail convert the scoreboard to a string that
    // is too small.
    char too_small_str[10] = {0};
    size_t too_small_str_length = 0;
    assert(!stringify_scoreboard(&board, too_small_str, sizeof(too_small_str),
                                 &too_small_str_length));
    assert(too_small_str[0] == '\0');
    assert(too_small_str_length == 0);

    assert(strcmp(str, " 1.     10 JJJ\n"
                       " 2.      9 III\n"
                       " 3.      8 HHH\n"
                       " 4.      7 GGG\n"
                       " 5.      6 FFF\n"
                       " 6.      5 EEE\n"
                       " 7.      4 DDD\n"
                       " 8.      3    \n"
                       " 9.      2 B B\n"
                       "10.      2 KKK\n") == 0);

    // parse_scoreboard should convert a string created by stringify_scoreboard
    // back to a scoreboard.
    Scoreboard board_from_str = {0};
    assert(parse_scoreboard(&board_from_str, str));

    // The scores should be in the correct order.
    assert(strcmp(board_from_str.scores[0].initials,
                  board.scores[0].initials) == 0);
    assert(strcmp(board_from_str.scores[1].initials,
                  board.scores[1].initials) == 0);
    assert(strcmp(board_from_str.scores[2].initials,
                  board.scores[2].initials) == 0);
    assert(strcmp(board_from_str.scores[3].initials,
                  board.scores[3].initials) == 0);
    assert(strcmp(board_from_str.scores[4].initials,
                  board.scores[4].initials) == 0);
    assert(strcmp(board_from_str.scores[5].initials,
                  board.scores[5].initials) == 0);
    assert(strcmp(board_from_str.scores[6].initials,
                  board.scores[6].initials) == 0);
    assert(strcmp(board_from_str.scores[7].initials,
                  board.scores[7].initials) == 0);
    assert(strcmp(board_from_str.scores[8].initials,
                  board.scores[8].initials) == 0);
    assert(strcmp(board_from_str.scores[9].initials,
                  board.scores[9].initials) == 0);

    // The scores should have the correct number of points.
    assert(board_from_str.scores[0].points == board.scores[0].points);
    assert(board_from_str.scores[1].points == board.scores[1].points);
    assert(board_from_str.scores[2].points == board.scores[2].points);
    assert(board_from_str.scores[3].points == board.scores[3].points);
    assert(board_from_str.scores[4].points == board.scores[4].points);
    assert(board_from_str.scores[5].points == board.scores[5].points);
    assert(board_from_str.scores[6].points == board.scores[6].points);
    assert(board_from_str.scores[7].points == board.scores[7].points);
    assert(board_from_str.scores[8].points == board.scores[8].points);
    assert(board_from_str.scores[9].points == board.scores[9].points);

    // parse_scoreboard should fail to convert invalid strings.
    Scoreboard board_from_invalid_str = {0};
    const char *invalid_strs[] = {"111111111", " 1.", " 1.     10", "JJJ"};
    for (size_t i = 0; i < ARRAY_SIZE(invalid_strs); i++) {
        assert(!parse_scoreboard(&board_from_invalid_str, invalid_strs[i]));
    }

    return EXIT_SUCCESS;
}
