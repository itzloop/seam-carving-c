# Seam carving algorithm using c and stb-image

## TODO

- [x] handle multiple seam removals in one go
- [x] add horizontal seam removal
- [x] update energy image each time
- [ ] make a cool cli

## Build

For building the project just do make.

```bash
$ make
```

## Usage

Four arguments is needed to run this program. First the name of the file then the number of pixels to be deleted from width and heigh of the image. The last argument is an integer specifying to save the steps of algorithm a gif or not. 0 for storting only the final image and any other number for storing the steps as well. The reason is that saving a gif requires a lot of IO operations so it's not fast. For heavier i.e. large size images or big resize targets don't save the steps only use the final result.
There is picture inside data folder sample.png the size is 612x345 if you run the following, a new directory will be created called output and and the final image result.png will be stored there. Notice the gif of the steps won't be created. This command will reduce the size of this image to 312x45.

```
$ ./main.o data/sample.png 300 300 0
```

With this command the same will happen but with storing the gif aswell.

```
$ ./main.o filename.png 300 300 1
```

For further details on the code see includes/seam_carve.h
