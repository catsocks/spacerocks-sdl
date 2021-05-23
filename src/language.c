#include "spacerocks/language.h"

// The characters included in these strings are meant to be passed to
// render_text so they must be present in the GlyphChar enumeration in text.c,
// otherwise a generic substitute character will be shown in their place.
const char *const langs_strings[NUM_LANGUAGES][NUM_STRINGS] = {
    [LANGUAGE_ENGLISH] =
        {
            [STRING_COIN_PLAY] = "1 COIN 1 PLAY",
            [STRING_ENTER_INITIALS] = "YOUR SCORE IS ONE OF THE TEN BEST\n"
                                      "PLEASE ENTER YOUR INITIALS\n"
                                      "PUSH ROTATE TO SELECT LETTER\n"
                                      "PUSH HYPERSPACE WHEN LETTER IS CORRECT",
            [STRING_GAME_OVER] = "GAME OVER",
            [STRING_HIGHSCORES] = "HIGH SCORES",
            [STRING_LANGUAGE] = "ENGLISH",
            [STRING_PLAYER] = "PLAYER",
            [STRING_PUSH_START] = "PUSH START",
        },
    [LANGUAGE_PORTUGUESE] =
        {
            [STRING_COIN_PLAY] = "1 MOEDA 1 JOGO",
            [STRING_ENTER_INITIALS] =
                "SUA PONTUAÇÃO É UMA DAS DEZ MELHORES\n"
                "POR FAVOR INSIRA SUAS INICIAIS\n"
                "PRESSIONE GIRAR PARA SELECIONAR A LETRA\n"
                "APERTE HYPERSPACE PARA CONFIRMAR A LETRA",
            [STRING_GAME_OVER] = "FIM DE JOGO",
            [STRING_HIGHSCORES] = "PONTUAÇÕES ALTAS",
            [STRING_LANGUAGE] = "PORTUGUÊS",
            [STRING_PLAYER] = "JOGADOR",
            [STRING_PUSH_START] = "APERTE INICIAR",
        },
};

static const char *const langs_code[NUM_LANGUAGES] = {
    [LANGUAGE_ENGLISH] = "en",
    [LANGUAGE_PORTUGUESE] = "pt",
};

// Return the user's preferred language or the given fallback.
Language get_user_language(Language fallback) {
    Language lang = fallback;
    // TODO: Remove this macro when the SDL version in the Emscripten Ports
    // collection is >= 2.0.14.
#if SDL_VERSION_ATLEAST(2, 0, 14)
    SDL_Locale *locale = SDL_GetPreferredLocales();
    if (locale == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't get preferred locales: %s", SDL_GetError());
        return lang;
    }
    for (int i = 0; i < NUM_LANGUAGES; i++) {
        if (strcmp(locale->language, langs_code[i]) == 0) {
            lang = i;
        }
    }
    SDL_free(locale);
#endif
    return lang;
}
