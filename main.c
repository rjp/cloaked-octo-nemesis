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
greyscale(double z, int max_z)
{
    int g = (255*z)/max_z;
    return (rgb){ g,g,g };
}

int
main(void)
{
    int i,j;
    double max_val = 1.0;
    double generation = 40;
    int gen_count = 0;

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
    generate_samples(512, 512, &active, &inactive, 8, 0, 0);

    while (generation > 49.0) {
        double min_height = 99999.0, max_height = -99999.0;
        generation = generation * 0.75;
        gen_count++;

        printf("g=%.2f AC=%d IN=%d\n", generation, TAILQ_EMPTY(&active), TAILQ_EMPTY(&inactive));

        TAILQ_FOREACH(np, &inactive, entries) {
            if (np->p.z < min_height) { min_height = np->p.z; }
            if (np->p.z > max_height) { max_height = np->p.z; }
        }
        TAILQ_CONCAT(&active, &inactive, entries);

        printf("AC=%d IN=%d\n", TAILQ_EMPTY(&active), TAILQ_EMPTY(&inactive));

        printf("MIN:MAX = %.3f, %.3f\n", min_height, max_height);

        generate_samples(512, 512, &active, &inactive, generation, 1, gen_count);
    }

    point buffer[8192];
    double min_height = 99999.0, max_height = -99999.0;

    TAILQ_FOREACH_SAFE(np, &inactive, entries, np_temp) {
    //    canvas[np->p.y][np->p.x] = greyscale(np->p.z, max_val);
        buffer[i] = np->p;
        i++;
        if (np->p.z < min_height) { min_height = np->p.z; }
        if (np->p.z > max_height) { max_height = np->p.z; }
    }
    int howmany = i;
    int last_point;

    for(i=0; i<512; i++) {
        for(j=0; j<512; j++) {
            double min = 512*512*2;
            point mp = {};
            for(int q=0; q<howmany; q++) {
                int px = buffer[q].x, py = buffer[q].y;
                int d = (px-j)*(px-j) + (py-i)*(py-i);
                if (d < min) {
                    min = d; mp = buffer[q]; last_point = q;
                }
            }
            if (mp.c.r == -1) { // no assigned colour
                buffer[last_point].c = colour_by_height(mp.z / max_val);
                canvas[i][j] = buffer[last_point].c;
                printf("C for <%d,%d> is {%d,%d,%d}\n", buffer[last_point].x, buffer[last_point].y, buffer[last_point].c.r, buffer[last_point].c.g, buffer[last_point].c.b);
            }
            else {
                canvas[i][j] = mp.c;
            }
        }
    }

    output_ppm(stderr, 512, 512, canvas);
    printf("heights = %.3f to %.3f (%.3f to %.3f)\n", min_height, max_height, min_height / max_val, max_height / max_val);
}
