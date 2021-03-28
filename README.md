# RandomArt

This is a C++ implementation of a random art generator. It is based on and very similar in structure and function to the [random-art.org Python implementation](http://www.random-art.org/about/), but much faster.
All of the following images were generated by this program. I'm using one of them for my profile picture.

![](collage.png)

## Building

Running `make` should compile the program, yielding the executable `./run`. The program has no external dependencies.

## Usage

```
./run "some text"
```

This will generate a random image, using the text you provide as a seed. This is deterministic; the image is always the same for a given input string.

```
./run
```

Running the program without arguments will use the current time in milliseconds as the seed.

In both cases, the image is written in Netpbm PPM format out to `random.ppm`. By default the program generates a 1000x1000 image, but this can be easily changed by editing `main.cpp`

## Functionality

The program creates a tree of image operations with a specified depth. The default depth is 5. Increasing the depth increases both runtime and level of detail. This is an example tree:

![](example.png)

The actual trees generated by the program are much larger.
