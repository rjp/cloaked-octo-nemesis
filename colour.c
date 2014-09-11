#include "landscape.h"
#include "colour.h"

rgb c_sand = { 255, 192, 64 };
rgb c_water = { 0, 0, 255 };
rgb c_grass = { 0, 255, 0 };
rgb c_stone = { 128, 128, 128 };
rgb c_snow = { 255, 255, 255 };

rgb
colour_by_height(double scaled_height)
{
    if (scaled_height < 0.20) {
        return c_water;
    }

    if (scaled_height < 0.25) {
        return c_sand;
    }

    if (scaled_height > 0.80) {
        return c_snow;
    }

    if (scaled_height > 0.70) {
        return c_stone;
    }

    return c_grass;
}

