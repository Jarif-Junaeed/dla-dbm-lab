#include "spawn.h"

#include <math.h>
#include <stdint.h>

#include "cluster.h"
#include "config.h"

void SpawnSideUniform(struct point *p, pcg32_random_t *rng) {
  int w = WIDTH - 1;
  int h = HEIGHT - 1;
  int side = pcg32_random_r(rng) & 3;
  switch (side) {
  case 0:
    p->x = pcg32_random_r(rng) % w + 1;
    p->y = 0;
    break;
  case 1:
    p->x = WIDTH - 1;
    p->y = pcg32_random_r(rng) % h + 1;
    break;
  case 2:
    p->x = pcg32_random_r(rng) % w;
    p->y = HEIGHT - 1;
    break;
  case 3:
    p->x = 0;
    p->y = pcg32_random_r(rng) % h;
    break;
  }
}

void SpawnPixelUniform(struct point *p, pcg32_random_t *rng) {
  int w = WIDTH - 1;
  int h = HEIGHT - 1;
  int perimeter = 2 * (w + h);
  int r = pcg32_random_r(rng) % perimeter;

  if (r < w) {
    p->x = r + 1;
    p->y = 0;
  } else if (r < w + h) {
    p->x = w;
    p->y = (r - w) + 1;
  } else if (r < (2 * w + h)) {
    p->x = r - (w + h);
    p->y = h;
  } else {
    p->x = 0;
    p->y = r - ((2 * w) + h);
  }
}

void SpawnCircumferenceUniform(struct point *p, double radius, size_t center, pcg32_random_t *rng) {
  struct point center_point = ClusterCoordsFromIndex(center);

  double r = (double)pcg32_random_r(rng) / UINT32_MAX;
  p->x = (radius * sin(r * 2 * PI) + center_point.x);
  p->y = (radius * cos(r * 2 * PI) + center_point.y);
}
