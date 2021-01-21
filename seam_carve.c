#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "includes/seam_carve.h"
#include "lib/gifenc/gifenc.h"
#include <stdio.h>
#include <stdbool.h>

// helper functions

// converts a 2d index to 1d equivalant
// size_t idx(size_t row, size_t col, int w);
// get the next vertical and horizontal seam
size_t idx(size_t row, size_t col, int w);
float calc_max(float a, float b, float c);
float calc_min_index(float a, float b, float c, int j);

// calculates the min btween a,b and c and set the index based on
// following rules:
// if a is min j is the index
// if b is min j - 1 is the index
// if c is min j + 1 is the index
float calc_min(float a, float b, float c, int j, int *index);
float color_diff(pixel3_t p0, pixel3_t p1);

void draw_vseam(seam_carve_t *sc)
{
  ge_GIF *gif = sc->gif;
  int w = w = sc->w - sc->current_w, h = sc->h - sc->current_h;
  pixel3_t *img = sc->energy_map_image;
  int *vseam = sc->vseam;
  if (gif != NULL)
  {
    gif->w = w;
    gif->h = h;

    for (int i = 0; i < h; i++)
    {
      for (int j = 0; j < w; j++)
      {
        gif->frame[idx(i, j, w)] = img[idx(i, j, w)].r;
      }
    }
  }

  for (int i = 0; i < h; i++)
  {
    if (gif != NULL)
      gif->frame[idx(i, vseam[i], w)] = 255;
    img[idx(i, vseam[i], w)].r = 255;
    img[idx(i, vseam[i], w)].g = 0;
    img[idx(i, vseam[i], w)].b = 0;
  }
}

void draw_hseam(seam_carve_t *sc)
{
  ge_GIF *gif = sc->gif;
  int w = sc->w - sc->current_w, h = sc->h - sc->current_h;
  pixel3_t *img = sc->energy_map_image;
  int *hseam = sc->hseam;
  if (gif != NULL)
  {
    gif->w = w;
    gif->h = h;
    for (int i = 0; i < h; i++)
    {
      for (int j = 0; j < w; j++)
      {
        gif->frame[idx(i, j, w)] = img[idx(i, j, w)].r;
      }
    }
  }

  for (int j = 0; j < w; j++)
  {
    if (gif != NULL)
      gif->frame[idx(hseam[j], j, w)] = 255;
    img[idx(hseam[j], j, w)].r = 255;
    img[idx(hseam[j], j, w)].g = 0;
    img[idx(hseam[j], j, w)].b = 0;
  }
}
//pixel3_t **img, int *vseam, int w, int h
void remove_vseam(seam_carve_t *sc)
{
  if (sc->vseam == NULL || sc->img == NULL)
  {
    printf("vseam or img can't be null\n");
    return;
  }

  pixel3_t *img = sc->img;
  int w = sc->w - (sc->current_w + 1), h = sc->h - sc->current_h;
  for (int i = 0; i < h; ++i)
  {
    for (int j = 0, l = 0; j < w + 1; ++j, ++l)
    {
      if (j == sc->vseam[i])
        j++;
      img[idx(i, l, w)].r = img[idx(i, j, w + 1)].r;
      img[idx(i, l, w)].g = img[idx(i, j, w + 1)].g;
      img[idx(i, l, w)].b = img[idx(i, j, w + 1)].b;
    }
  }

  // sc->img = realloc(sc->img, w * h * sizeof(pixel3_t));
}

void remove_hseam(seam_carve_t *sc)
{
  if (sc->hseam == NULL || sc->img == NULL)
  {
    printf("hseam or img can't be null\n");
    return;
  }
  pixel3_t *img = sc->img;
  int w = sc->w - (sc->current_w + has_vseam(sc)), h = sc->h - (sc->current_h + 1);
  for (int j = 0; j < w; ++j)
  {
    for (int i = 0, l = 0; i < h + 1; ++i, ++l)
    {
      if (i == sc->hseam[j])
        i++;
      // if (sc->current_w >= 47)
      // printf("j: %d i: %d\n", j, i);
      img[idx(l, j, w)].r = img[idx(i, j, w)].r;
      img[idx(l, j, w)].g = img[idx(i, j, w)].g;
      img[idx(l, j, w)].b = img[idx(i, j, w)].b;
    }
  }
  // sc->img = realloc(sc->img, w * h * sizeof(pixel3_t));
}

