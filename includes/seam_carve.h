// contents of includes/seam_carve.h
// this file has all the information about what the program is doing
// the implementations are in main.c
#define MAX_ENERGY 624.61988441 // this value is calucated by maximizing the energy formula
#define CHANNEL 3               // Channel corresponds to RGB

typedef unsigned char pixel_t;

// this struct contains the RGB values for one pixel
typedef struct pixel3
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} pixel3_t;

// this struct is used to store values of shortest path
// every time we get the minimum of 3 elements and calulate
// the following forumla:
// Horizontal: M[i,j].val = E[i,j] + MIN(M[i,j-1] , M[i - 1,j-1] ,M[i + 1,j-1])
// Vertical: M[i,j].val = E[i,j] + MIN(M[i - 1,j] , M[i - 1,j-1] ,M[i - 1,j+1])
// where `M` is the table containing the shortest path and `E` contains energy of each pixel.
// The chosen element's index is stored in variable `from` of fext.

typedef struct fext
{
    float val;
    int from;
} fext_t;

// calculates energy of given image and returns an array of floats
// it also makes a another image from the enenrgy and returns it
// as an out prameter energy_img
float *calc_energy3(pixel3_t *img, int w, int h, pixel3_t **energy_img);

// calculates the min btween a,b and c and set the index based on
// following rules:
// if a is min j is the index
// if b is min j - 1 is the index
// if c is min j + 1 is the index
float calc_min(float a, float b, float c, int j, int *index);

// finds a vertical seam from table of energies
int *find_vseam(int w, int h, float *e);

// finds a horizontal seam from table of energies
int *find_hseam(int w, int h, float *e);

// removes two seams from img one vertical by vseam and one horizontal by hseam
pixel3_t *remove_seam(pixel3_t *img, int *vseam, int *hseam, int w, int h);

// draws a vertical seam on the energy image and also updates the gif
void draw_vseam(pixel3_t *img, int *vseam, int w, int h, ge_GIF *gif);

// draws a horizontal seam on the energy image and also updates the gif
void draw_hseam(pixel3_t *img, int *hseam, int w, int h, ge_GIF *gif);

// removes a vertical seam from img. vseam is an array of indecies
pixel3_t *remove_vseam(pixel3_t *img, int *vseam, int w, int h);

// removes a horizontal seam from img. hseam is an array of indecies
pixel3_t *remove_hseam(pixel3_t *img, int *hseam, int w, int h);

// converts a 2d index to 1d equivalant
size_t idx(size_t row, size_t col, int w);