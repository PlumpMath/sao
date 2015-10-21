#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sao_math.h"

int
main(int argc, char* argv[])
{
    printf("You should write some more tests!\n");

    V2 foo;
    foo.x = 1;
    foo.y = 2;

    V2 bar;
    bar.x = 3;
    bar.y = 4;

    V2 baz = add(foo, bar);

    assert(baz.x == foo.x + bar.x);
    assert(baz.y == foo.y + bar.y);

    V2 wow = sub(foo, bar);

    assert(wow.x == foo.x - bar.x);
    assert(wow.y == foo.y - bar.y);

    V2 n = normalize(foo);

    float mag = sqrt(n.x * n.x + n.y * n.y);
    float eps = 0.0001;
    assert(mag <= 1+eps && mag >= 1-eps);

    V2 s = scale(foo, 2.0f);

    assert(s.x == foo.x * 2);
    assert(s.y == foo.y * 2);
}
