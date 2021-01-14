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
#include "lib/gifenc/gifenc.h"

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
float *calc_energy3(pixel3_t *pixels, int w, int h, pixel3_t **energy_img);
// pixel3_t *resize_image(pixel3_t *img, fext_t **vm, int vsi, int w, int h);
size_t idx(size_t row, size_t col, int w);
int *find_vseam(int w, int h, float *e);
int *find_hseam(int w, int h, float *e);
pixel3_t *remove_seam(pixel3_t *img, int *vseam, int *hseam, int w, int h);
// void update_energy_img(pixel3_t *img, fext_t **vm, int w, int h, int vsi, ge_GIF *gif);
void draw_seam(pixel3_t *img, int *vseam, int w, int h);

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
  pixel3_t *img = (pixel3_t *)stbi_load(filename, &w, &h, &ch, CHANNEL);

  if (img == NULL)
  {
    printf("image doesn't exist.\n");
    stbi_image_free(img);
    return 1;
  }

  pixel3_t *resized_img, *energy_img = NULL;
  float *e;           // energy table
  int *vseam, *hseam; // vertical and horizontal seams

  struct timespec s, end;
  clock_gettime(CLOCK_REALTIME, &s);

  // red is 255
  // pixel3_t *pallet = malloc(256 * sizeof(*pallet));
  // for (int i = 0; i < 255; i++)
  // {
  //   pallet[i].r = i;
  //   pallet[i].g = i;
  //   pallet[i].b = i;
  // }
  // pallet[255].r = 255;
  // pallet[255].g = 0;
  // pallet[255].b = 0;
  // for (int i = 0; i < 257; i++)
  // {
  //   printf("(%d,%d,%d)\n", pallet[i].r,
  //          pallet[i].g,
  //          pallet[i].b);
  // }
  // ge_GIF *gif = ge_new_gif(
  //     "output/example.gif", /* file name */
  //     w, h,                 /* canvas size */
  //     (uint8_t *)pallet,
  //     8, /* palette depth == log2(# of colors) */
  //     0  /* infinite loop */
  // );

  int i = 0, j = 0;
  while (j < w - target_width || i < h - target_height)
  {
    e = calc_energy3(img, w - j, h - i, &energy_img);
    vseam = j < w - target_width ? find_vseam(w - j, h, e) : NULL;
    hseam = i < h - target_height ? find_hseam(w, h - i, e) : NULL;
    if (vseam != NULL)
      draw_seam(energy_img, vseam, w - j, h - i);

    // for (int k = 0; k < h; k++)
    // {
    //   printf("%d\t", hseam[k]);
    // }
    // printf("\n");

    // resized_img = remove_seam(img, vseam, hseam, w - (j + 1), h - (i + 1));

    // free(img);
    // img = resized_img;
    printf("%p\n", energy_img);
    stbi_write_png("output/energy.png", w - j, h - i, CHANNEL, energy_img, (w - j) * CHANNEL);
    // stbi_write_png(outname, target_width, target_height, CHANNEL, img, target_width * CHANNEL);
    // free(energy_img);
    j += 1 * (j < w - target_width);
    i += 1 * (i < h - target_height);
  }

  // stbi_write_png(outname, target_width, target_height, CHANNEL, resized_img, target_width * CHANNEL);
  // stbi_write_png("output/energy.png", w, h, CHANNEL, energy_img, w * CHANNEL);
  // printf("%lf\n", end.tv_sec - s.tv_sec + (end.tv_nsec - s.tv_nsec) / pow(10, 9));

  // for (int i = 0; i < w - target_width; ++i)
  // {
  //   e = calc_energy3(actual_img, w - i, h, i == 0 ? energy_img : NULL);
  //   vm = find_vseam(w - i, h, e, &vsi);
  //   hm = find_hseam(w, h - i, e, &vsi);
  //   update_energy_img(energy_img, vm, w, h, vsi, gif);
  //   ge_add_frame(gif, 10);
  //   resized_img = resize_image(actual_img, vm, vsi, w - (i + 1), h);

  //   // free resources and repeat
  //   if (i != 0)
  //     stbi_image_free(actual_img);
  //   actual_img = resized_img;
  // }
  clock_gettime(CLOCK_REALTIME, &end);

  // stbi_write_png(outname, target_width, h, CHANNEL, resized_img, target_width * CHANNEL);
  // stbi_write_png("output/energy.png", w, h, CHANNEL, energy_img, w * CHANNEL);
  // printf("%lf\n", end.tv_sec - s.tv_sec + (end.tv_nsec - s.tv_nsec) / pow(10, 9));

  // // free resources at the end
  // ge_close_gif(gif);
  // stbi_image_free(pixels);
  // stbi_image_free(energy_img);
  // stbi_image_free(resized_img);
  return 0;
}

float calc_min(float a, float b, float c, int j, int *index)
{
  *index = a < b ? (a < c ? j : j + 1) : (b < c ? j - 1 : j + 1);
  return a < b ? (a < c ? a : c) : (b < c ? b : c);
}

