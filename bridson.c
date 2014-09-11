/* Generate a Poisson Disc using Bridson */
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <math.h>
#include <assert.h>

#include "landscape.h"

#ifdef DEBUG_OUTPUT
#define debug printf
#else
#define debug(...)
#endif

/* 8k points should be enough as a buffer */

typedef struct { point p; int d; } buffer_entry;
buffer_entry buffer[8192];

int
sort_by_dist(const void *a, const void *b)
{
    buffer_entry *ba = (buffer_entry *)a;
    buffer_entry *bb = (buffer_entry *)b;

    return ba->d - bb->d;
}

int
find_nearest(int x, int y, struct active_h *p_active, struct inactive_h *p_inactive)
{
    int i = 0;

    /* First we stuff the points from the linked lists into the buffer */
    TAILQ_FOREACH(np, p_active, entries) {
        int tx = np->p.x, ty = np->p.y;
        int d = (tx-x)*(tx-x) + (ty-y)*(ty-y);
        buffer[i] = (buffer_entry){ np->p, d };
        i++;
    }

    TAILQ_FOREACH(np, p_inactive, entries) {
        int tx = np->p.x, ty = np->p.y;
        int d = (tx-x)*(tx-x) + (ty-y)*(ty-y);
        buffer[i] = (buffer_entry){ np->p, d };
        i++;
    }

    qsort(buffer, i, sizeof(buffer_entry), sort_by_dist);

    return i;
}

void
interpolate_height(point *p, struct active_h *p_active, struct inactive_h *p_inactive)
{
    int count = find_nearest(p->x, p->y, p_active, p_inactive);
    int i, lim = (count > 10) ? 10 : count;
    double dsq[20], total_d = 0.0, scaled_height = 0.0;

    for (i=0; i<lim; i++) {
        double d = buffer[i].d;
        if (d > 0.0) { /* NAN ALERT */
            double dr2 = 1.0 / pow(d, 2);
            double scaled = dr2 * buffer[i].p.z;
            total_d += dr2;
            scaled_height += scaled;
            printf("I+ <%d,%d,%.2f> d=%.2f r=%.3f s=%.3f\n", buffer[i].p.x, buffer[i].p.y, buffer[i].p.z, d, dr2, scaled);
        }
        else {
            dsq[i] = 0.0;
        }
    }

    p->z = scaled_height / total_d;

    printf("IH <%d,%d> interpolated height %.2f\n",p->x,p->y,p->z);
}

int
random_under(int max)
{
    long r_val = random();
    return (int)(r_val % max);
}

struct pq_entry *
new_point(int x, int y, double z)
{
    struct pq_entry *f = malloc(sizeof(struct pq_entry));

    f->p.x = x; f->p.y = y; f->p.z = z;

    return f;
}

double
random_clamp(int min, int max)
{
    double n = random();
    double n_scaled = n / (RAND_MAX+1.0);
    double n_offset = n_scaled * (max-min);

    return n_offset + min;
}

struct pq_entry *
new_random_point(int width, int height, double elevation)
{
    int first_x = random_under(width), first_y = random_under(height);
    int first_z = random_clamp(0, elevation);

    return new_point(first_x, first_y, first_z);
}

void
generate_samples(int width, int height, struct active_h *p_active, struct inactive_h *p_inactive, int annulus, int interpolating)
{
    int howmany_active = 0;
    struct pq_entry *np;

    if (TAILQ_EMPTY(p_active)) {
        /* We start with a single random active point */
        struct pq_entry *f = new_random_point(width, height, (double)annulus);
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
        struct pq_entry *live = NULL;

        TAILQ_FOREACH(np, p_active, entries) {
            if (i == random_active) { live = np; break; }
            i++;
        }

        assert(live != NULL);

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
                    /* New points get the same height as the old one for now */
                    double new_height = live->p.z;
                    struct pq_entry *f = new_point(cx, cy, new_height);

                    printf("GP %i <%d,%d,%.2f>/a=%d/i=%d parent=<%d,%d> ACCEPTED\n", i, cx, cy, f->p.z, annulus, interpolating, ax, ay);

                    if (interpolating) {
                        interpolate_height(&(f->p), p_active, p_inactive);
                    }

                    f->p.z += random_clamp(-(annulus/2), annulus/2);

                    if (f->p.z < 0.0) { f->p.z = 0.0; }
                    if (f->p.z > 50.0) { f->p.z = 50.0; }

                    found_one = 1;
                    printf("GP %i <%d,%d,%.2f>/a=%d/i=%d parent=<%d,%d> ACCEPTED\n", i, cx, cy, f->p.z, annulus, interpolating, ax, ay);
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
