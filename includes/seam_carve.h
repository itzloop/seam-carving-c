// contents of includes/seam_carve.h
// this file has all the information about what the program is doing
// the implementations are in seam_carve.c
#include <stdbool.h>

#define MAX_ENERGY_BACKWARD 624.61988441 // this value is calucated by maximizing the energy formula
#define CHANNEL 3						 // Channel corresponds to RGB
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
// Function pointer for energy fucntions. calc_energy_backward or calc_energy_forward
typedef void (*energy_function)(seam_carve_t *sc);
// Function pointer for finding a seam. find or calc_energy_forward
typedef void (*seam_finder)(seam_carve_t *sc);

// MODE represents running this program with original algorithm or with forward energy
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
	int *vseam;					// vertical seam
	int *hseam;					// horizontal seam
	float *energy_map_backward; // the energy map for when we're using the backward energy
	pixel3_t *img;				// the image that we're resizing
	pixel3_t *energy_map_image; // the energy map image of img
	cost_t *energy_map_forward; // the energy map for when we're using the forward energy
	ge_GIF *gif;				// gif each energy_map_image
	fext_t **min;				// min table;
	energy_function ef;			// energy function
	seam_finder vsf;			// vertical seam finder
	seam_finder hsf;			// horizontal seam finder
	MODE_T mode;				// either Forward or Backward
} seam_carve_t;

// finds a vertical seam from table of energies and stores it in vseam
void find_vseam(seam_carve_t *sc);

// finds a horizontal seam from table of energies and stores it in hseam
void find_hseam(seam_carve_t *sc);

// draws vseam (vertical seam) on the energy image and also updates the gif
void draw_vseam(seam_carve_t *sc);

// draws hseam (horizontal seam) on the energy image and also updates the gif
void draw_hseam(seam_carve_t *sc);

// removes vseam (vertical seam) from img.
void remove_vseam(seam_carve_t *sc);

// removes hseam (horizontal seam) from img.
void remove_hseam(seam_carve_t *sc);

// initializes everything needed to resize an image and returns a pointer to the struct.
seam_carve_t *seam_carve_init(pixel3_t *img, int w, int h, int target_w, int target_h, MODE_T mode, bool gif);

// given the same pointer returned from seam_carve_init it will free all the resources
// allocated. It will also frees the img which was alocated by the user :).
void seam_carve_free(seam_carve_t *sc);

// calculates energy of given image and store it in energy map
// it also makes a another image from the enenrgy map
// if the mode is BACKWARD and store it in energy_map_image
void calculate_energy(seam_carve_t *sc);

// add 1 to current_h or current_w if we have seam in that direction
void next_seam(seam_carve_t *sc);

// Do we have any seam to find or end?
bool has_next(seam_carve_t *sc);

//Do we have a vertical seam?
bool has_vseam(seam_carve_t *sc);

// Do we have a horizontal seam?
bool has_hseam(seam_carve_t *sc);