int *find_vseam(int w, int h, float *e)
{
  int i, j, k;
  float min = FLT_MAX, a, b, c;
  fext_t **m = malloc(h * sizeof(*m));

  for (i = 0; i < h; ++i)
  {
    m[i] = malloc(w * sizeof(**m));
  }

  // initialize the first row to energies
  for (i = 0; i < w; ++i)
  {
    m[0][i].val = e[idx(0, i, w)];
  }

  // iterate over energies and calculate minimum
  for (i = 1; i < h; ++i)
  {
    for (j = 0; j < w; ++j)
    {
      a = m[i - 1][j].val;
      b = j - 1 >= 0 ? m[i - 1][j - 1].val : FLT_MAX;
      c = j + 1 < w ? m[i - 1][j + 1].val : FLT_MAX;
      m[i][j].val = e[idx(i, j, w)] + calc_min(a, b, c, j, &m[i][j].from);
    }
  }

  // find the minimum number at the end row
  for (k = 0, i = 0; i < w; ++i)
  {
    if (min > m[h - 1][i].val)
    {
      min = m[h - 1][i].val;
      k = i;
    }
  }
  // bactrace to top and create the seam
  int *seam = malloc(h * sizeof(*seam));

  for (i = h - 1; i >= 0; --i)
  {
    seam[i] = k;
    k = m[i][k].from;
  }
  // free resources
  for (int j = 0; j < h; ++j)
  {
    free(m[j]);
  }
  free(m);

  return seam;
}

int *find_hseam(int w, int h, float *e)
{
  int i, j, k;
  float min = FLT_MAX;
  fext_t **m = malloc(h * sizeof(*m));

  for (i = 0; i < h; ++i)
  {
    m[i] = malloc(w * sizeof(**m));
  }

  // initialize the first column to energies
  for (i = 0; i < h; ++i)
  {
    m[i][0].val = e[idx(i, 0, w)];
  }

  float a, b, c;
  // iterate over energies and calculate minimum
  for (j = 1; j < w; ++j)
  {
    for (i = 0; i < h; ++i)
    {
      a = m[i][j - 1].val;
      b = i - 1 >= 0 ? m[i - 1][j - 1].val : FLT_MAX;
      c = i + 1 < h ? m[i + 1][j - 1].val : FLT_MAX;
      m[i][j].val = e[idx(i, j, w)] + calc_min(a, b, c, i, &m[i][j].from);
    }
  }

  // find the minimum number at the end column
  for (k = 0, i = 0; i < h; ++i)
  {
    if (min > m[i][w - 1].val)
    {
      min = m[i][w - 1].val;
      k = i;
    }
  }
  // bactrace to top and create the seam
  int *seam = malloc(w * sizeof(*seam));
  for (i = w - 1; i >= 0; --i)
  {
    seam[i] = k;
    k = m[k][i].from;
  }

  // free resources
  for (int j = 0; j < h; ++j)
  {
    free(m[j]);
  }
  free(m);

  return seam;
}

// draw only the vertical seam
void draw_seam(pixel3_t *img, int *vseam, int w, int h)
{
  for (int i = 0; i < h; i++)
  {
    img[idx(i, vseam[i], w)].r = 255;
    img[idx(i, vseam[i], w)].g = 0;
    img[idx(i, vseam[i], w)].b = 0;
  }

  // stbi_write_png("output/energy.png", w, h, CHANNEL, img, w * CHANNEL);
}

pixel3_t *remove_seam(pixel3_t *img, int *vseam, int *hseam, int w, int h)
{
  pixel3_t *resized_img = malloc(w * h * sizeof(*resized_img));

  for (int i = 0; i < h; ++i)
  {
    for (int j = 0, l = 0; j < w; ++j, ++l)
    {
      if (j == vseam[i])
        j++;
      resized_img[idx(i, l, w)].r = img[idx(i, j, w + 1)].r;
      resized_img[idx(i, l, w)].g = img[idx(i, j, w + 1)].g;
      resized_img[idx(i, l, w)].b = img[idx(i, j, w + 1)].b;
    }
  }
  for (int j = 0; j < w; ++j)
  {
    for (int i = 0, l = 0; i < h; ++i, ++l)
    {
      if (i == hseam[j])
        i++;
      resized_img[idx(i, l, w)].r = img[idx(i, j, w + 1)].r;
      resized_img[idx(i, l, w)].g = img[idx(i, j, w + 1)].g;
      resized_img[idx(i, l, w)].b = img[idx(i, j, w + 1)].b;
    }
  }

  return resized_img;
}

float *calc_energy3(pixel3_t *pixels, int w, int h, pixel3_t **energy_img)
{
  int rx, ry, gx, gy, bx, by;
  float dx, dy;
  *energy_img = malloc(w * h * sizeof(*energy_img));
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

      (*energy_img)[idx(i, j, w)].r = (energies[idx(i, j, w)] / MAX_ENERGY) * 255;
      (*energy_img)[idx(i, j, w)].g = (energies[idx(i, j, w)] / MAX_ENERGY) * 255;
      (*energy_img)[idx(i, j, w)].b = (energies[idx(i, j, w)] / MAX_ENERGY) * 255;
      // printf("%.3f\n", (energies[idx(i, j, w)] / MAX_ENERGY));
    }
  }

  return energies;
}

size_t idx(size_t row, size_t col, int w)
{
  return row * w + col;
}
