#include <stdio.h>
#include <stdlib.h>

#include "my-ppm.h"
#include "bridson.h"

int landscape[512][512];

int
main(void)
{
    int i,j;
    for(i=0; i<512; i++) {
        for(j=0; j<512; j++) {
            landscape[i][j] = 0;
        }
    }        
    generate_samples(512, 512, landscape, 20);
    output_pgm(stderr, 512, 512, landscape);
}