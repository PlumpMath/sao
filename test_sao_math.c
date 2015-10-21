#include <stdio.h>
#include <stdlib.h>

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

    printf("(%f,%f) + (%f,%f) = (%f,%f)\n",
           foo.x, foo.y, bar.x, bar.y, baz.x, baz.y);
}
