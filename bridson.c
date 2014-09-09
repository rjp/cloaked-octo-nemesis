/* Generate a Poisson Disc using Bridson */
#include <stdio.h>
#include <stdlib.h>

/* We need to keep track of active vs inactive points */
typedef struct { int x; int y; } point;

int
random_under(int max)
{
    long r_val = random();
    return (int)(r_val % max);
}

void
generate_samples(int width, int height, int output[height][width], int annulus)
{
    /* Currently we fake this by generating a dreadful uniform random
     * sample of points based on `width` * `height`.
     */
    int dots = (width * height) / 20, i;

    for(i=0; i<dots; i++) {
        int r_x = random_under(width), r_y = random_under(height);
        output[r_y][r_x] = 255;
    }
}
