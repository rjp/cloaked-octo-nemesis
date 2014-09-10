/* Generate a Poisson Disc using Bridson */
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>

/* We need to keep track of active vs inactive points */
typedef struct { int x; int y; } point;
struct pq_entry { point p; TAILQ_ENTRY(pq_entry) entries; };

/* Utilise the magic BSD list macros to save sanity */
TAILQ_HEAD(point_queue, point) pq;

int
random_under(int max)
{
    long r_val = random();
    return (int)(r_val % max);
}

struct pq_entry *
new_active_point(int width, int height)
{
    int first_x = random_under(width), first_y = random_under(height);
    struct pq_entry *f = malloc(sizeof(struct pq_entry));

    f->p.x = first_x; f->p.y = first_y;

    return f;
}

void
generate_samples(int width, int height, int output[height][width], int annulus)
{
    int howmany_active = 0;
    struct pq_entry *np;

    TAILQ_HEAD(active_h, pq_entry) active = TAILQ_HEAD_INITIALIZER(active);
    TAILQ_HEAD(inactive_h, pq_entry) inactive = TAILQ_HEAD_INITIALIZER(inactive);

    TAILQ_INIT(&active);

    struct pq_entry *f = new_active_point(width, height);
    TAILQ_INSERT_HEAD(&active, f, entries);
    howmany_active = 1;

    while ( ! TAILQ_EMPTY(&active) ) {
        struct pq_entry *q = TAILQ_FIRST(&active);
        printf("Active is still going, %p <%d,%d>\n", q, q->p.x, q->p.y);
        TAILQ_REMOVE(&active, q, entries);
        TAILQ_INSERT_HEAD(&inactive, q, entries);
    }

    /* Convert the list of inactive points into dots on our canvas */
    TAILQ_FOREACH(np, &inactive, entries) {
        int x = np->p.x, y = np->p.y;
        output[y][x] = 255;
    }
}

