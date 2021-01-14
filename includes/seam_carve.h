#pragma once
#include <stddef.h>

#define MAX_ENERGY 624.61988441
#define CHANNEL 3

typedef unsigned char pixel_t;
typedef struct pixel3
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} pixel3_t;

typedef struct fext
{
    float val;
    int from;
} fext_t;

float *calc_energy3(pixel3_t *pixels, int w, int h, pixel3_t **energy_img);
size_t idx(size_t row, size_t col, int w);
int *find_vseam(int w, int h, float *e);
int *find_hseam(int w, int h, float *e);
pixel3_t *remove_seam(pixel3_t *img, int *vseam, int *hseam, int w, int h);
void draw_vseam(pixel3_t *img, int *vseam, int w, int h, ge_GIF *gif);
void draw_hseam(pixel3_t *img, int *hseam, int w, int h, ge_GIF *gif);
pixel3_t *remove_vseam(pixel3_t *img, int *vseam, int w, int h);
pixel3_t *remove_hseam(pixel3_t *img, int *hseam, int w, int h);