#include "spacerocks/font.h"

enum {
    CHAR_ARRAY_SIZE = 5, // multi-byte char. with space for a null term.
};

enum GlyphChar {
    GLYPH_CHAR_RECTANGLE, // substitute character
    GLYPH_CHAR_SPACE,
    GLYPH_CHAR_PERIOD,
    GLYPH_CHAR_0,
    GLYPH_CHAR_1,
    GLYPH_CHAR_2,
    GLYPH_CHAR_3,
    GLYPH_CHAR_4,
    GLYPH_CHAR_5,
    GLYPH_CHAR_6,
    GLYPH_CHAR_7,
    GLYPH_CHAR_8,
    GLYPH_CHAR_9,
    GLYPH_CHAR_A,
    GLYPH_CHAR_A_TILDE,
    GLYPH_CHAR_B,
    GLYPH_CHAR_C,
    GLYPH_CHAR_C_CEDILLA,
    GLYPH_CHAR_D,
    GLYPH_CHAR_E,
    GLYPH_CHAR_E_ACUTE,
    GLYPH_CHAR_E_CIRCUMFLEX,
    GLYPH_CHAR_F,
    GLYPH_CHAR_G,
    GLYPH_CHAR_H,
    GLYPH_CHAR_I,
    GLYPH_CHAR_J,
    GLYPH_CHAR_K,
    GLYPH_CHAR_L,
    GLYPH_CHAR_M,
    GLYPH_CHAR_N,
    GLYPH_CHAR_O,
    GLYPH_CHAR_O_TILDE,
    GLYPH_CHAR_P,
    GLYPH_CHAR_Q,
    GLYPH_CHAR_R,
    GLYPH_CHAR_S,
    GLYPH_CHAR_T,
    GLYPH_CHAR_U,
    GLYPH_CHAR_V,
    GLYPH_CHAR_W,
    GLYPH_CHAR_X,
    GLYPH_CHAR_Y,
    GLYPH_CHAR_Z,
    GLYPH_CHAR_UNDERSCORE,
    GLYPH_CHAR_COPYRIGHT,
    GLYPH_CHAR_VERTICAL_BAR,
    NUM_GLYPH_CHARS,
};

typedef enum GlyphChar GlyphChar;

struct GlyphCharCache {
    char char_array[CHAR_ARRAY_SIZE]; // null terminated
    size_t length;                    // excluding null term.
};

typedef struct GlyphCharCache GlyphCharCache;

struct Glyph {
    Lines lines;
    float half_width; // for calculating the space between glyphs
};

typedef struct Glyph Glyph;

