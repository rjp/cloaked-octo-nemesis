/* Generate a Poisson Disc using Bridson */
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <math.h>

#include "landscape.h"

#ifdef DEBUG_OUTPUT
#define debug printf
#else
#define debug(...)
#endif

int
random_under(int max)
{
    long r_val = random();
    return (int)(r_val % max);
}

struct pq_entry *
new_point(int x, int y)
{
    struct pq_entry *f = malloc(sizeof(struct pq_entry));

    f->p.x = x; f->p.y = y;

    return f;
}

struct pq_entry *
new_random_point(int width, int height)
{
    int first_x = random_under(width), first_y = random_under(height);

    return new_point(first_x, first_y);
}

void
generate_samples(int width, int height, struct active_h *p_active, struct inactive_h *p_inactive, int annulus)
{
    int howmany_active = 0;
    struct pq_entry *np;

    if (TAILQ_EMPTY(p_active)) {
        /* We start with a single random active point */
        struct pq_entry *f = new_random_point(width, height);
        TAILQ_INSERT_HEAD(p_active, f, entries);
        howmany_active = 1;
    }
    else {
        TAILQ_FOREACH(np, p_active, entries) {
            howmany_active++;
        }
    }

    /* And then we run until we have no more active points */
    while ( ! TAILQ_EMPTY(p_active) ) {
        /* Pick a random point on the active list */
        int random_active = random_under(howmany_active), i=0;
        struct pq_entry *live;

        TAILQ_FOREACH(np, p_active, entries) {
            if (i == random_active) { live = np; break; }
            i++;
        }
        debug("LP %d/%d <%d,%d>\n", random_active, howmany_active, live->p.x, live->p.y);
        int ax = live->p.x, ay = live->p.y;

        /* Ok, now we have our random active point in `live`, we generate
         * a bunch of candidate points in the surrounding annulus and
         * return the first one that's ok to use.  If we don't find any,
         * then we invalid the parent and loop again.
         */
        int found_one = 0;

        for(i=0; i<30; i++) {
            int radius = annulus + random_under(annulus);
            double angle = 2 * M_PI * (random_under(360)/360.0);
            int ox = radius * cos(angle), oy = radius * sin(angle);
            int cx = ax + ox, cy = ay + oy;

            /* Only bother if our new point is fully in our bounds */
            if (cx >= 0 && cx < width && cy >= 0 && cy < height) {
                int safe_distance_count = 0;
                int tested_points = 0;

                TAILQ_FOREACH(np, p_active, entries) {
                    int tx = np->p.x, ty = np->p.y;
                    int d = (tx-cx)*(tx-cx) + (ty-cy)*(ty-cy);
debug("DD %d/%d <%d,%d> = <%d,%d> (%d,%d)\n", d, annulus*annulus, cx,cy, ax,ay, tx-cx,ty-cy);
                    if (d > annulus*annulus) {
                        safe_distance_count++;
                    }
                    else {
                        debug("CA %i <%d,%d> p<%d,%d> REJECTED (%d < %d)\n", i, cx,cy, ax,ay, d, annulus*annulus);
                    }
                    tested_points++;
                }

                TAILQ_FOREACH(np, p_inactive, entries) {
                    int tx = np->p.x, ty = np->p.y;
                    int d = (tx-cx)*(tx-cx) + (ty-cy)*(ty-cy);
debug("DD %d/%d <%d,%d> = <%d,%d> (%d,%d)\n", d, annulus*annulus, cx,cy, ax,ay, tx-cx,ty-cy);
                    if (d > annulus*annulus) {
                        safe_distance_count++;
                    }
                    else {
                        debug("CI %i <%d,%d> p<%d,%d> REJECTED (%d < %d)\n", i, cx,cy, ax,ay, d, annulus*annulus);
                    }
                    tested_points++;
                }

                /* If our candidate is far enough away from all the
                 * existing points we have, keep it
                 */
                if (safe_distance_count == tested_points) {
                    struct pq_entry *f = new_point(cx, cy);
                    found_one = 1;
                    debug("GP %i <%d,%d> parent=<%d,%d> ACCEPTED\n", i, cx, cy, ax, ay);
                    i = 31;
                    TAILQ_INSERT_TAIL(p_active, f, entries);
                    howmany_active++;
                    break;
                }
            }
        }

        if (! found_one) {
            debug("IN parent <%d,%d> no candidates DEACTIVATED\n", ax,ay);
            TAILQ_REMOVE(p_active, live, entries);
            TAILQ_INSERT_HEAD(p_inactive, live, entries);
            howmany_active--;
        }
        debug("HA %d\n", howmany_active);
    }
}
