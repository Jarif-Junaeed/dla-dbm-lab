#include "spawn.h"

#include <math.h>
#include <stdint.h>

#include "config.h"
#include "grid.h"

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
