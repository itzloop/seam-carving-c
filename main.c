#define _GNU_SOURCE
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <float.h>
#include <stdarg.h>
#include "lib/gifenc/gifenc.h"
#include "includes/seam_carve.h"
#include "lib/stb-image/stb_image.h"
#include "lib/stb-image/stb_image_write.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define NANO 1000000000

char *create_str(const char *format, ...)
{
  va_list args;
  char *buffer = calloc(255, sizeof(char));
  va_start(args, format);
  vsnprintf(buffer, 255, format, args);
  va_end(args);

  return buffer;
}

int main(int argc, char const *argv[])
{
  if (argc != 5)
  {
    printf("too few arguments!");
    return 1;
  }
  const char *filename = argv[1];
  int target_width = atoi(argv[2]);
  int target_height = atoi(argv[3]);
  int save_gif = atoi(argv[4]);

  if (target_height < 0 || target_width < 0)
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
  if (target_height > h || target_width > w)
  {
    printf("target size must be smaller that image size!\n");
    stbi_image_free(img);
    return 1;
  }
  if (target_height == 0 && target_width == 0)
  {
    printf("target size is 0. No point in running the program!\n");
    stbi_image_free(img);
    return 1;
  }
  struct stat st = {0};

  if (stat("output/", &st) == -1)
  {
    mkdir("output/", 0700);
  }

  pixel3_t *resized_img = NULL, *energy_img = NULL;
  float *e;           // energy table
  int *vseam, *hseam; // vertical and horizontal seams

  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);

  // red is 255
  // this is the collors used in the gif
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

  ge_GIF *gif =
      save_gif == 0 ? NULL : ge_new_gif("output/result.gif",  /* file name */
                                        w, h,                 /* canvas size */
                                        (uint8_t *)pallet, 8, /* palette depth == log2(# of colors) */
                                        1                     /* infinite loop */
                             );
  int i = 0, j = 0;
  printf("calculation in progress...");
  fflush(stdout);
  while (i < target_height || j < target_width)
  {
    printf("i:%d , j: %d\n", i, j);
    resized_img = NULL;
    e = calc_energy3(img, w - j, h - i, &energy_img);
    vseam = j < target_width ? find_vseam(w - j, h - i, e) : NULL;
    hseam = i < target_height ? find_hseam(w - j, h - i, e) : NULL;
    if (vseam)
    {
      draw_vseam(energy_img, vseam, w - j, h - i, hseam ? NULL : gif);
      // ge_add_frame(vertical_gif, 10);
      resized_img = remove_vseam(img, vseam, w - (j + 1), h - i);
      if (hseam)
      {
        free(img);
        img = resized_img;
      }
      free(vseam);
    }
    if (hseam)
    {
      draw_hseam(energy_img, hseam, w - j, h - i, gif);
      resized_img = remove_hseam(img, hseam,
                                 vseam ? w - (j + 1) : w - target_width,
                                 h - (i + 1));
      free(hseam);
    }

    if (gif)
      ge_add_frame(gif, 7);

    i += 1 * (i < target_height);
    j += 1 * (j < target_width);

    free(img);
    img = resized_img;
    free(e);
    free(energy_img);
  }

  clock_gettime(CLOCK_REALTIME, &end);
  printf("\nrunnig time(seconds): %lf\n",
         end.tv_sec - start.tv_sec +
             (double)(end.tv_nsec - start.tv_nsec) / NANO);

  stbi_write_png("output/result.png",
                 w - target_width,
                 h - target_height,
                 CHANNEL,
                 resized_img,
                 (w - target_width) * CHANNEL);

  free(resized_img);
  if (gif)
    ge_close_gif(gif);
  return 0;
}
