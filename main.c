#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define ITERATIONS 10000
#define COLOR_CHANNELS 3
#define WIDTH 600
#define HEIGHT 400

inline int PixelIndex(int pixel) { return pixel * COLOR_CHANNELS; }

void SpawnSideUniform(int *x, int *y) {
  int w = WIDTH - 1;
  int h = HEIGHT - 1;
  int side = rand() % 4;
  switch (side) {
  case 0:
    *x = rand() % w + 1;
    *y = 0;
    break;
  case 1:
    *x = WIDTH - 1;
    *y = rand() % h + 1;
    break;
  case 2:
    *x = rand() % w;
    *y = HEIGHT - 1;
    break;
  case 3:
    *x = 0;
    *y = rand() % h;
    break;
  }
}

void SpawnPixelUniform(int *x, int *y) {
  int w = WIDTH - 1;
  int h = HEIGHT - 1;
  int perimeter = 2 * (w + h);
  int r = rand() % perimeter;

  if (r < w) {
    *x = r + 1;
    *y = 0;
  } else if (r < w + h) {
    *x = w;
    *y = (r - w) + 1;
  } else if (r < (2 * w + h)) {
    *x = r - (w + h);
    *y = h;
  } else {
    *x = 0;
    *y = r - ((2 * w) + h);
  }
}

int Render(bool *grid) {
  const int unsigned screen_size = WIDTH * HEIGHT * COLOR_CHANNELS;
  const int unsigned pixel_count = WIDTH * HEIGHT;

  int *screen = malloc(screen_size * sizeof(int));
  if (screen == NULL) {
    perror("Could not allocate memory for screen");
    return 1;
  }

  for (unsigned int pixel = 0; pixel < pixel_count; pixel++) {
    int i = pixel * COLOR_CHANNELS;

    if (pixel < pixel_count / 2) {
      screen[i] = 255;     // R
      screen[i + 1] = 255; // G
      screen[i + 2] = 255; // B
    } else {
      screen[i] = 0;     // R
      screen[i + 1] = 0; // G
      screen[i + 2] = 0; // B
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

  for (unsigned int pixel = 0; pixel < pixel_count; pixel++) {
    int i = pixel * COLOR_CHANNELS;

    fprintf(fp, "%d %d %d\n", screen[i], screen[i + 1], screen[i + 2]);
  }

  free(screen);
  fclose(fp);

  return 0;
}

void Walk(int x, int y, bool *grid) {}

int main(void) {
  const unsigned int grid_size = WIDTH * HEIGHT;

  const int grid_center = (HEIGHT / 2) * WIDTH + (WIDTH / 2);
  bool *grid = calloc(grid_size, sizeof(bool));
  if (grid == NULL) {
    perror("could not allocate memory for grid");
    return -1;
  }

  grid[grid_center] = 1;

  unsigned int spawn_site_distribution = UINT_MAX;
  printf("1. Side-uniform Distribution\n2. Pixel-uniform Distribution\n");

  while (spawn_site_distribution != 1 && spawn_site_distribution != 2) {
    scanf("%u", &spawn_site_distribution);
  }

  for (int i = 0; i < ITERATIONS; i++) {
    int x, y;
    switch (spawn_site_distribution) {
    case 1:
      SpawnSideUniform(&x, &y);
      break;

    case 2:
      SpawnPixelUniform(&x, &y);
      break;
    }
  }

  if (Render(grid) == 1) {
    return 1;
  }

  free(grid);

  return 0;
}