// NOTE: I've chosen to not initialize this at runtime to avoid mutable global
// state. Not a particularly pretty sight.
static const GlyphCharCache char_to_glyph_char_cache_map[NUM_GLYPH_CHARS] = {
    [GLYPH_CHAR_RECTANGLE] = {"▯", sizeof("▯") - 1},
    [GLYPH_CHAR_SPACE] = {" ", sizeof(" ") - 1},
    [GLYPH_CHAR_PERIOD] = {".", sizeof(".") - 1},
    [GLYPH_CHAR_0] = {"0", sizeof("0") - 1},
    [GLYPH_CHAR_1] = {"1", sizeof("1") - 1},
    [GLYPH_CHAR_2] = {"2", sizeof("2") - 1},
    [GLYPH_CHAR_3] = {"3", sizeof("3") - 1},
    [GLYPH_CHAR_4] = {"4", sizeof("4") - 1},
    [GLYPH_CHAR_5] = {"5", sizeof("5") - 1},
    [GLYPH_CHAR_6] = {"6", sizeof("6") - 1},
    [GLYPH_CHAR_7] = {"7", sizeof("7") - 1},
    [GLYPH_CHAR_8] = {"8", sizeof("8") - 1},
    [GLYPH_CHAR_9] = {"9", sizeof("9") - 1},
    [GLYPH_CHAR_A] = {"A", sizeof("A") - 1},
    [GLYPH_CHAR_A_TILDE] = {"Ã", sizeof("Ã") - 1},
    [GLYPH_CHAR_B] = {"B", sizeof("B") - 1},
    [GLYPH_CHAR_C] = {"C", sizeof("C") - 1},
    [GLYPH_CHAR_C_CEDILLA] = {"Ç", sizeof("Ç") - 1},
    [GLYPH_CHAR_D] = {"D", sizeof("D") - 1},
    [GLYPH_CHAR_E] = {"E", sizeof("E") - 1},
    [GLYPH_CHAR_E_ACUTE] = {"É", sizeof("É") - 1},
    [GLYPH_CHAR_E_CIRCUMFLEX] = {"Ê", sizeof("Ê") - 1},
    [GLYPH_CHAR_F] = {"F", sizeof("F") - 1},
    [GLYPH_CHAR_G] = {"G", sizeof("G") - 1},
    [GLYPH_CHAR_H] = {"H", sizeof("H") - 1},
    [GLYPH_CHAR_I] = {"I", sizeof("I") - 1},
    [GLYPH_CHAR_J] = {"J", sizeof("J") - 1},
    [GLYPH_CHAR_K] = {"K", sizeof("K") - 1},
    [GLYPH_CHAR_L] = {"L", sizeof("L") - 1},
    [GLYPH_CHAR_M] = {"M", sizeof("M") - 1},
    [GLYPH_CHAR_N] = {"N", sizeof("N") - 1},
    [GLYPH_CHAR_O] = {"O", sizeof("O") - 1},
    [GLYPH_CHAR_O_TILDE] = {"Õ", sizeof("Õ") - 1},
    [GLYPH_CHAR_P] = {"P", sizeof("P") - 1},
    [GLYPH_CHAR_Q] = {"Q", sizeof("Q") - 1},
    [GLYPH_CHAR_R] = {"R", sizeof("R") - 1},
    [GLYPH_CHAR_S] = {"S", sizeof("S") - 1},
    [GLYPH_CHAR_T] = {"T", sizeof("T") - 1},
    [GLYPH_CHAR_U] = {"U", sizeof("U") - 1},
    [GLYPH_CHAR_V] = {"V", sizeof("V") - 1},
    [GLYPH_CHAR_W] = {"W", sizeof("W") - 1},
    [GLYPH_CHAR_X] = {"X", sizeof("X") - 1},
    [GLYPH_CHAR_Y] = {"Y", sizeof("Y") - 1},
    [GLYPH_CHAR_Z] = {"Z", sizeof("Z") - 1},
    [GLYPH_CHAR_UNDERSCORE] = {"_", sizeof("_") - 1},
    [GLYPH_CHAR_COPYRIGHT] = {"©", sizeof("©") - 1},
    [GLYPH_CHAR_VERTICAL_BAR] = {"|", sizeof("|") - 1},
};

