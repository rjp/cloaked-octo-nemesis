#include "landscape.h"
#include "colour.h"

rgb c_ocean = { 0, 0, 128 };
rgb c_sand = { 255, 192, 64 };
rgb c_water = { 0, 0, 255 };
rgb c_grass = { 0, 255, 0 };
rgb c_stone = { 128, 128, 128 };
rgb c_snow = { 255, 255, 255 };
rgb c_dirt = { 128, 64, 0 };

int
clamp_byte(int v)
{
    if (v > 255) { return 255; }
    if (v < 0)   { return 0; }
    return v;
}

rgb
twiddle(rgb in, int grey, int scale)
{
    rgb out = in;
    int dr, dg, db;

    dr = (int)(scale * m1_p1());
    if (grey) {
        dg = dr; db = dr;
    }
    else {
        dg = (int)(scale * m1_p1());
        db = (int)(scale * m1_p1());
    }

    out = (rgb){
        clamp_byte(in.r+dr),
        clamp_byte(in.g+dg),
        clamp_byte(in.b+db)
    };

    return out;
}

rgb
colour_by_height(double scaled_height)
{
    if (scaled_height < 0.10) {
        return twiddle(c_ocean, 0, 8);
    }

    if (scaled_height < 0.20) {
        return twiddle(c_water, 0, 16);
    }

    if (scaled_height < 0.25) {
        return twiddle(c_sand, 0, 8);
    }

    if (scaled_height > 0.80) {
        return twiddle(c_snow, 1, 32);
    }

    if (scaled_height > 0.70) {
        return twiddle(c_stone, 1, 32);
    }

    if (scaled_height> 0.60) {
        return twiddle(c_dirt, 1, 16);
    }

    return twiddle(c_grass, 0, 24);
}

