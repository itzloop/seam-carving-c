#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "includes/seam_carve.h"
#include "lib/gifenc/gifenc.h"

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

pixel3_t *remove_vseam(pixel3_t *img, int *vseam, int w, int h)
{
    if (vseam == NULL)
        return NULL;

    pixel3_t *resized_img = malloc(w * h * sizeof(*resized_img));

    for (int i = 0; i < h; ++i)
    {
        for (int j = 0, l = 0; j < w + 1; ++j, ++l)
        {
            if (j == vseam[i])
                j++;
            resized_img[idx(i, l, w)].r = img[idx(i, j, w + 1)].r;
            resized_img[idx(i, l, w)].g = img[idx(i, j, w + 1)].g;
            resized_img[idx(i, l, w)].b = img[idx(i, j, w + 1)].b;
        }
    }

    return resized_img;
}

pixel3_t *remove_hseam(pixel3_t *img, int *hseam, int w, int h)
{
    if (hseam == NULL)
        return NULL;
    pixel3_t *resized_img = malloc(w * h * sizeof(*resized_img));

    for (int j = 0; j < w; ++j)
    {
        for (int i = 0, l = 0; i < h + 1; ++i, ++l)
        {
            if (i == hseam[j])
                i++;

            resized_img[idx(l, j, w)].r = img[idx(i, j, w)].r;
            resized_img[idx(l, j, w)].g = img[idx(i, j, w)].g;
            resized_img[idx(l, j, w)].b = img[idx(i, j, w)].b;
        }
    }
    return resized_img;
}

float *calc_energy3(pixel3_t *img, int w, int h, pixel3_t **energy_img)
{
    int rx, ry, gx, gy, bx, by;
    float dx, dy;
    *energy_img = malloc(w * h * sizeof(*energy_img));
    float *energies = malloc(w * h * sizeof(float));

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
            energies[idx(i, j, w)] = sqrt(dx + dy);

            (*energy_img)[idx(i, j, w)].r = (energies[idx(i, j, w)] / MAX_ENERGY) * 255;
            (*energy_img)[idx(i, j, w)].g = (energies[idx(i, j, w)] / MAX_ENERGY) * 255;
            (*energy_img)[idx(i, j, w)].b = (energies[idx(i, j, w)] / MAX_ENERGY) * 255;
        }
    }

    return energies;
}

size_t idx(size_t row, size_t col, int w)
{
    return row * w + col;
}
