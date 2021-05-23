#pragma once

#include <SDL.h>

// Language-specific strings.
enum String {
    STRING_COIN_PLAY,
    STRING_ENTER_INITIALS,
    STRING_GAME_OVER,
    STRING_HIGHSCORES,
    STRING_LANGUAGE,
    STRING_PLAYER,
    STRING_PUSH_START,
    NUM_STRINGS,
};

typedef enum String String;

// NOTE: Languages should be in alphabetical order.
enum Language {
    LANGUAGE_ENGLISH,
    LANGUAGE_PORTUGUESE,
    NUM_LANGUAGES,
};

typedef enum Language Language;

extern const char *const langs_strings[NUM_LANGUAGES][NUM_STRINGS];

Language get_user_language(Language fallback);
