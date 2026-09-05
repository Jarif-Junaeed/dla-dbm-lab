#include "pcg_basic.h"
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "griddelta.h"

#define PI 3.1415926536

#define ITERATIONS 10000
#define COLOR_CHANNELS 3
#define WIDTH 600
#define HEIGHT 400
#define DEFAULT_RADIUS 10.00

#define BIT_MASK_32 0xFFFFFFFFu

static inline int PixelIndex(int pixel) { return pixel * COLOR_CHANNELS; }

static inline int GridIndexFromCoords(int x, int y) { return y * WIDTH + x; }
static inline uint64_t GridCoordsFromIndex(int index) {
  int y = index / WIDTH;
  int x = index % WIDTH;

  uint64_t coords = 0;
  coords = (coords | x) << 32;
  coords = (coords | y);

  return coords;
}

static double EuclideanDistance(int x1, int y1, int x2, int y2) {
  int delx = x2 - x1;
  int dely = y2 - y1;
  
  return sqrt((delx * delx) + (dely * dely));
}

void InitRNG(pcg32_random_t *rng) {
  uint64_t seed;
  uint64_t sequence;
  unsigned char random_buffer[8];
  ssize_t seed_n = getrandom(random_buffer, sizeof(random_buffer), 0);
  if (seed_n != sizeof(random_buffer)) {
    perror("CreateRNG() failed, unable to get seed");
    exit(1);
  }
  memcpy(&seed, random_buffer, sizeof(seed));

  ssize_t sequence_n = getrandom(random_buffer, sizeof(random_buffer), 0);
  if (sequence_n != sizeof(random_buffer)) {
    perror("CreateRNG() failed, unable to get sequence");
    exit(1);
  }
  memcpy(&sequence, random_buffer, sizeof(sequence));

  pcg32_srandom_r(rng, seed, sequence);
}

void SpawnSideUniform(int *x, int *y, pcg32_random_t *rng) {
  int w = WIDTH - 1;
  int h = HEIGHT - 1;
  int side = pcg32_random_r(rng) & 3;
  switch (side) {
  case 0:
    *x = pcg32_random_r(rng) % w + 1;
    *y = 0;
    break;
  case 1:
    *x = WIDTH - 1;
    *y = pcg32_random_r(rng) % h + 1;
    break;
  case 2:
    *x = pcg32_random_r(rng) % w;
    *y = HEIGHT - 1;
    break;
  case 3:
    *x = 0;
    *y = pcg32_random_r(rng) % h;
    break;
  }
}

