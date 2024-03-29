#ifndef __LANDSCAPE_H
#define __LANDSCAPE_H 1

#include <sys/queue.h>
#include "colour.h"

/* We need to keep track of active vs inactive points */
typedef struct { int x; int y; double z; rgb c; } point;
struct pq_entry { point p; TAILQ_ENTRY(pq_entry) entries; };
struct pq_entry *np, *np_temp;

/* Utilise the magic BSD list macros to save sanity */
TAILQ_HEAD(point_queue, point) pq;

TAILQ_HEAD(active_h, pq_entry);
TAILQ_HEAD(inactive_h, pq_entry);

extern struct active_h active;
extern struct inactive_h inactive;

extern double m1_p1(void);
#endif
