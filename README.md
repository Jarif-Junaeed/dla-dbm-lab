# DLA-DBM Lab

DLA-DBM Lab is a small C program. It simulates diffusion-limited aggregation (DLA) on a two-dimensional grid. It writes an image and a video of the result.

> **NOTE:** This project is not complete. The DLA simulation works. The Laplacian growth part is not in the code yet.

## Planned work

The next part of the project is a Laplacian growth system. It will use the fuse breakdown algorithm from this paper:

Kim, T., Sewall, J., Sud, A., and Lin, M. C. "Fast Simulation of Laplacian Growth." *IEEE Computer Graphics and Applications*, 27(2), pages 68–76, 2007.

## How the DLA simulation works

The program makes a grid of cells. It puts one seed cell in the center of the grid. Then it releases particles one at a time.

Each particle moves one cell in a random direction at each step. The four directions are up, down, left, and right.

A particle that is next to the cluster can stick to it. The chance to stick is the stickiness factor. If the particle does not stick, it stays in the same cell for that step.

If a particle leaves the grid, the program puts it at a new spawn position. If a particle does not stick in the maximum number of steps, the program removes it.

When a particle sticks, it becomes part of the cluster. Then the program releases the next particle.

### Spawn modes

The program asks you to select one of three spawn modes when it starts.

| Mode | Name                  | Spawn position                                                                          |
| ---- | --------------------- | --------------------------------------------------------------------------------------- |
| 1    | Side-uniform          | The program selects one of the four sides. Each side has the same chance. Then it selects a random position on that side. |
| 2    | Pixel-uniform         | The program selects a random cell on the border. Each border cell has the same chance.  |
| 3    | Circumference-uniform | The program selects a random point on a circle around the center of the grid.           |

In mode 3, the circle starts with the default radius. The radius increases when the cluster becomes larger. The radius has a limit. The limit is half of the shorter side of the grid. The radius does not increase after it reaches this limit.

## Requirements

- A Linux computer. The program uses Linux system functions.
- A C compiler that supports C11.
- CMake version 3.16 or later.
- Git. CMake uses Git to download two libraries.
- An Internet connection for the first build.
- ffmpeg with libx264 support. The program uses ffmpeg to make the video.

CMake downloads these libraries:

- `pcg-c-basic` for random numbers.
- `inih` to read the INI file.

## Build

1. Open a terminal in the project folder.
2. Configure the project:

   ```sh
   cmake -S . -B build
   ```

3. Build the project:

   ```sh
   cmake --build build
   ```

The executable file is `build/main`.

## Run

> **CAUTION:** Start the program from the project folder. The program reads `config.ini` from the current folder. It also writes the `output` folder in the current folder.

If the program cannot read `config.ini`, it uses the default values.

1. Change the values in `config.ini`, if necessary.
2. Start the program:

   ```sh
   ./build/main
   ```

3. Enter `1`, `2`, or `3` to select the spawn mode.
4. Wait until the program shows `Animated DLA run`.

The program shows a message at the end of each stage.

## Output files

The program writes these files in the `output` folder.

| File            | Description                                                                          |
| --------------- | ------------------------------------------------------------------------------------ |
| `image.ppm`     | The final cluster. The cluster is black. The background is white. The format is PPM. |
| `animation.mp4` | A video of the run. The video has 120 frames each second. The particles are white.   |

The video has one frame for each particle move. A large run makes a long video and a large file.

## Configuration

The file `config.ini` contains these keys.

| Key                  | Default | Description                                                                  |
| -------------------- | ------- | ---------------------------------------------------------------------------- |
| `particle_count`     | 1000    | The number of particles that the program releases. Not all of them stick.    |
| `max_particle_steps` | 1000    | The maximum number of steps for each particle.                               |
| `width`              | 600     | The width of the grid, in cells.                                             |
| `height`             | 400     | The height of the grid, in cells.                                            |
| `default_radius`     | 10.00   | The start radius of the spawn circle in mode 3, in cells.                    |
| `stickiness_factor`  | 0.1     | The chance that a particle sticks when it is next to the cluster.            |

The program does not check these values. Use positive numbers. Use a number from 0 to 1 for `stickiness_factor`.

## Random numbers

The program uses the PCG32 random number generator. At each start, it gets a new seed from the operating system. Each run gives a different result. You cannot set the seed.

## Statistics

For each particle that sticks, the program records these data:

- The number of steps that the particle moved.
- The position where the particle stuck.
- The radius of gyration of the cluster.

The program does not write these data to a file yet.

## Source files

| File                    | Description                                          |
| ----------------------- | ---------------------------------------------------- |
| `src/main.c`            | The start of the program and the main loop.          |
| `src/particle.c`        | The random walk and the sticking of the particles.   |
| `src/spawn.c`           | The three spawn modes.                               |
| `src/clusterdelta.c`    | The list of changes that the animation uses.         |
| `src/render.c`          | The image file and the video.                        |
| `src/config.c`          | The reader for `config.ini`.                         |
| `src/rng.c`             | The start of the random number generator.            |
| `src/stats.c`           | The storage for the statistics.                      |

## License

This project uses the MIT License. Read the file `LICENSE` for more information.

## Credits

- `pcg-c-basic` by Melissa O'Neill.
- `inih` by Ben Hoyt.
