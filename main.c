#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define ITERATIONS 10000
#define COLOR_CHANNELS 3
#define WIDTH 600
#define HEIGHT 400

inline int PixelIndex(int pixel) { return pixel * COLOR_CHANNELS; }

int render(bool *grid) {
  const int screen_size = WIDTH * HEIGHT * COLOR_CHANNELS;
  const int pixel_count = WIDTH * HEIGHT;

  int *screen = malloc(screen_size * sizeof(int));
  if (screen == NULL) {
    perror("Could not allocate memory for screen");
    return 1;
  }

  for (int pixel = 0; pixel < pixel_count; pixel++) {
    int i = pixel * COLOR_CHANNELS;

    if (pixel < pixel_count / 2) {
      screen[i] = 255;     // R
      screen[i + 1] = 255; // G
      screen[i + 2] = 255; // B
    } else {
      screen[i] = 0;
      screen[i + 1] = 0;
      screen[i + 2] = 0;
    }
  }

  if (mkdir("output", 0755) == -1 && errno != EEXIST) {
    perror("mkdir");
    return 1;
  }

  FILE *fp = fopen("output/image.ppm", "w");
  if (fp == NULL) {
    perror("Could not open image.ppm");
    return 1;
  }

  fprintf(fp, "P3\n%d %d\n255\n", WIDTH, HEIGHT);

  for (int pixel = 0; pixel < pixel_count; pixel++) {
    int i = pixel * COLOR_CHANNELS;

    fprintf(fp, "%d %d %d\n", screen[i], screen[i + 1], screen[i + 2]);
  }

  free(screen);
  fclose(fp);

  return 0;
}

void walk(int x, int y, bool *grid) {
  
}

int main(void) {
  const int grid_size = WIDTH * HEIGHT;

  const int grid_center = (HEIGHT / 2) * WIDTH + (WIDTH / 2);
  bool *grid = calloc(grid_size, sizeof(bool));
  if (grid == NULL) {
    perror("could not allocate memory for grid");
    return -1;
  }

  grid[grid_center] = 1;

  for (int i = 0; i < ITERATIONS; i++) {
    int side = rand() % 4;
    int x, y;
    switch (side) {
      case 0:
          x = rand() % WIDTH; 
          y = 0;
          break;
      case 1:
          x = WIDTH - 1;
          y = rand() % HEIGHT;
          break;
      case 2:
          x = rand() % WIDTH;
          y = HEIGHT - 1;
          break;
      case 3:
          x = 0;
          y = rand() % HEIGHT;
          break;

    }

  }

  if (render(grid) == 1) {
    return 1;
  }

  free(grid);

  return 0;
}
