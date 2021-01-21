# Seam carving algorithm using c and stb-image

## TODO

- [x] handle multiple seam removals in one go
- [x] add horizontal seam removal
- [x] update energy image each time
- [ ] make a cool cli

## Build

For building the project just run make command.

```bash
$ make
```

## Usage

Five arguments are needed to run this program. First the name of the file then the number of pixels to be deleted from width and heigh of the image and one integer for mode which is either 1 or `FORWARD`, 0 or `BACKWARD`. The last argument is an integer again specifying to save the steps of algorithm as a gif or not. 0 for storting only the final image and any other number for storing the steps as well. The reason is that saving a gif requires a lot of IO operations so it's not fast. For heavier i.e. large size images or big resize targets don't save the steps only use the final result.
There is picture inside data folder sample.png the size is 612x345 if you run the following, a new directory will be created called output and and the final image result.png will be stored there. Notice the gif of the steps won't be created. This command will reduce the size of this image to 312x45.

```
./main.o filename target_width target_height mode save_gif
$ ./main.o data/sample.png 300 300 0 0
```

With this command the same will happen but with storing the gif aswell.

```
$ ./main.o filename.png 300 300 0 1
```

## Explaination

`BACKWARD` is the original algorithm which calulcated energies using dual gradient but, this approach does not take into account the energy of pixels getting close by removing a seam in between them. For this reason there is the `FORWARD` method which calulates the 3 values for each pixel called `cost`. Each of them corresponds to the color difference (given two pixels `p1` and `p2`, `diff = (p1.r-p2.r)^2 + (p1.g-p2.g)^2 + (p1.b-p2.b)^2`) of one seam. There is better explaination in the header and also in this [link](https://avikdas.com/2019/07/29/improved-seam-carving-with-forward-energy.html)

For further details on the code see includes/seam_carve.h and read seam_carve.c for implementation details.

## Comparison between two calculation methods

Command used in this comparison:
```bash
# BACKWARD
$ ./main.o geese.jpg 300 100 0 1
# FORWARD
$ ./main.o geese.jpg 300 100 1 1
```
### Original Image:
<p align="center">
  <img src="https://drive.google.com/uc?id=1lAryTTbygeFwmiwnSbe-CZR8hBgIZq4K&export=download" alt="Original image" />
</p>
## Using backward energy

<p align="center">
  <img src="https://drive.google.com/uc?id=1GMxwQKUiYhQ8GswoBGakljOCwS1OeAAB&export=download" alt="Backward Image"/>
</p>

[**Backward Gif**](https://drive.google.com/uc?id=1LXIR1zWSb_PvPcz04VM76owynwO8G3eC&export=download)




## Using forward energy

<p align="center">
  <img src="https://drive.google.com/uc?id=1-TeRC6zwukyPQfoaRwH1TSxYEMeoLhtF&export=download" alt="Forward Image"/>
  
</p>

[**Forward Gif**](https://drive.google.com/uc?id=1BOZDMDtXYX4g-eT2nU3KjV52OwZg044B&export=download)


**The original image is from the link given above in the Explaination section but resized image and the gifs are the outputs of my program.**