// NOTE: This took a little while to write and you're welcome to copy it.
// TODO: Remove duplicate lines which are leftovers from when the points were
// passed directly to SDL_RenderDrawLinesF.
static const Glyph glyph_char_to_glyph_map[NUM_GLYPH_CHARS] = {
    [GLYPH_CHAR_RECTANGLE] =
        {
            {{{-0.5f, -1.0f},
              {-0.5f, 1.0f},
              {-0.5f, 1.0f},
              {0.5f, 1.0f},
              {0.5f, 1.0f},
              {0.5f, -1.0f},
              {0.5f, -1.0f},
              {-0.5f, -1.0f}},
             8,
             0},
            0.5f,
        },
    [GLYPH_CHAR_SPACE] =
        {
            {{{0.0f, 0.0f}}, 0, 0},
            0.7f,
        },
    [GLYPH_CHAR_PERIOD] =
        {
            {{{0.0f, 0.9f}, {0.0f, 1.0f}}, 2, 0},
            0.7f,
        },
    [GLYPH_CHAR_0] =
        {
            {{
                 {-0.7f, -1.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {-0.7f, -1.0f},
             },
             8,
             0},
            0.7f,
        },
    [GLYPH_CHAR_1] =
        {
            {{
                 {0.7f, -1.0f},
                 {0.7f, 1.0f},
             },
             2,
             0},
            0.7f,
        },
    [GLYPH_CHAR_2] =
        {
            {{
                 {-0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.7f, 0.0f},
                 {0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {0.7f, 1.0f},
             },
             10,
             0},
            0.7f,
        },
    [GLYPH_CHAR_3] =
        {
            {{
                 {-0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.7f, 0.0f},
                 {0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {0.7f, 0.0f},
                 {0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {-0.7f, 1.0f},
             },
             10,
             0},
            0.7f,
        },
    [GLYPH_CHAR_4] =
        {
            {{
                 {-0.7f, -1.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {0.7f, 0.0f},
                 {0.7f, -1.0f},
                 {0.7f, 1.0f},
             },
             6,
             0},
            0.7f,
        },
    [GLYPH_CHAR_5] =
        {
            {{
                 {0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {0.7f, 0.0f},
                 {0.7f, 0.0f},
                 {0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {-0.7f, 1.0f},
             },
             10,
             0},
            0.7f,
        },
    [GLYPH_CHAR_6] =
        {
            {{
                 {-0.7f, -1.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {0.7f, 0.0f},
                 {0.7f, 0.0f},
                 {-0.7f, 0.0f},
             },
             8,
             0},
            0.7f,
        },
    [GLYPH_CHAR_7] =
        {
            {{
                 {-0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.7f, 1.0f},
             },
             4,
             0},
            0.7f,
        },
    [GLYPH_CHAR_8] =
        {
            {{
                 {0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {0.7f, -1.0f},
                 {0.7f, 0.0f},
                 {-0.7f, 0.0f},
             },
             10,
             0},
            0.7f,
        },
    [GLYPH_CHAR_9] =
        {
            {{
                 {0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.7f, 1.0f},
             },
             8,
             0},
            0.7f,
        },
    [GLYPH_CHAR_A] =
        {
            {{
                 {-0.7f, 1.0f},
                 {-0.7f, 0.3f},
                 {-0.7f, 0.3f},
                 {0.7f, 0.3f},
                 {0.7f, 0.3f},
                 {-0.7f, 0.3f},
                 {-0.7f, 0.3f},
                 {-0.7f, -0.4f},
                 {-0.7f, -0.4f},
                 {0.0f, -1.0f},
                 {0.0f, -1.0f},
                 {0.7f, -0.4f},
                 {0.7f, -0.4f},
                 {0.7f, 1.0f},
             },
             14,
             0},
            0.7f,
        },
    [GLYPH_CHAR_A_TILDE] =
        {
            {{
                 {-0.7f, 1.0f},  {-0.7f, 0.3f},  {-0.7f, 0.3f}, {0.7f, 0.3f},
                 {0.7f, 0.3f},   {-0.7f, 0.3f},  {-0.7f, 0.3f}, {-0.7f, -0.4f},
                 {-0.7f, -0.4f}, {0.0f, -1.0f},  {0.0f, -1.0f}, {0.7f, -0.4f},
                 {0.7f, -0.4f},  {0.7f, 1.0f},

                 {-0.7f, -1.6f}, {-0.7f, -1.3f},

                 {-0.7f, -1.3f}, {0.7f, -1.6f},

                 {0.7f, -1.6f},  {0.7f, -1.3f},
             },
             20,
             0},
            0.7f,
        },
    [GLYPH_CHAR_B] =
        {
            {{
                 {-0.7f, -1.0f}, {0.35f, -1.0f}, {0.35f, -1.0f}, {0.7f, -0.6f},
                 {0.7f, -0.6f},  {0.7f, -0.4f},  {0.7f, -0.4f},  {0.35f, 0.0f},
                 {0.35f, 0.0f},  {-0.7f, 0.0f},  {-0.7f, 0.0f},  {0.35f, 0.0f},
                 {0.35f, 0.0f},  {0.7f, 0.4f},   {0.7f, 0.4f},   {0.7f, 0.6f},
                 {0.7f, 0.6f},   {0.35f, 1.0f},  {0.35f, 1.0f},  {-0.7f, 1.0f},
                 {-0.7f, 1.0f},  {-0.7f, -1.0f},
             },
             22,
             0},
            0.7f,
        },
    [GLYPH_CHAR_C] =
        {
            {{
                 {0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {0.7f, 1.0f},
             },
             6,
             0},
            0.7f,
        },
    [GLYPH_CHAR_C_CEDILLA] =
        {
            {{
                 {0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {0.7f, 1.0f},

                 {0.0f, 1.0f},
                 {-0.3f, 1.4f},

                 {-0.3f, 1.4f},
                 {-0.7f, 1.4f},
             },
             10,
             0},
            0.7f,
        },
    [GLYPH_CHAR_D] =
        {
            {{
                 {-0.7f, -1.0f},
                 {0.21f, -1.0f},
                 {0.21f, -1.0f},
                 {0.7f, -0.5f},
                 {0.7f, -0.5f},
                 {0.7f, 0.5f},
                 {0.7f, 0.5f},
                 {0.21f, 1.0f},
                 {0.21f, 1.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, -1.0f},
             },
             12,
             0},
            0.7f,
        },
    [GLYPH_CHAR_E] =
        {
            {{
                 {0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {0.35f, 0.0f},
                 {0.35f, 0.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {0.7f, 1.0f},
             },
             12,
             0},
            0.7f,
        },
    [GLYPH_CHAR_E_ACUTE] =
        {
            {{
                 {0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {0.35f, 0.0f},
                 {0.35f, 0.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {0.7f, 1.0f},

                 {-0.5f, -1.2f},
                 {0.5f, -1.5f},
             },
             14,
             0},
            0.7f,
        },
    [GLYPH_CHAR_E_CIRCUMFLEX] =
        {
            {{
                 {0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {0.35f, 0.0f},
                 {0.35f, 0.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {0.7f, 1.0f},

                 {-0.6f, -1.2f},
                 {0.0f, -1.5f},
                 {0.0f, -1.5f},
                 {0.6f, -1.2f},
             },
             16,
             0},
            0.7f,
        },
    [GLYPH_CHAR_F] =
        {
            {{
                 {0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {0.35f, 0.0f},
                 {0.35f, 0.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 1.0f},
             },
             10,
             0},
            0.7f,
        },
    [GLYPH_CHAR_G] =
        {
            {{
                 {0.7f, -0.5f},
                 {0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {0.7f, 0.25f},
                 {0.7f, 0.25f},
                 {0.0f, 0.25f},
             },
             12,
             0},
            0.7f,
        },
    [GLYPH_CHAR_H] =
        {
            {{
                 {-0.7f, -1.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {0.7f, 0.0f},
                 {0.7f, 0.0f},
                 {0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.7f, 1.0f},
             },
             10,
             0},
            0.7f,
        },
    [GLYPH_CHAR_I] =
        {
            {{
                 {-0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.0f, -1.0f},
                 {0.0f, -1.0f},
                 {0.0f, 1.0f},
                 {0.0f, 1.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {0.7f, 1.0f},
             },
             10,
             0},
            0.7f,
        },
    [GLYPH_CHAR_J] =
        {
            {{
                 {0.7f, -1.0f},
                 {0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {0.0f, 1.0f},
                 {0.0f, 1.0f},
                 {-0.7f, 0.5f},
             },
             6,
             0},
            0.7f,
        },
    [GLYPH_CHAR_K] =
        {
            {{
                 {0.7f, -1.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {0.7f, 1.0f},
             },
             10,
             0},
            0.7f,
        },
    [GLYPH_CHAR_L] =
        {
            {{
                 {-0.7f, -1.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {0.7f, 1.0f},
             },
             4,
             0},
            0.7f,
        },
    [GLYPH_CHAR_M] =
        {
            {{
                 {-0.7f, 1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {0.0f, 0.0f},
                 {0.0f, 0.0f},
                 {0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.7f, 1.0f},
             },
             8,
             0},
            0.7f,
        },
    [GLYPH_CHAR_N] =
        {
            {{
                 {-0.7f, 1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {0.7f, -1.0f},
             },
             6,
             0},
            0.7f,
        },
    [GLYPH_CHAR_O] =
        {
            {{
                 {-0.7f, -1.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {-0.7f, -1.0f},
             },
             8,
             0},
            0.7f,
        },
    [GLYPH_CHAR_O_TILDE] =
        {
            {{
                 {-0.7f, -1.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {-0.7f, -1.0f},

                 {-0.7f, -1.6f},
                 {-0.7f, -1.3f},

                 {-0.7f, -1.3f},
                 {0.7f, -1.6f},

                 {0.7f, -1.6f},
                 {0.7f, -1.3f},
             },
             14,
             0},
            0.7f,
        },
    [GLYPH_CHAR_P] =
        {
            {{
                 {-0.7f, 1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.7f, 0.0f},
                 {0.7f, 0.0f},
                 {-0.7f, 0.0f},
             },
             8,
             0},
            0.7f,
        },
    [GLYPH_CHAR_Q] =
        {
            {{
                 {0.7f, -1.0f},
                 {-0.7f, -1.0f},

                 {-0.7f, -1.0f},
                 {-0.7f, 1.0f},

                 {-0.7f, 1.0f},
                 {0.0f, 1.0f},

                 {0.0f, 1.0f},
                 {0.7f, 0.2f},

                 {0.7f, 0.2f},
                 {0.7f, -1.0f},

                 {0.0f, 0.2f},
                 {0.7f, 1.0f},
             },
             12,
             0},
            0.7f,
        },
    [GLYPH_CHAR_R] =
        {
            {{
                 {-0.7f, 1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.7f, 0.0f},
                 {0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {-0.3f, 0.0f},
                 {-0.3f, 0.0f},
                 {0.7f, 1.0f},
             },
             12,
             0},
            0.7f,
        },
    [GLYPH_CHAR_S] =
        {
            {{
                 {0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, -1.0f},
                 {-0.7f, 0.0f},
                 {-0.7f, 0.0f},
                 {0.7f, 0.0f},
                 {0.7f, 0.0f},
                 {0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {-0.7f, 1.0f},
             },
             10,
             0},
            0.7f,
        },
    [GLYPH_CHAR_T] =
        {
            {{
                 {-0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.0f, -1.0f},
                 {0.0f, -1.0f},
                 {0.0f, 1.0f},
             },
             6,
             0},
            0.7f,
        },
    [GLYPH_CHAR_U] =
        {
            {{
                 {-0.7f, -1.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {0.7f, -1.0f},
             },
             6,
             0},
            0.7f,
        },
    [GLYPH_CHAR_V] =
        {
            {{
                 {-0.7f, -1.0f},
                 {0.0f, 1.0f},
                 {0.0f, 1.0f},
                 {0.7f, -1.0f},
             },
             4,
             0},
            0.7f,
        },
    [GLYPH_CHAR_W] =
        {
            {{
                 {-0.7f, -1.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {0.0f, 0.0f},
                 {0.0f, 0.0f},
                 {0.7f, 1.0f},
                 {0.7f, 1.0f},
                 {0.7f, -1.0f},
             },
             8,
             0},
            0.7f,
        },
    [GLYPH_CHAR_X] =
        {
            {{
                 {-0.5f, -1.0f},
                 {0.5f, 1.0f},
                 {0.5f, 1.0f},
                 {0.0f, 0.0f},
                 {0.0f, 0.0f},
                 {0.5f, -1.0f},
                 {0.5f, -1.0f},
                 {-0.5f, 1.0f},
             },
             8,
             0},
            0.7f,
        },
    [GLYPH_CHAR_Y] =
        {
            {{
                 {-0.7f, -1.0f},
                 {0.0f, -0.2f},
                 {0.0f, -0.2f},
                 {0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.0f, -0.2f},
                 {0.0f, -0.2f},
                 {0.0f, 1.0f},
             },
             8,
             0},
            0.7f,
        },
    [GLYPH_CHAR_Z] =
        {
            {{
                 {-0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {0.7f, -1.0f},
                 {-0.7f, 1.0f},
                 {-0.7f, 1.0f},
                 {0.7f, 1.0f},
             },
             6,
             0},
            0.7f,
        },
    [GLYPH_CHAR_UNDERSCORE] =
        {
            {{
                 {-0.7f, 1.0f},
                 {0.7f, 1.0f},
             },
             2,
             0},
            0.7f,
        },
    [GLYPH_CHAR_COPYRIGHT] =
        {
            {{
                 {0.0f, -1.0f}, {-1.0f, -0.8f}, {-1.0f, -0.8f}, {-1.0f, 0.8f},
                 {-1.0f, 0.8f}, {0.0f, 1.0f},   {0.0f, 1.0f},   {1.0f, 0.8f},
                 {1.0f, 0.8f},  {1.0f, -0.8f},  {1.0f, -0.8f},  {0.0f, -1.0f},
                 {0.4f, -0.4f}, {-0.4f, -0.4f}, {-0.4f, -0.4f}, {-0.4f, 0.4f},
                 {-0.4f, 0.4f}, {0.4f, 0.4f},   {0.4f, 0.4f},
             },
             18,
             0},
            1.0f,
        },
    [GLYPH_CHAR_VERTICAL_BAR] =
        {
            {{{0.0f, -1.0f}, {0.0f, 1.0f}}, 2, 0},
            0.7f,
        },
};

static const float glyph_gap_factor = 0.25f;
static const float line_gap_factor = 0.5f;

static size_t get_glyph_char_from_str(const char *str, GlyphChar *ch);
static void debug_render_text_outline(RendererWrapper *renderer,
                                      Vec2Df position, float height,
                                      const char *str);

// TODO: Consider converting the given string to an array of 32-bit code points
// to be used for looking glyphs up, for the sake of performance.
void render_text(RendererWrapper *renderer, Vec2Df position, float height,
                 const char *str, SDL_Color color) {
    SDL_assert(str != NULL);
    if (str == NULL) {
        return;
    }

    Vec2Df line = {0};
    for (size_t i = 0; i < strlen(str); i++) {
        if (str[i] == '\n') {
            line.x = 0.0f;
            line.y += height + (height * line_gap_factor);
            continue;
        }

        GlyphChar ch = 0;
        size_t chars_read = get_glyph_char_from_str(&str[i], &ch);
        Glyph glyph = glyph_char_to_glyph_map[ch];
        float width = height * glyph.half_width;
        float gap = width * glyph_gap_factor;

        if (line.x > 0) {
            line.x += gap;
        }

        float scale = height / 2.0f;
        Vec2Df char_pos = {
            .x = position.x + line.x + (width / 2.0f),
            .y = position.y + line.y + scale,
        };
        transform_lines(&glyph.lines, char_pos, 0.0f, scale);

        render_lines(renderer, &glyph.lines, color);

        line.x += width + gap;

        if (chars_read > 0) {
            i += chars_read - 1;
        }
    }

    if (DEBUG_RENDERED_TEXT_SIZE) {
        debug_render_text_outline(renderer, position, height, str);
    }
}

static size_t get_glyph_char_from_str(const char *str, GlyphChar *ch) {
    for (int i = 0; i < NUM_GLYPH_CHARS; i++) {
        GlyphCharCache cache = char_to_glyph_char_cache_map[i];
        if (strncmp(str, cache.char_array, cache.length) == 0) {
            *ch = i;
            return cache.length;
        }
    }
    return 0;
}

// Return the space that the given string will take when rendered by
// render_text (excluding diacritics).
Vec2Df get_rendered_text_size(float height, const char *str) {
    if (str == NULL) {
        return (Vec2Df){0};
    }

    Vec2Df size = {0.0f, height + 1.0f};
    float line_width = 0.0f;
    size_t str_len = strlen(str);
    for (size_t i = 0; i < str_len; i++) {
        if (str[i] == '\n') {
            line_width = 0.0f;
            size.y += height + (height * line_gap_factor);
            continue;
        }

        GlyphChar ch = 0;
        int chars_read = get_glyph_char_from_str(&str[i], &ch);
        Glyph glyph = glyph_char_to_glyph_map[ch];
        float width = height * glyph.half_width;
        float gap = width * glyph_gap_factor;
        line_width += width;

        if (str_len > 1) {
            line_width += gap;
            if (i > 0 && i + 1 < str_len) {
                line_width += gap;
            }
        }

        if (line_width > size.x) {
            size.x = line_width + 1.0f;
        }

        if (chars_read > 0) {
            i += chars_read - 1;
        }
    }

    return size;
}

static void debug_render_text_outline(RendererWrapper *renderer,
                                      Vec2Df position, float height,
                                      const char *str) {
    uint8_t c[4] = {0};
    SDL_GetRenderDrawColor(renderer->renderer, &c[0], &c[1], &c[2], &c[3]);
    SDL_SetRenderDrawColor(renderer->renderer, 0, 255, 0, 255);
    Vec2Df size = get_rendered_text_size(height, str);
    Rect2Df rect = {position.x, position.y, size.x, size.y};
    rect = renderer_wrapper_scale_rect2df(renderer, rect);
    SDL_RenderDrawRectF(renderer->renderer, &rect);
    SDL_SetRenderDrawColor(renderer->renderer, c[0], c[1], c[2], c[3]);
}
