#define _POSIX_C_SOURCE 200809L

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

#define ITERATIONS 1000
#define COLOR_CHANNELS 3
#define WIDTH 600
#define HEIGHT 400
#define DEFAULT_RADIUS 10.00

#define BIT_MASK_32 0xFFFFFFFFu

static inline size_t PixelIndex(int pixel) { return pixel * COLOR_CHANNELS; }

static inline size_t GridIndexFromCoords(int x, int y) { return (size_t)y * WIDTH + (size_t)x; }
static inline uint64_t GridCoordsFromIndex(size_t index) {
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

void SpawnCircumferenceUniform(int *x, int *y, double radius, size_t center, pcg32_random_t *rng) {
  uint64_t center_coords = GridCoordsFromIndex(center);
  int center_x = (center_coords >> 32) & BIT_MASK_32;
  int center_y = center_coords & BIT_MASK_32;

  double r = (double)pcg32_random_r(rng) / UINT32_MAX;
  *x = (radius * sin(r * 2 * PI) + center_x);
  *y = (radius * cos(r * 2 * PI) + center_y);
}

int Render(bool *grid) {
  const size_t screen_size = WIDTH * HEIGHT * COLOR_CHANNELS;
  const size_t pixel_count = WIDTH * HEIGHT;

  uint8_t *screen = calloc(screen_size, sizeof(uint8_t));
  if (screen == NULL) {
    perror("Could not allocate memory for screen");
    return 1;
  }

  for (size_t pixel = 0; pixel < pixel_count; pixel++) {
    size_t i = pixel * COLOR_CHANNELS;
    
    if (grid[pixel] == 0) {
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
    return 1;
  }

  fprintf(fp, "P3\n%d %d\n255\n", WIDTH, HEIGHT);

  for (size_t pixel = 0; pixel < pixel_count; pixel++) {
    int i = pixel * COLOR_CHANNELS;

    fprintf(fp, "%d %d %d\n", screen[i], screen[i + 1], screen[i + 2]);
  }

  free(screen);
  fclose(fp);

  return 0;
}

int AnimateAggregation(struct gridDeltas *grid_deltas) {
  const size_t screen_size = WIDTH * HEIGHT * COLOR_CHANNELS;

  uint8_t *screen = calloc(screen_size, sizeof(uint8_t));
  if (screen == NULL) {
    perror("Could not allocate memory for screen");
    return 1;
  }

  char command[1024];

  snprintf(command, sizeof(command),
           "ffmpeg -y "
           "-f rawvideo "
           "-pixel_format rgb24 "
           "-video_size %zux%zu "
           "-framerate 120 "
           "-i - "
           "-c:v libx264 "
           "-pix_fmt yuv420p "
           "output/animation.mp4",
           (size_t)WIDTH, (size_t)HEIGHT);

  FILE *ffmpeg = popen(command, "w");

  if (ffmpeg == NULL) {
    perror("Could not start ffmpeg");
    free(screen);
    return 1;
  }

  for (size_t frame = 0; frame < grid_deltas->m_used_capacity; frame++) {
    size_t src = grid_deltas->m_grid_deltas[frame].src * COLOR_CHANNELS;
    size_t dest = (grid_deltas->m_grid_deltas[frame].dest == SIZE_MAX)
                      ? SIZE_MAX
                      : grid_deltas->m_grid_deltas[frame].dest * COLOR_CHANNELS;

    if (src == dest) {
      screen[src] = 255;
      screen[src + 1] = 255;
      screen[src + 2] = 255;
    } else if (dest == SIZE_MAX) {
      // Remove Walker from src
      screen[src] = 0;
      screen[src + 1] = 0;
      screen[src + 2] = 0;
    } else {
      // Remove Walker from src
      screen[src] = 0;
      screen[src + 1] = 0;
      screen[src + 2] = 0;

      // Add Walker at dest
      screen[dest] = 255;
      screen[dest + 1] = 255;
      screen[dest + 2] = 255;
    }

    fwrite(screen, 1, screen_size, ffmpeg);
  }

  pclose(ffmpeg);
  free(screen);
  return 0;
}

void Walk(struct gridDeltas *grid_deltas, int x, int y, size_t grid_center, double *radius, bool *grid, pcg32_random_t *rng) {
  for (size_t i = 0; i < ITERATIONS; i++) {
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
      grid_deltas->m_ReduceGridDeltasUsedCapacity(grid_deltas, i+1);
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
    else if ( i == ITERATIONS - 1) {
      grid_deltas->m_ReduceGridDeltasUsedCapacity(grid_deltas, i+1);
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
  if ((ITERATIONS * ITERATIONS * sizeof(size_t)) >= SIZE_MAX) {
    perror("ITERATION count too high. It should be lower than SIZE_MAX (18446744073709551615)");
    return 1;
  }

  pcg32_random_t rng1;
  InitRNG(&rng1);

  const size_t grid_size = WIDTH * HEIGHT;

  const size_t grid_center = GridIndexFromCoords((WIDTH / 2), (HEIGHT / 2));
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
  for (size_t i = 1; i <= ITERATIONS; i++) {
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

  printf("Reached the End of Simulation\n");

  if (Render(grid) == 1) {
    return 1;
  }

  printf("Rendered Final State of DLA run\n");

  if (AnimateAggregation(&grid_deltas) == 1) {
    return 1;
  }

  printf("Animated DLA run\n");

  free(grid);
  GridDeltasDestroy(&grid_deltas);

  return 0;
}