void calc_energy_backward(seam_carve_t *sc)
{
  int rx, ry, gx, gy, bx, by;
  int w = w = sc->w - sc->current_w, h = sc->h - sc->current_h;
  float dx, dy;
  // sc->energy_map_image = realloc(sc->energy_map_image, w * h * sizeof(*sc->energy_map_image));
  // sc->energy_map_backward = realloc(sc->energy_map_backward, w * h * sizeof(sc->energy_map_backward));
  pixel3_t *img = sc->img;

  for (int i = 0; i < h; i++)
  {
    for (int j = 0; j < w; j++)
    {
      ry = (i + 1 < h ? img[idx(i + 1, j, w)].r : 0) - (i - 1 >= 0 ? img[idx(i - 1, j, w)].r : 0);
      gy = (i + 1 < h ? img[idx(i + 1, j, w)].g : 0) - (i - 1 >= 0 ? img[idx(i - 1, j, w)].g : 0);
      by = (i + 1 < h ? img[idx(i + 1, j, w)].b : 0) - (i - 1 >= 0 ? img[idx(i - 1, j, w)].b : 0);
      rx = (j + 1 < w ? img[idx(i, j + 1, w)].r : 0) - (j - 1 >= 0 ? img[idx(i, j - 1, w)].r : 0);
      gx = (j + 1 < w ? img[idx(i, j + 1, w)].g : 0) - (j - 1 >= 0 ? img[idx(i, j - 1, w)].g : 0);
      bx = (j + 1 < w ? img[idx(i, j + 1, w)].b : 0) - (j - 1 >= 0 ? img[idx(i, j - 1, w)].b : 0);

      dx = (pow(rx, 2) + pow(gx, 2) + pow(bx, 2));
      dy = (pow(ry, 2) + pow(gy, 2) + pow(by, 2));
      sc->energy_map_backward[idx(i, j, w)] = sqrt(dx + dy);

      sc->energy_map_image[idx(i, j, w)].r = (sc->energy_map_backward[idx(i, j, w)] / MAX_ENERGY_BACKWARD) * 255;
      sc->energy_map_image[idx(i, j, w)].g = (sc->energy_map_backward[idx(i, j, w)] / MAX_ENERGY_BACKWARD) * 255;
      sc->energy_map_image[idx(i, j, w)].b = (sc->energy_map_backward[idx(i, j, w)] / MAX_ENERGY_BACKWARD) * 255;
    }
  }
}

