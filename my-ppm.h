#ifndef __MY_PPM_H
#define __MY_PPM_H

#include <stdio.h>
#include <stdlib.h>

typedef struct { int r; int g; int b; } rgb;

void output_pgm(FILE *stream, int width, int height, int pgm[height][width]);
void output_ppm(FILE *stream, int width, int height, rgb ppm[height][width]);
#endif
