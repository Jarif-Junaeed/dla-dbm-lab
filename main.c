#include "pcg_basic.h"
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ITERATIONS 10000
#define COLOR_CHANNELS 3
#define WIDTH 600
#define HEIGHT 400

struct gridDelta {
  int dest;
  int src;
};

inline int PixelIndex(int pixel) { return pixel * COLOR_CHANNELS; }

inline int GridIndex(int x, int y) { return y * WIDTH + x; }

int InitSeedSequence(uint64_t *seed, uint64_t *sequence) {
  unsigned char random_buffer[8];
  ssize_t seed_n = getrandom(random_buffer, sizeof(random_buffer), 0);
  if (seed_n != sizeof(random_buffer)) {
    return 1;
  }
  memcpy(seed, random_buffer, sizeof(*seed));

  ssize_t sequence_n = getrandom(random_buffer, sizeof(random_buffer), 0);
  if (sequence_n != sizeof(random_buffer)) {
    return 1;
  }
  memcpy(sequence, random_buffer, sizeof(*sequence));

  return 0;
}

static void ensure_capacity(int *index_count, int *capacity, struct gridDelta **grid_deltas) {
  if (*index_count >= *capacity) {
    *capacity *= 2;
    struct gridDelta *tmp = realloc(*grid_deltas, *capacity * sizeof(struct gridDelta));
    if (tmp == NULL) { perror("gridDelta realloc failed"); exit(1); }
    *grid_deltas = tmp;
  }
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

int Walk(int *index_count, int *capacity, int x, int y, bool *grid, struct gridDelta **grid_deltas, pcg32_random_t *rng) {
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

    if (next_x < 0 || next_x >= WIDTH || next_y < 0 || next_y >= HEIGHT) {
      ensure_capacity(index_count, capacity, grid_deltas);
      (*grid_deltas)[(*index_count)++] = (struct gridDelta){.dest = -1, .src = GridIndex(x, y)};
      return 0;
    }

    if ((next_y > 0 && grid[GridIndex(next_x, next_y - 1)]) ||
        (next_y < HEIGHT - 1 && grid[GridIndex(next_x, next_y + 1)]) ||
        (next_x > 0 && grid[GridIndex(next_x - 1, next_y)]) ||
        (next_x < WIDTH - 1 && grid[GridIndex(next_x + 1, next_y)])) {

      grid[GridIndex(next_x, next_y)] = 1;
      
      ensure_capacity(index_count, capacity, grid_deltas);
      (*grid_deltas)[(*index_count)++] = (struct gridDelta){
        .dest = GridIndex(next_x, next_y), 
        .src = GridIndex(x, y)
      };

      return 0;
    } else if ( i == ITERATIONS) {
      ensure_capacity(index_count, capacity, grid_deltas);
      (*grid_deltas)[(*index_count)++] = (struct gridDelta){.dest = GridIndex(next_x, next_y), .src = GridIndex(x, y)};
      ensure_capacity(index_count, capacity, grid_deltas);
      (*grid_deltas)[(*index_count)++] = (struct gridDelta){.dest = -1, .src = GridIndex(next_x, next_y)};
      return 0;
    }

    ensure_capacity(index_count, capacity, grid_deltas);
    (*grid_deltas)[(*index_count)++] = (struct gridDelta){.dest = GridIndex(next_x, next_y), .src = GridIndex(x, y)};
    x = next_x;
    y = next_y;
    
  }
  return 0;
}

int main(void) {
  uint64_t seed;
  uint64_t sequence;
  if (InitSeedSequence(&seed, &sequence) == 1) {
    perror("InitSeedSequence failed");
    return 1;
  }
  pcg32_random_t rng;
  pcg32_srandom_r(&rng, seed, sequence);

  const unsigned int grid_size = WIDTH * HEIGHT;

  const int grid_center = GridIndex((WIDTH / 2), (HEIGHT / 2));
  bool *grid = calloc(grid_size, sizeof(bool));
  if (grid == NULL) {
    perror("could not allocate memory for grid");
    return -1;
  }

  //struct gridDelta *grid_deltas = malloc((ITERATIONS * (ITERATIONS + 1) + 1) * sizeof(struct gridDelta));
  int capacity = 500;
  struct gridDelta *grid_deltas = malloc(capacity * sizeof(struct gridDelta));
  if (grid_deltas == NULL) {
    perror("could not allocate memory for grid");
    return -1;
  }
  grid[grid_center] = 1;
  grid_deltas[0] = (struct gridDelta){.dest = grid_center, .src = grid_center};

  unsigned int spawn_site_distribution = UINT_MAX;
  printf("1. Side-uniform Distribution\n2. Pixel-uniform Distribution\n");

  while (spawn_site_distribution != 1 && spawn_site_distribution != 2) {
    scanf("%u", &spawn_site_distribution);
  }

  int index_count = 1;
  for (int i = 1; i <= ITERATIONS; i++) {
    int x, y;
    switch (spawn_site_distribution) {
    case 1:
      SpawnSideUniform(&x, &y, &rng);
      break;

    case 2:
      SpawnPixelUniform(&x, &y, &rng);
      break;
    }

    ensure_capacity(&index_count, &capacity, &grid_deltas);
    grid_deltas[index_count++] = (struct gridDelta){.dest = GridIndex(x, y), .src = GridIndex(x, y)};

    if(Walk(&index_count, &capacity, x, y, grid, &grid_deltas, &rng) == 1) {
      return 1;
    };
  }

  if (Render(grid) == 1) {
    return 1;
  }

  free(grid);
  free(grid_deltas);

  return 0;
}
