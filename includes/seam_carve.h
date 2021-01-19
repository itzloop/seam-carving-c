#include <stdbool.h>
// contents of includes/seam_carve.h
// this file has all the information about what the program is doing
// the implementations are in seam_carve.c
#define MAX_ENERGY_BACKWARD 624.61988441 // this value is calucated by maximizing the energy formula
#define CHANNEL 3                        // Channel corresponds to RGB
#define MAX_ENERGY_FORWARD 195075
// forward declation
typedef struct ge_GIF ge_GIF;

// this struct contains the RGB values for one pixel
typedef struct pixel3
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} pixel3_t;

/*
Now for calculating forward we first assume that we remove a seam and
calculate the pixel difference of those pixels that are becoming neighbors
if we delete that seam.
Assume this structure of pixels and we're on pixel x which is p(i,j).
+---+---+---+
| 1 | 2 | 3 |
+---+---+---+
| 4 | x | 6 |
+---+---+---+
| 7 | 8 | 9 |
+---+---+---+

Vertical Seam:

we have three options for vertical seam and i wrote the corresponding 
name i used for each value in the struct bellow:
1->x(vertical left): by removing this seam pixels 2(i - 1 , j) and 4(i , j - 1) and pixels 4(i , j - 1) and 6(i , j + 1) will become neighbors
2->x(vertical up): by removing this seam pixels 4(i , j - 1) and 6(i , j + 1) will become neighbors
3->x(vertical right): by removing this seam pixels 2(i - 1 , j) and 6(i , j + 1) and pixels 4(i , j - 1) and 6(i , j + 1) will become neighbors

now we have 3 values for energy of p(i,j) (instead of 1 with backward energy) so 
we have to see which one gives the minimum value when we're finding the shortest path

e is cost_t
and M is fext_t

M(i,j) = MIN {
    M[i - 1][j].vall + e.vu
    M[i - 1][j - 1].val + e.vl,
    M[i - 1][j + 1].val + e.vr,
}

the rest is the same as backward energy

Horizontal Seam:

1->x(horzontal up): by removing this seam pixels 2(i - 1 , j) and 4(i , j - 1) and pixels 2(i - 1 , j) and 8(i + 1 , j) will become neighbors
4->x(horzontal left): by removing this seam pixels 2(i - 1 , j) and 8(i + 1 , j) will become neighbors
7->x(horzontal down): by removing this seam pixels 4(i , j - 1) and 8(i + 1 , j) and pixels 2(i - 1 , j) and 8(i + 1 , j) will become neighbors

M(i,j) = MIN {
    M[i][j - 1].val + e.hl,
    M[i - 1][j - 1].val + e.hu,
    M[i + 1][j - 1].val + e.hd,
}

the rest is the same as backward energy

*/

typedef struct cost
{
    float vl; // vertical left
    float vr; // vertical right
    float vu; // vertical up
    float hl; // horizontal left
    float hu; // horizontal up
    float hd; // horizontal down
} cost_t;

/* 
this struct is used to store values of shortest path
every time we get the minimum of 3 elements and calulate
the following forumla:
Horizontal: M[i,j].val = E[i,j] + MIN(M[i,j-1] , M[i - 1,j-1] ,M[i + 1,j-1])
Vertical: M[i,j].val = E[i,j] + MIN(M[i - 1,j] , M[i - 1,j-1] ,M[i - 1,j+1])
where `M` is the table containing the shortest path and `E` contains energy of each pixel.
The chosen element's index is stored in variable `from` of fext.
*/
typedef struct fext
{
    float val;
    int from;
} fext_t;

typedef struct seam_carve seam_carve_t;
typedef void (*energy_function)(seam_carve_t *sc);
typedef void (*seam_finder)(seam_carve_t *sc);
typedef enum MODE
{
    FORWARD,
    BACKWARD
} MODE_T;

typedef struct seam_carve
{
    int w;
    int h;
    int target_w;
    int target_h;
    int current_w;
    int current_h;
    int max_energy_map;
    int *vseam;                 // vertical seam
    int *hseam;                 // horizontal seam
    float *energy_map_backward; // the energy map for when we're using the backward energy
    pixel3_t *img;              // the image that we're resizing
    pixel3_t *energy_map_image; // the energy map image of img
    cost_t *energy_map_forward; // the energy map for when we're using the forward energy
    ge_GIF *gif;                // gif each energy_map_image
    fext_t **min;               // min table;
    energy_function ef;         // energy function
    seam_finder vsf;            // vertical seam finder
    seam_finder hsf;            // horizontal seam finder
    MODE_T mode;                // either Forward or Backward
} seam_carve_t;

// calculates energy of given image and returns an array of floats
// it also makes a another image from the enenrgy and returns it
// as an out prameter energy_img
void calc_energy_backward(seam_carve_t *sc);
void calc_energy_forward(seam_carve_t *sc);
// void find_vseam_forward(seam_carve_t *sc);
// void find_hseam_forward(seam_carve_t *sc);
// calculates the min btween a,b and c and set the index based on
// following rules:
// if a is min j is the index
// if b is min j - 1 is the index
// if c is min j + 1 is the index
float calc_min(float a, float b, float c, int j, int *index);

// finds a vertical seam from table of energies and stores it in seam
// void find_vseam_backward(seam_carve_t *sc);

// finds a horizontal seam from table of energies and stores it in seam
// void find_hseam_backward(seam_carve_t *sc);
void find_vseam(seam_carve_t *sc);
void find_hseam(seam_carve_t *sc);
// draws a vertical seam on the energy image and also updates the gif
void draw_vseam(seam_carve_t *sc);

// draws a horizontal seam on the energy image and also updates the gif
void draw_hseam(seam_carve_t *sc);

// removes a vertical seam from img. vseam is an array of indecies
void remove_vseam(seam_carve_t *sc);

// removes a horizontal seam from img. hseam is an array of indecies
void remove_hseam(seam_carve_t *sc);

// converts a 2d index to 1d equivalant
size_t idx(size_t row, size_t col, int w);
// get the next vertical and horizontal seam
seam_carve_t *seam_carve_init(pixel3_t *img, int w, int h, int target_w, int target_h, MODE_T mode, bool gif);
void seam_carve_free(seam_carve_t *sc);
void calculate_energy(seam_carve_t *sc);
void next_seam(seam_carve_t *sc);
bool has_next(seam_carve_t *sc);
// void draw_seam(seam_carve_t *sc);
// void remove_seam(seam_carve_t *sc);
bool has_vseam(seam_carve_t *sc);
bool has_hseam(seam_carve_t *sc);