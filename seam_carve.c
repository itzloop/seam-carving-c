#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "includes/seam_carve.h"
#include "lib/gifenc/gifenc.h"
#include <stdio.h>
#include <stdbool.h>

float calc_min(float a, float b, float c, int j, int *index)
{
    *index = a < b ? (a < c ? j : j + 1) : (b < c ? j - 1 : j + 1);
    return a < b ? (a < c ? a : c) : (b < c ? b : c);
}

void find_vseam(int **seam, int w, int h, float *e, fext_t ***m)
{
    int i, j, k;
    float min = FLT_MAX, a, b, c;
    // *m = realloc(*m, h * sizeof(fext_t *));
    // for (i = 0; i < h; ++i)
    // {
    //     (*m)[i] = realloc((*m)[i], w * sizeof(fext_t));
    // }

    // initialize the first row to energies
    for (i = 0; i < w; ++i)
    {
        (*m)[0][i].val = e[idx(0, i, w)];
    }

    // iterate over energies and calculate minimum
    for (i = 1; i < h; ++i)
    {
        for (j = 0; j < w; ++j)
        {
            a = (*m)[i - 1][j].val;
            b = j - 1 >= 0 ? (*m)[i - 1][j - 1].val : FLT_MAX;
            c = j + 1 < w ? (*m)[i - 1][j + 1].val : FLT_MAX;
            (*m)[i][j].val = e[idx(i, j, w)] + calc_min(a, b, c, j, &(*m)[i][j].from);
        }
    }

    // find the minimum number at the end row
    for (k = 0, i = 0; i < w; ++i)
    {
        if (min > (*m)[h - 1][i].val)
        {
            min = (*m)[h - 1][i].val;
            k = i;
        }
    }
    // bactrace to top and create the seam
    *seam = *seam == NULL ? malloc(h * sizeof(int)) : realloc(*seam, h * sizeof(int));

    for (i = h - 1; i >= 0; --i)
    {
        (*seam)[i] = k;
        k = (*m)[i][k].from;
    }
}

void find_hseam(int **seam, int w, int h, float *e, fext_t ***m)
{
    int i, j, k;
    float min = FLT_MAX;
    // *m = realloc(*m, h * sizeof(fext_t *));

    // for (i = 0; i < h; ++i)
    // {
    //     (*m)[i] = realloc((*m)[i], w * sizeof(fext_t));
    // }

    // initialize the first column to energies
    for (i = 0; i < h; ++i)
    {
        (*m)[i][0].val = e[idx(i, 0, w)];
    }

    float a, b, c;
    // iterate over energies and calculate minimum
    for (j = 1; j < w; ++j)
    {
        for (i = 0; i < h; ++i)
        {
            a = (*m)[i][j - 1].val;
            b = i - 1 >= 0 ? (*m)[i - 1][j - 1].val : FLT_MAX;
            c = i + 1 < h ? (*m)[i + 1][j - 1].val : FLT_MAX;
            (*m)[i][j].val = e[idx(i, j, w)] + calc_min(a, b, c, i, &(*m)[i][j].from);
        }
    }

    // find the minimum number at the end column
    for (k = 0, i = 0; i < h; ++i)
    {
        if (min > (*m)[i][w - 1].val)
        {
            min = (*m)[i][w - 1].val;
            k = i;
        }
    }
    // bactrace to top and create the seam
    *seam = *seam == NULL ? malloc(w * sizeof(int)) : realloc(*seam, w * sizeof(int));
    for (i = w - 1; i >= 0; --i)
    {
        (*seam)[i] = k;
        k = (*m)[k][i].from;
    }

    // free resources
    // for (int j = 0; j < h; ++j)
    // {
    //     free(m[j]);
    // }
    // free(m);
}

void draw_vseam(pixel3_t *img, int *vseam, int w, int h, ge_GIF *gif)
{
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

void draw_hseam(pixel3_t *img, int *hseam, int w, int h, ge_GIF *gif)
{
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

void remove_vseam(pixel3_t **img, int *vseam, int w, int h)
{
    if (vseam == NULL || *img == NULL)
    {
        printf("vseam or img can't be null\n");
        return;
    }

    pixel3_t *temp = *img;

    for (int i = 0; i < h; ++i)
    {
        for (int j = 0, l = 0; j < w + 1; ++j, ++l)
        {
            if (j == vseam[i])
                j++;
            temp[idx(i, l, w)].r = temp[idx(i, j, w + 1)].r;
            temp[idx(i, l, w)].g = temp[idx(i, j, w + 1)].g;
            temp[idx(i, l, w)].b = temp[idx(i, j, w + 1)].b;
        }
    }

    *img = realloc(*img, w * h * sizeof(pixel3_t));
}

void remove_hseam(pixel3_t **img, int *hseam, int w, int h)
{
    if (hseam == NULL || *img == NULL)
    {
        printf("hseam or img can't be null\n");
        return;
    }
    pixel3_t *temp = *img;

    for (int j = 0; j < w; ++j)
    {
        for (int i = 0, l = 0; i < h + 1; ++i, ++l)
        {
            if (i == hseam[j])
                i++;

            temp[idx(l, j, w)].r = temp[idx(i, j, w)].r;
            temp[idx(l, j, w)].g = temp[idx(i, j, w)].g;
            temp[idx(l, j, w)].b = temp[idx(i, j, w)].b;
        }
    }
    *img = realloc(*img, w * h * sizeof(pixel3_t));
}

void calc_energy3(pixel3_t *img, int w, int h, pixel3_t **energy_img, float **e)
{
    int rx, ry, gx, gy, bx, by;
    float dx, dy;
    *energy_img = *energy_img == NULL ? malloc(w * h * sizeof(*energy_img)) : realloc(*energy_img, w * h * sizeof(*energy_img));

    *e = *e == NULL ? malloc(w * h * sizeof(float)) : realloc(*e, w * h * sizeof(float));
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
            (*e)[idx(i, j, w)] = sqrt(dx + dy);

            (*energy_img)[idx(i, j, w)].r = ((*e)[idx(i, j, w)] / MAX_ENERGY) * 255;
            (*energy_img)[idx(i, j, w)].g = ((*e)[idx(i, j, w)] / MAX_ENERGY) * 255;
            (*energy_img)[idx(i, j, w)].b = ((*e)[idx(i, j, w)] / MAX_ENERGY) * 255;
        }
    }
}

