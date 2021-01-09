#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb-image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb-image/stb_image_write.h"

#define MAX_ENERGY 624.61988441
#define CHANNEL 3

typedef unsigned char pixel_t;

typedef struct pixel4
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} pixel4_t;
typedef struct pixel3
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} pixel3_t;

// TODO parse argument https://www.gnu.org/software/libc/manual/html_node/Argp.html
// void help();
// https://stackoverflow.com/questions/9642732/parsing-command-line-arguments-in-c
float *calc_energy3(pixel3_t *pixels, int w, int h, pixel3_t *energy_img);
float *calc_energy4(pixel4_t *pixels, int w, int h, pixel4_t *energy_img);
size_t idx(size_t row, size_t col, int w);

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("pass filename as argument!");
        return 1;
    }
    const char *filename = argv[1];
    int w, h, ch;
    unsigned char *pixels = stbi_load(filename, &w, &h, &ch, CHANNEL);

    if (pixels == NULL)
    {
        printf("image doesn't exist.\n");
        stbi_image_free(pixels);
        return 1;
    }

    // calculate energy
    unsigned char *energy_img = malloc(w * h * CHANNEL * sizeof(*energy_img));
    float *energies = calc_energy3((pixel3_t *)pixels, w, h, (pixel3_t *)energy_img);
    // save energy image
    stbi_write_png("output/energy.png", w, h, CHANNEL, energy_img, w * CHANNEL);

    // calculate seam

    printf("w: %d h: %d ch: %d\n", w, h, ch);

    stbi_image_free(pixels);
    return 0;
}
float *calc_energy3(pixel3_t *pixels, int w, int h, pixel3_t *energy_img)
{
    int rx, ry, gx, gy, bx, by;
    float dx, dy;

    float *energies = malloc(w * h * sizeof(float));

    for (size_t i = 0; i < h; i++)
    {
        for (size_t j = 0; j < w; j++)
        {
            rx = (i + 1 < h ? pixels[idx(i + 1, j, w)].r : 0) - (i - 1 >= 0 ? pixels[idx(i - 1, j, w)].r : 0);
            gx = (i + 1 < h ? pixels[idx(i + 1, j, w)].g : 0) - (i - 1 >= 0 ? pixels[idx(i - 1, j, w)].g : 0);
            bx = (i + 1 < h ? pixels[idx(i + 1, j, w)].b : 0) - (i - 1 >= 0 ? pixels[idx(i - 1, j, w)].b : 0);
            ry = (j + 1 < w ? pixels[idx(i, j + 1, w)].r : 0) - (j - 1 >= 0 ? pixels[idx(i, j - 1, w)].r : 0);
            gy = (j + 1 < w ? pixels[idx(i, j + 1, w)].g : 0) - (j - 1 >= 0 ? pixels[idx(i, j - 1, w)].g : 0);
            by = (j + 1 < w ? pixels[idx(i, j + 1, w)].b : 0) - (j - 1 >= 0 ? pixels[idx(i, j - 1, w)].b : 0);
            dx = (pow(rx, 2) + pow(gx, 2) + pow(bx, 2));
            dy = (pow(ry, 2) + pow(gy, 2) + pow(by, 2));
            energies[idx(i, j, w)] = sqrt(dx + dy);
            energy_img[idx(i, j, w)].r = (energies[idx(i, j, w)] / MAX_ENERGY) * 255;
            energy_img[idx(i, j, w)].g = (energies[idx(i, j, w)] / MAX_ENERGY) * 255;
            energy_img[idx(i, j, w)].b = (energies[idx(i, j, w)] / MAX_ENERGY) * 255;
            // printf("%.3f\n", (energies[idx(i, j, w)] / MAX_ENERGY));
        }
    }

    return energies;
}

size_t idx(size_t row, size_t col, int w)
{
    return row * w + col;
}