void calc_energy_forward(seam_carve_t *sc)
{

  int w = sc->w - sc->current_w, h = sc->h - sc->current_h;
  pixel3_t black = {0, 0, 0};
  pixel3_t *img = sc->img;
  // sc->energy_map_image = realloc(sc->energy_map_image, w * h * sizeof(*sc->energy_map_image));
  // sc->energy_map_forward = realloc(sc->energy_map_forward, w * h * sizeof(cost_t));
  cost_t *energies = sc->energy_map_forward;
  int index[3];

  if (!has_vseam(sc))
    goto skip_vertical;
  // cost of first row
  for (int j = 0; j < w; j++)
  {
    index[0] = idx(0, j, w);
    energies[index[0]].vl = 0;
    energies[index[0]].vr = 0;
    energies[index[0]].vu =
        color_diff(j - 1 >= 0 ? img[idx(0, j - 1, w)] : black,
                   j + 1 < w ? img[idx(0, j + 1, w)] : black);
  }

  // cost of left and right col
  for (int i = 1; i < h; i++)
  {
    // left col
    energies[idx(i, 0, w)].vl =
        color_diff(img[idx(i, 0, w)], img[idx(i, 1, w)]) +
        color_diff(img[idx(i - 1, 0, w)], img[idx(i, 1, w)]);
    energies[idx(i, 0, w)].vr =
        color_diff(img[idx(i, 0, w)], img[idx(i, 1, w)]) +
        color_diff(img[idx(i - 1, 0, w)], img[idx(i, 1, w)]);
    energies[idx(i, 0, w)].vu =
        color_diff(img[idx(i, 0, w)], img[idx(i, 1, w)]);
    // right col
    energies[idx(i, w - 1, w)].vl =
        color_diff(img[idx(i, w - 2, w)], img[idx(i, w - 1, w)]) +
        color_diff(img[idx(i - 1, w - 2, w)], img[idx(i, w - 1, w)]);
    energies[idx(i, w - 1, w)].vr =
        color_diff(img[idx(i, w - 2, w)], img[idx(i, w - 1, w)]) +
        color_diff(img[idx(i - 1, w - 2, w)], img[idx(i, w - 1, w)]);
    energies[idx(i, w - 1, w)].vu =
        color_diff(img[idx(i, w - 2, w)], img[idx(i, w - 1, w)]);
  }

  for (int i = 1; i < h; i++)
  {
    for (int j = 1; j < w - 1; j++)
    {
      energies[idx(i, j, w)].vl =
          color_diff(img[idx(i, j - 1, w)], img[idx(i, j + 1, w)]) +
          color_diff(img[idx(i - 1, j, w)], img[idx(i, j - 1, w)]);
      energies[idx(i, j, w)].vr =
          color_diff(img[idx(i, j - 1, w)], img[idx(i, j + 1, w)]) +
          color_diff(img[idx(i - 1, j, w)], img[idx(i, j + 1, w)]);
      energies[idx(i, j, w)].vu = color_diff(img[idx(i, j - 1, w)], img[idx(i, j + 1, w)]);
      int temp = calc_max(energies[idx(i, j, w)].vl, energies[idx(i, j, w)].vr, energies[idx(i, j, w)].vu);
      sc->max_energy_map = sc->max_energy_map > temp ? sc->max_energy_map : temp;
    }
  }
skip_vertical:
  if (!has_hseam(sc))
    goto skip_horizontal;

  // cost of the horizontal left col
  for (int i = 0; i < h; i++)
  {
    energies[idx(i, 0, w)].hu = 0;
    energies[idx(i, 0, w)].hd = 0;
    energies[idx(i, 0, w)].hl =
        color_diff(img[idx(i - 1 >= 0 ? i - 1 : i, 0, w)],
                   img[idx(i + 1 < h ? i + 1 : i, 0, w)]);
  }
  // cost of horizontal bottom and top row
  for (int j = 1; j < w; j++)
  {
    // top row
    energies[idx(0, j, w)].hd =
        color_diff(img[idx(0, j - 1, w)], img[idx(1, j, w)]) +
        color_diff(img[idx(1, j, w)], img[idx(0, j, w)]);
    energies[idx(0, j, w)].hu =
        color_diff(img[idx(0, j - 1, w)], img[idx(0, j, w)]) +
        color_diff(img[idx(0, j, w)], img[idx(1, j, w)]);
    energies[idx(0, j, w)].hl =
        color_diff(img[idx(0, j, w)], img[idx(1, j, w)]);
    // bottom row
    energies[idx(h - 1, j, w)].hd =
        color_diff(img[idx(h - 2, j - 1, w)], img[idx(h - 1, j, w)]) +
        color_diff(img[idx(h - 1, j, w)], img[idx(h - 2, j, w)]);
    energies[idx(h - 1, j, w)].hu =
        color_diff(img[idx(h - 2, j - 1, w)], img[idx(h - 2, j, w)]) +
        color_diff(img[idx(h - 2, j, w)], img[idx(h - 1, j, w)]);
    energies[idx(h - 1, j, w)].hl =
        color_diff(img[idx(h - 2, j, w)], img[idx(h - 1, j, w)]);
  }

  for (int j = 1; j < w; j++)
  {
    for (int i = 1; i < h - 1; i++)
    {
      energies[idx(i, j, w)].hd =
          color_diff(img[idx(i, j - 1, w)], img[idx(i + 1, j, w)]) +
          color_diff(img[idx(i - 1, j, w)], img[idx(i + 1, j, w)]);
      energies[idx(i, j, w)].hu =
          color_diff(img[idx(i, j - 1, w)], img[idx(i - 1, j, w)]) +
          color_diff(img[idx(i - 1, j, w)], img[idx(i + 1, j, w)]);
      energies[idx(i, j, w)].hl = color_diff(img[idx(i - 1, j, w)], img[idx(i + 1, j, w)]);
      int temp = calc_max(energies[idx(i, j, w)].hl, energies[idx(i, j, w)].hd, energies[idx(i, j, w)].hu);
      sc->max_energy_map = sc->max_energy_map > temp ? sc->max_energy_map : temp;
    }
  }
skip_horizontal:
  return;
}

void find_vseam_backward(seam_carve_t *sc)
{
  int i, j, k, w = sc->w - sc->current_w, h = sc->h - sc->current_h;
  float min = FLT_MAX, a, b, c;
  fext_t **m = sc->min;
  float *e = sc->energy_map_backward;
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
  // sc->vseam = realloc(sc->vseam, h * sizeof(*sc->vseam));

  for (i = h - 1; i >= 0; --i)
  {
    sc->vseam[i] = k;
    k = m[i][k].from;
  }
}

void find_hseam_backward(seam_carve_t *sc)
{
  int i, j, k, w = sc->w - sc->current_w, h = sc->h - sc->current_h;
  float min = FLT_MAX;
  fext_t **m = sc->min;
  float *e = sc->energy_map_backward;
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
  // sc->hseam = realloc(sc->hseam, w * sizeof(*sc->hseam));
  for (i = w - 1; i >= 0; --i)
  {
    sc->hseam[i] = k;
    k = m[k][i].from;
  }
}

