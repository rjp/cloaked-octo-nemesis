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

double
randn (double mu, double sigma)
{
  double U1, U2, W, mult;
  static double X1, X2;
  static int call = 0;

  if (call == 1)
    {
      call = !call;      return (mu + sigma * (double) X2);
    }

  do
    {
      U1 = -1 + ((double) rand () / RAND_MAX) * 2;
      U2 = -1 + ((double) rand () / RAND_MAX) * 2;
      W = pow (U1, 2) + pow (U2, 2);    }
  while (W >= 1 || W == 0);

  mult = sqrt ((-2 * log (W)) / W);
  X1 = U1 * mult;
  X2 = U2 * mult;

  call = !call;

  return (mu + sigma * (double) X1);
}

double
m1_p1(void)
{
    double n = randn(0.0, 1.0);
    return n;
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
            debug("I+ <%d,%d,%.2f> d=%.2f r=%.3f s=%.3f\n", buffer[i].p.x, buffer[i].p.y, buffer[i].p.z, d, dr2, scaled);
        }
        else {
            dsq[i] = 0.0;
        }
    }

    p->z = scaled_height / total_d;

    debug("IH <%d,%d> interpolated height %.2f\n",p->x,p->y,p->z);
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
random_clamp(double min, double max)
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
    double first_z = 0.3 + 0.3*m1_p1();
if(first_z < 0.0) { first_z = 0.0; }
if(first_z > 1.0) { first_z = 1.0; }

    return new_point(first_x, first_y, first_z);
}

void
generate_samples(int width, int height, struct active_h *p_active, struct inactive_h *p_inactive, double annulus, int interpolating, int generation)
{
    int howmany_active = 0;
    struct pq_entry *np;
    double reduction = pow(0.5, generation);

    if (TAILQ_EMPTY(p_active)) {
        /* We start with a single random active point */
        struct pq_entry *f = new_random_point(width, height, 92);
        debug("++ <%d,%d,%.3f> an=%d\n", f->p.x, f->p.y, f->p.z, annulus);
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
                    double new_height = live->p.z + reduction*0.025*m1_p1(); // live->p.z;
                    struct pq_entry *f = new_point(cx, cy, new_height);

                    printf("GP %i <%d,%d,%.2f>/a=%.2f/i=%d parent=<%d,%d,%.2f> ACCEPTED\n", i, cx, cy, f->p.z, annulus, interpolating, ax, ay, live->p.z);

                    if (interpolating) {
                        interpolate_height(&(f->p), p_active, p_inactive);
                    }

                    double offset = reduction * 0.05 * m1_p1();
                    printf("GP %.3f\n", offset);

                    f->p.z += offset;

                    if (f->p.z < 0.0) { f->p.z = 0.0; debug("CLAMPED\n");}
                    if (f->p.z > 1.0) { f->p.z = 1.0; }

                    found_one = 1;
                    printf("GP %i <%d,%d,%.2f>/a=%.2f/i=%d parent=<%d,%d> FINAL\n", i, cx, cy, f->p.z, annulus, interpolating, ax, ay);
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
