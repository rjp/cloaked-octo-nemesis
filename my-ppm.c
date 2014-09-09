#include "my-ppm.h"

void
output_pgm(FILE *stream, int width, int height, int pgm[height][width])
{
    int i, j;

    fprintf(stream, "P2 %d %d 255\n", width, height);
    for(i=0; i<height; i++) {
        for(j=0; j<width; j++) {
            fprintf(stream, "%d ", pgm[i][j]);
            if (j % 15 == 14) { fprintf(stream, "\n"); }
        }
        fprintf(stream, "\n");
    }
}