void SpawnPixelUniform(int *x, int *y, pcg32_random_t *rng) {
  int w = WIDTH - 1;
  int h = HEIGHT - 1;
  int perimeter = 2 * (w + h);
  int r = pcg32_random_r(rng) % perimeter;

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

void SpawnCircumferenceUniform(int *x, int *y, double radius, int center, pcg32_random_t *rng) {
  uint64_t center_coords = GridCoordsFromIndex(center);
  int center_x = (center_coords >> 32) & BIT_MASK_32;
  int center_y = center_coords & BIT_MASK_32;

  double r = (double)pcg32_random_r(rng) / UINT32_MAX;
  *x = (radius * sin(r * 2 * PI) + center_x);
  *y = (radius * cos(r * 2 * PI) + center_y);
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
    free(screen);
    return 1;
  }

  FILE *fp = fopen("output/image.ppm", "w");
  if (fp == NULL) {
    perror("Could not open image.ppm");
    free(screen);
    fclose(fp);
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

void Walk(struct gridDeltas *grid_deltas, int x, int y, int grid_center, double *radius, bool *grid, pcg32_random_t *rng) {
  for (int i = 1; i <= ITERATIONS; i++) {
    unsigned int direction = pcg32_random_r(rng) & 3;
    int next_x = x;
    int next_y = y;
    switch (direction) {
    case 0:
      next_y--;
      break;
    case 1:
      next_x++;
      break;
    case 2:
      next_y++;
      break;
    case 3:
      next_x--;
      break;
    }

    // Walker went out of bounds
    if (next_x < 0 || next_x >= WIDTH || next_y < 0 || next_y >= HEIGHT) {
      grid_deltas->m_RecordGridDelta(grid_deltas, -1, GridIndexFromCoords(x, y));
      return;
    }

    // Walker sticks to the aggregate
    if ((next_y > 0 && grid[GridIndexFromCoords(next_x, next_y - 1)]) ||
        (next_y < HEIGHT - 1 && grid[GridIndexFromCoords(next_x, next_y + 1)]) ||
        (next_x > 0 && grid[GridIndexFromCoords(next_x - 1, next_y)]) ||
        (next_x < WIDTH - 1 && grid[GridIndexFromCoords(next_x + 1, next_y)])) {

      grid[GridIndexFromCoords(next_x, next_y)] = 1;
      grid_deltas->m_RecordGridDelta(grid_deltas, GridIndexFromCoords(next_x, next_y), GridIndexFromCoords(x, y));

      int maximum_radius = (WIDTH > HEIGHT) ? (HEIGHT/ 2) : (WIDTH / 2);
      if (*radius != -1 && ((*radius) < maximum_radius)) {
        uint64_t grid_center_coords = GridCoordsFromIndex(grid_center);
        int x1 = (grid_center_coords >> 32) & BIT_MASK_32;
        int y1 = grid_center_coords & BIT_MASK_32;
        int new_radius = EuclideanDistance(x1, y1, next_x, next_y); 
        if (new_radius > (*radius)) *radius = new_radius;
      }

      return;
    } 
    // if walker does not stick to aggregate within ITERATIONS, then we kill it
    else if ( i == ITERATIONS) {
      grid_deltas->m_RecordGridDelta(grid_deltas, GridIndexFromCoords(next_x, next_y), GridIndexFromCoords(x, y));
      grid_deltas->m_RecordGridDelta(grid_deltas, -1, GridIndexFromCoords(next_x, next_y));
      return;
    }

    // Walker moved to a valid position that is not adjacent to the aggregate within ITERATIONS
    grid_deltas->m_RecordGridDelta(grid_deltas, GridIndexFromCoords(next_x, next_y), GridIndexFromCoords(x, y));
    x = next_x;
    y = next_y;
    
  }
  return;
}

int main(void) {
  pcg32_random_t rng1;
  InitRNG(&rng1);

  const unsigned int grid_size = WIDTH * HEIGHT;

  const int grid_center = GridIndexFromCoords((WIDTH / 2), (HEIGHT / 2));
  bool *grid = calloc(grid_size, sizeof(bool));
  if (grid == NULL) {
    perror("could not allocate memory for grid");
    return -1;
  }

  struct gridDeltas grid_deltas = GridDeltasCreate(500);

  grid[grid_center] = 1;
  grid_deltas.m_RecordGridDelta(&grid_deltas, grid_center, grid_center);

  unsigned int spawn_site_distribution = UINT_MAX;
  printf("1. Side-uniform Distribution\n2. Pixel-uniform Distribution\n3. Circle Circumference-uniform Distribution\n");

  while (spawn_site_distribution != 1 && spawn_site_distribution != 2 && spawn_site_distribution != 3) {
    if (scanf("%u", &spawn_site_distribution) == 1) {
      int c;
      while((c = getchar()) != '\n' && c != EOF) {};
    };
  }

  double radius = (spawn_site_distribution == 3) ? DEFAULT_RADIUS : -1.00;
  for (int i = 1; i <= ITERATIONS; i++) {
    int x, y;
    switch (spawn_site_distribution) {
    case 1:
      SpawnSideUniform(&x, &y, &rng1);
      break;

    case 2:
      SpawnPixelUniform(&x, &y, &rng1);
      break;

    case 3:
      SpawnCircumferenceUniform(&x, &y, radius, grid_center, &rng1);
      break;
    }

    grid_deltas.m_RecordGridDelta(&grid_deltas, GridIndexFromCoords(x, y), GridIndexFromCoords(x, y));

    Walk(&grid_deltas, x, y, grid_center, &radius, grid, &rng1);
  }

  if (Render(grid) == 1) {
    return 1;
  }

  free(grid);
  GridDeltasDestroy(&grid_deltas);

  return 0;
}