void find_vseam_forward(seam_carve_t *sc)
{
  int w = sc->w - sc->current_w, h = sc->h - sc->current_h;
  // sc->energy_map_image = realloc(sc->energy_map_image, w * h * sizeof(*sc->energy_map_image));
  int i, j, k, index;
  float min = FLT_MAX, a, b, c, selected_pixel;
  pixel3_t *energy_img = sc->energy_map_image;
  fext_t **m = sc->min;
  cost_t *e = sc->energy_map_forward;
  // initialize the first row to energies
  float avg = 0;
  for (i = 0; i < w; ++i)
  {
    index = idx(0, i, w);
    m[0][i].val = e[index].vu;
    avg += e[index].vu;
    selected_pixel = e[index].vu;
    energy_img[index].r = (selected_pixel / MAX_ENERGY_FORWARD) * 255;
    energy_img[index].g = (selected_pixel / MAX_ENERGY_FORWARD) * 255;
    energy_img[index].b = (selected_pixel / MAX_ENERGY_FORWARD) * 255;
  }

  // iterate over energies and calculate minimum
  for (i = 1; i < h; ++i)
  {
    for (j = 0; j < w; ++j)
    {
      index = idx(i, j, w);
      a = m[i - 1][j].val + e[index].vu;
      b = j - 1 >= 0 ? m[i - 1][j - 1].val + e[index].vl : FLT_MAX;
      c = j + 1 < w ? m[i - 1][j + 1].val + e[index].vr : FLT_MAX;
      m[i][j].val = calc_min(a, b, c, j, &m[i][j].from);
      k = m[i][j].from;
      selected_pixel = (j == k) * e[index].vu +
                       (j + 1 == k) * e[index].vr +
                       (j - 1 == k) * e[index].vl;
      avg += selected_pixel;
      energy_img[index].r = (selected_pixel / MAX_ENERGY_FORWARD) * 255;
      energy_img[index].g = (selected_pixel / MAX_ENERGY_FORWARD) * 255;
      energy_img[index].b = (selected_pixel / MAX_ENERGY_FORWARD) * 255;
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
  // sc->vseam = realloc(sc->vseam, h * sizeof(*sc->vseam));

  for (i = h - 1; i >= 0; --i)
  {
    sc->vseam[i] = k;
    k = m[i][k].from;
  }
}

void find_hseam_forward(seam_carve_t *sc)
{
  int i, j, k, w = sc->w - sc->current_w, h = sc->h - sc->current_h;
  float min = FLT_MAX, selected_pixel;
  pixel3_t *energy_img = sc->energy_map_image;
  fext_t **m = sc->min;
  cost_t *e = sc->energy_map_forward;
  // initialize the first column to energies
  for (i = 0; i < h; ++i)
  {
    m[i][0].val = e[idx(i, 0, w)].hl;
    if (has_vseam(sc))
      continue;
    selected_pixel = e[idx(i, 0, w)].hl;
    energy_img[idx(i, 0, w)].r = (selected_pixel / MAX_ENERGY_FORWARD) * 255;
    energy_img[idx(i, 0, w)].g = (selected_pixel / MAX_ENERGY_FORWARD) * 255;
    energy_img[idx(i, 0, w)].b = (selected_pixel / MAX_ENERGY_FORWARD) * 255;
  }

  float a, b, c;
  // iterate over energies and calculate minimum

  for (j = 1; j < w; ++j)
  {
    for (i = 0; i < h; ++i)
    {
      a = m[i][j - 1].val + e[idx(i, j, w)].hl;
      b = i - 1 >= 0 ? m[i - 1][j - 1].val + e[idx(i, j, w)].hu : FLT_MAX;
      c = i + 1 < h ? m[i + 1][j - 1].val + e[idx(i, j, w)].hd : FLT_MAX;
      m[i][j].val = calc_min(a, b, c, i, &m[i][j].from);
      k = m[i][j].from;
      if (has_vseam(sc))
        continue;
      selected_pixel = (i == k) * e[idx(i, j, w)].hl +
                       (i + 1 == k) * e[idx(i, j, w)].hd +
                       (i - 1 == k) * e[idx(i, j, w)].hu;
      energy_img[idx(i, j, w)].r = (selected_pixel / MAX_ENERGY_FORWARD) * 255;
      energy_img[idx(i, j, w)].g = (selected_pixel / MAX_ENERGY_FORWARD) * 255;
      energy_img[idx(i, j, w)].b = (selected_pixel / MAX_ENERGY_FORWARD) * 255;
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
  // sc->hseam = realloc(sc->hseam, w * sizeof(*sc->hseam));
  for (i = w - 1; i >= 0; --i)
  {
    sc->hseam[i] = k;
    k = m[k][i].from;
  }
}

seam_carve_t *seam_carve_init(pixel3_t *img, int w, int h, int target_w, int target_h, MODE_T mode, bool gif)
{
  seam_carve_t *sc = malloc(sizeof(seam_carve_t));
  sc->mode = mode;
  sc->w = w;
  sc->h = h;
  sc->target_w = target_w;
  sc->target_h = target_h;
  sc->current_w = 0;
  sc->current_h = 0;
  sc->max_energy_map = 0;
  sc->vseam = malloc(h * sizeof(int));
  sc->hseam = malloc(w * sizeof(int));
  sc->energy_map_backward = mode == BACKWARD ? malloc(w * h * sizeof(float)) : NULL;
  sc->energy_map_forward = mode == FORWARD ? malloc(w * h * sizeof(cost_t)) : NULL;
  sc->img = img;
  sc->energy_map_image = malloc(w * h * sizeof(pixel3_t));
  sc->min = malloc(h * sizeof(fext_t *));
  for (int i = 0; i < h; i++)
  {
    sc->min[i] = malloc(w * sizeof(fext_t));
  }

  // Set functions
  sc->ef = mode == FORWARD ? &calc_energy_forward : &calc_energy_backward;
  sc->vsf = mode == FORWARD ? &find_vseam_forward : &find_vseam_backward;
  sc->hsf = mode == FORWARD ? &find_hseam_forward : &find_hseam_backward;
  sc->gif = NULL;
  if (gif)
  {
    pixel3_t pallet[256];
    for (int i = 0; i < 255; i++)
    {
      pallet[i].r = i;
      pallet[i].g = i;
      pallet[i].b = i;
    }
    pallet[255].r = 255;
    pallet[255].g = 0;
    pallet[255].b = 0;

    sc->gif =
        ge_new_gif("output/result.gif",  /* file name */
                   w, h,                 /* canvas size */
                   (uint8_t *)pallet, 8, /* palette depth == log2(# of colors) */
                   0                     /* infinite loop */
        );
  }

  return sc;
}
void calculate_energy(seam_carve_t *sc)
{
  sc->ef(sc);
}
void find_vseam(seam_carve_t *sc)
{
  sc->vsf(sc);
}
void find_hseam(seam_carve_t *sc)
{
  sc->hsf(sc);
}
void next_seam(seam_carve_t *sc)
{
  if (sc->gif)
    ge_add_frame(sc->gif, 7);

  sc->current_w += sc->current_w < sc->target_w;
  sc->current_h += sc->current_h < sc->target_h;
}
inline bool has_next(seam_carve_t *sc)
{
  return sc->current_w < sc->target_w || sc->current_h < sc->target_h;
}
inline bool has_vseam(seam_carve_t *sc)
{
  return sc->current_w < sc->target_w;
}
inline bool has_hseam(seam_carve_t *sc)
{
  return sc->current_h < sc->target_h;
}
void seam_carve_free(seam_carve_t *sc)
{
  free(sc->vseam);
  free(sc->hseam);
  free(sc->energy_map_image);
  switch (sc->mode)
  {
  case FORWARD:
    free(sc->energy_map_forward);
    break;
  case BACKWARD:
    free(sc->energy_map_backward);
    break;
  }

  for (int i = 0; i < sc->h; i++)
  {
    free(sc->min[i]);
  }

  free(sc->min);
  if (sc->gif)
    ge_close_gif(sc->gif);
  free(sc->img);
  free(sc);
}

inline size_t idx(size_t row, size_t col, int w)
{
  return row * w + col;
}

inline float calc_max(float a, float b, float c)
{
  return a > b ? (a > c ? a : c) : (b > c ? b : c);
}
inline float calc_min_index(float a, float b, float c, int j)
{
  return a < b ? (a < c ? j : j + 1) : (b < c ? j - 1 : j + 1);
}

inline float calc_min(float a, float b, float c, int j, int *index)
{
  *index = a < b ? (a < c ? j : j + 1) : (b < c ? j - 1 : j + 1);
  return a < b ? (a < c ? a : c) : (b < c ? b : c);
}

inline float color_diff(pixel3_t p0, pixel3_t p1)
{
  return pow(abs(p0.r - p1.r), 2) + pow(abs(p0.g - p1.g), 2) + pow(abs(p0.b - p1.b), 2);
}