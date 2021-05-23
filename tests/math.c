#include <assert.h>
#include <math.h>

#include "spacerocks/math.h"

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    // wrap_around should wrap both positive and negative integers around.
    assert(wrap_around(0, 10) == 0);
    assert(wrap_around(11, 10) == 1);
    assert(wrap_around(-11, 10) == 9);

    // fwrap_around should wrap both positive and negative floats around.
    assert(roundf(fwrap_around(0.0f, 10.0f)) == 0.0f);
    assert(roundf(fwrap_around(11.0f, 10.0f)) == 1.0f);
    assert(roundf(fwrap_around(-11.0f, 10.0f)) == 9.0f);

    return EXIT_SUCCESS;
}
