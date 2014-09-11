#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>

#include "my-ppm.h"
#include "bridson.h"
#include "landscape.h"

struct active_h active = TAILQ_HEAD_INITIALIZER(active);
struct inactive_h inactive = TAILQ_HEAD_INITIALIZER(inactive);

rgb canvas[512][512];

rgb colour_black = { 0,0,0 };
rgb colour_red   = { 255,0,0 };
rgb colour_green = { 0,255,0 };
rgb colour_blue  = { 0,0,255 };
rgb colour_yellow = { 255,255,0 };

rgb
greyscale(int z, int max_z)
{
    int g = (255*z)/max_z;
    return (rgb){ g,g,g };
}

int
main(void)
{
    int i,j;
    int max_val = 40 + (40/2);

    for(i=0; i<512; i++) {
        for(j=0; j<512; j++) {
            canvas[i][j] = colour_black;
        }
    }
    /* Start with a completely random state for the minute */
    srandomdev();

    /* Initialise our lists */
    TAILQ_INIT(&active);
    TAILQ_INIT(&inactive);

    /* First we generate a coarse red grid */
    generate_samples(512, 512, &active, &inactive, 40, 0);

    TAILQ_FOREACH_SAFE(np, &inactive, entries, np_temp) {
        /* canvas[np->p.y][np->p.x] = colour_red; */
        canvas[np->p.y][np->p.x] = greyscale(np->p.z, max_val);
        TAILQ_INSERT_HEAD(&active, np, entries);
        TAILQ_REMOVE(&inactive, np, entries);
    }

    generate_samples(512, 512, &active, &inactive, 20, 1);

    TAILQ_FOREACH_SAFE(np, &inactive, entries, np_temp) {
        /* canvas[np->p.y][np->p.x] = colour_green; */
        canvas[np->p.y][np->p.x] = greyscale(np->p.z, max_val);
        TAILQ_INSERT_HEAD(&active, np, entries);
        TAILQ_REMOVE(&inactive, np, entries);
    }

    generate_samples(512, 512, &active, &inactive, 10, 1);

    TAILQ_FOREACH_SAFE(np, &inactive, entries, np_temp) {
        canvas[np->p.y][np->p.x] = greyscale(np->p.z, max_val);
        TAILQ_INSERT_HEAD(&active, np, entries);
        TAILQ_REMOVE(&inactive, np, entries);
    }

    output_ppm(stderr, 512, 512, canvas);
}
