#ifndef __BRIDSON_H
#define __BRIDSON_H 1
#include <sys/queue.h>

#include "landscape.h"

void generate_samples(int width, int height, struct active_h *active, struct inactive_h *inactive, int annulus);
#endif