float color_diff(pixel3_t p0, pixel3_t p1)
{
    return pow(abs(p0.r - p1.r), 2) + pow(abs(p0.g - p1.g), 2) + pow(abs(p0.b - p1.b), 2);
}

void calc_energy_forward(pixel3_t *img, int w, int h, pixel3_t **energy_img, cost_t **e)
{
    *energy_img = *energy_img == NULL ? malloc(w * h * sizeof(*energy_img)) : realloc(*energy_img, w * h * sizeof(*energy_img));

    *e = *e == NULL ? malloc(w * h * sizeof(cost_t)) : realloc(*e, w * h * sizeof(cost_t));

    pixel3_t black = {0, 0, 0};

    // cost of first row
    for (int j = 0; j < w; j++)
    {
        (*e)[idx(0, j, w)].l = 0;
        (*e)[idx(0, j, w)].r = 0;
        (*e)[idx(0, j, w)].u =
            color_diff(j - 1 >= 0 ? img[idx(j - 1, 0, w)] : black,
                       j + 1 < w ? img[idx(j + 1, 0, w)] : black);
    }

    // cost of left and right col
    for (int i = 0; i < w; i++)
    {
        // left col
        (*e)[idx(i, 0, w)].l =
            color_diff(img[idx(i, 0, w)], img[idx(i, 1, w)]) +
            color_diff(img[idx(i - 1, 0, w)], img[idx(i, 1, w)]);
        (*e)[idx(i, 0, w)].r =
            color_diff(img[idx(i, 0, w)], img[idx(i, 1, w)]) +
            color_diff(img[idx(i - 1, 0, w)], img[idx(i, 1, w)]);
        (*e)[idx(i, 0, w)].u =
            color_diff(img[idx(i, 0, w)], img[idx(i, 1, w)]);
        // right col
        (*e)[idx(i, w - 1, w)].l =
            color_diff(img[idx(i, w - 2, w)], img[idx(i, w - 1, w)]) +
            color_diff(img[idx(i - 1, w - 2, w)], img[idx(i, w - 1, w)]);
        (*e)[idx(i, w - 1, w)].r =
            color_diff(img[idx(i, w - 2, w)], img[idx(i, w - 1, w)]) +
            color_diff(img[idx(i - 1, w - 2, w)], img[idx(i, w - 1, w)]);
        (*e)[idx(i, w - 1, w)].u =
            color_diff(img[idx(i, w - 2, w)], img[idx(i, w - 1, w)]);
    }

    for (int i = 1; i < h; i++)
    {
        for (int j = 1; j < w - 1; j++)
        {
            (*e)[idx(i, j, w)].l =
                color_diff(img[idx(i, j - 1, w)], img[idx(i, j + 1, w)]) +
                color_diff(img[idx(i - 1, j, w)], img[idx(i, j - 1, w)]);
            (*e)[idx(i, j, w)].r =
                color_diff(img[idx(i, j - 1, w)], img[idx(i, j + 1, w)]) +
                color_diff(img[idx(i - 1, j, w)], img[idx(i, j + 1, w)]);
            (*e)[idx(i, j, w)].u = color_diff(img[idx(i - 1, j, w)], img[idx(i + 1, j, w)]);
        }
    }
}

void find_vseam_forward(int **seam, int w, int h, cost_t *e, fext_t ***m)
{
    int i, j, k;
    float min = FLT_MAX, a, b, c;
    // *m = realloc(*m, h * sizeof(fext_t *));
    // for (i = 0; i < h; ++i)
    // {
    //     (*m)[i] = realloc((*m)[i], w * sizeof(fext_t));
    // }

    // initialize the first row to energies
    for (i = 0; i < w; ++i)
    {
        (*m)[0][i].val = e[idx(0, i, w)].u;
    }

    // iterate over energies and calculate minimum
    for (i = 1; i < h; ++i)
    {
        for (j = 0; j < w; ++j)
        {
            a = (*m)[i - 1][j].val + e[idx(i, j, w)].u;
            b = j - 1 >= 0 ? (*m)[i - 1][j - 1].val + e[idx(i, j, w)].l : FLT_MAX;
            c = j + 1 < w ? (*m)[i - 1][j + 1].val + e[idx(i, j, w)].r : FLT_MAX;
            (*m)[i][j].val = calc_min(a, b, c, j, &(*m)[i][j].from);
        }
    }

    // find the minimum number at the end row
    for (k = 0, i = 0; i < w; ++i)
    {
        if (min > (*m)[h - 1][i].val)
        {
            min = (*m)[h - 1][i].val;
            k = i;
        }
    }
    // bactrace to top and create the seam
    *seam = *seam == NULL ? malloc(h * sizeof(int)) : realloc(*seam, h * sizeof(int));

    for (i = h - 1; i >= 0; --i)
    {
        (*seam)[i] = k;
        k = (*m)[i][k].from;
    }
}

size_t idx(size_t row, size_t col, int w)
{
    return row * w + col;
}
