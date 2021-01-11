#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <float.h>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb-image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb-image/stb_image_write.h"

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

// TODO parse argument https://www.gnu.org/software/libc/manual/html_node/Argp.html
// void help();
// https://stackoverflow.com/questions/9642732/parsing-command-line-arguments-in-c
float *
calc_energy3(pixel3_t *pixels, int w, int h, pixel3_t *energy_img);
void resize_image(unsigned char *pixels, int w, int h, int target_width, int target_height, const char *outname);
size_t idx(size_t row, size_t col, int w);

int main(int argc, char const *argv[])
{
  if (argc != 5)
  {
    printf("pass filename and target size!");
    return 1;
  }
  const char *filename = argv[1];
  int target_width = atoi(argv[2]);
  int target_height = atoi(argv[3]);
  const char *outname = argv[4];

  if (target_height <= 0 || target_width <= 0)
  {
    printf("invalid pixels.\n");
    return 1;
  }

  int w, h, ch;
  unsigned char *pixels = stbi_load(filename, &w, &h, &ch, CHANNEL);

  if (pixels == NULL)
  {
    printf("image doesn't exist.\n");
    stbi_image_free(pixels);
    return 1;
  }

  pixel3_t *temp = pixels;
  for (size_t i = 0; i < h; i++)
  {
    for (size_t j = 0; j < w; j++)
    {
      printf("(%d,%d,%d)\t", temp[idx(i, j, w)].r, temp[idx(i, j, w)].g, temp[idx(i, j, w)].b);
    }
    printf("\n");
  }
  printf("\n");
  printf("\n");
  struct timespec s, e;
  clock_gettime(CLOCK_REALTIME, &s);
  resize_image(pixels, w, h, target_width, target_height, outname);
  clock_gettime(CLOCK_REALTIME, &e);
  printf("%lf\n", e.tv_sec - s.tv_sec + (e.tv_nsec - s.tv_nsec) / pow(10, 9));

  // calculate seam
  printf("w: %d h: %d ch: %d\n", w, h, ch);

  stbi_image_free(pixels);
  return 0;
}

float calc_min(float a, float b, float c)
{
  return a < b ? (a < c ? a : c) : (b < c ? b : c);
}

void resize_image(unsigned char *pixels, int w, int h, int target_width, int target_height, const char *outname)
{
  // calculate energy
  pixel3_t *energy_img = malloc(w * h * sizeof(*energy_img));
  float *e = calc_energy3((pixel3_t *)pixels, w, h, energy_img);
  int i, j, k, l;
  fext_t **m = malloc(h * sizeof(*m));

  for (i = 0; i < h; i++)
  {
    m[i] = malloc(w * sizeof(*m));
  }

  float min = FLT_MAX;
  for (i = 0; i < w; i++)
  {
    m[0][i].val = e[idx(0, i, w)];
  }

  for (i = 1; i < h; i++)
  {
    for (j = 0; j < w; j++)
    {
      m[i][j].val = e[idx(i, j, w)] + calc_min(m[i - 1][j].val,
                                               j - 1 >= 0 ? m[i - 1][j - 1].val : FLT_MAX,
                                               j + 1 < w ? m[i - 1][j + 1].val : FLT_MAX);
      float a = m[i - 1][j].val;
      float b = j - 1 >= 0 ? m[i - 1][j - 1].val : FLT_MAX;
      float c = j + 1 < w ? m[i - 1][j + 1].val : FLT_MAX;
      m[i][j].from = a < b ? (a < c ? j : j + 1) : (b < c ? j - 1 : j + 1);
    }
  }

  // find the index of minimum energy at the bottom row

  for (k = 0, i = 0; i < w; i++)
  {
    if (min > m[h - 1][i].val)
    {
      min = m[h - 1][i].val;
      k = i;
    }
  }

  pixel3_t *resized_img = malloc((w - 1) * h * sizeof(*resized_img));
  pixel3_t *actual_img = (pixel3_t *)pixels;
  // backtrace to the top
  for (i = h - 1; i >= 0; --i)
  {
    for (l = 0, j = 0; j < w; j++, l++)
    {
      if (j == k)
        j++;
      resized_img[idx(i, l, w - 1)].r = actual_img[idx(i, j, w)].r;
      resized_img[idx(i, l, w - 1)].g = actual_img[idx(i, j, w)].g;
      resized_img[idx(i, l, w - 1)].b = actual_img[idx(i, j, w)].b;
    }
    energy_img[idx(i, k, w)].r = 255;
    energy_img[idx(i, k, w)].g = 0;
    energy_img[idx(i, k, w)].b = 0;
    k = m[i][k].from;
  }

  stbi_write_png(outname, w - 1, h, CHANNEL, resized_img, (w - 1) * CHANNEL);
  stbi_write_png("output/energy.png", w, h, CHANNEL, energy_img, w * CHANNEL);
  stbi_image_free(energy_img);
  stbi_image_free(resized_img);
  for (i = 0; i < h; i++)
  {
    free(m[i]);
  }
  free(m);
}

float *calc_energy3(pixel3_t *pixels, int w, int h, pixel3_t *energy_img)
{
  int rx, ry, gx, gy, bx, by;
  float dx, dy;

  float *energies = malloc(w * h * sizeof(float));

  for (int i = 0; i < h; i++)
  {
    for (int j = 0; j < w; j++)
    {
      ry = (i + 1 < h ? pixels[idx(i + 1, j, w)].r : 0) - (i - 1 >= 0 ? pixels[idx(i - 1, j, w)].r : 0);
      gy = (i + 1 < h ? pixels[idx(i + 1, j, w)].g : 0) - (i - 1 >= 0 ? pixels[idx(i - 1, j, w)].g : 0);
      by = (i + 1 < h ? pixels[idx(i + 1, j, w)].b : 0) - (i - 1 >= 0 ? pixels[idx(i - 1, j, w)].b : 0);
      rx = (j + 1 < w ? pixels[idx(i, j + 1, w)].r : 0) - (j - 1 >= 0 ? pixels[idx(i, j - 1, w)].r : 0);
      gx = (j + 1 < w ? pixels[idx(i, j + 1, w)].g : 0) - (j - 1 >= 0 ? pixels[idx(i, j - 1, w)].g : 0);
      bx = (j + 1 < w ? pixels[idx(i, j + 1, w)].b : 0) - (j - 1 >= 0 ? pixels[idx(i, j - 1, w)].b : 0);

      dx = (pow(rx, 2) + pow(gx, 2) + pow(bx, 2));
      dy = (pow(ry, 2) + pow(gy, 2) + pow(by, 2));
      energies[idx(i, j, w)] = sqrt(dx + dy);

      if (energy_img == NULL)
        continue;

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
