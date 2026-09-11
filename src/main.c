#include "pcg_basic.h"
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "grid.h"
#include "griddelta.h"
#include "render.h"
#include "rng.h"
#include "spawn.h"
#include "stats.h"
#include "walker.h"

int main(void) {
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

  struct stats grid_stats = StatsCreate();

  unsigned int spawn_site_distribution = UINT_MAX;
  printf("1. Side-uniform Distribution\n2. Pixel-uniform Distribution\n3. Circle Circumference-uniform Distribution\n");

  while (spawn_site_distribution != 1 && spawn_site_distribution != 2 && spawn_site_distribution != 3) {
    if (scanf("%u", &spawn_site_distribution) == 1) {
      int c;
      while((c = getchar()) != '\n' && c != EOF) {};
    };
  }

  double radius = (spawn_site_distribution == 3) ? DEFAULT_RADIUS : -1.00;
  for (size_t i = 1; i <= WALKER_COUNT; i++) {
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
    grid_stats.m_total_walkers++;

    Walk(&grid_deltas, &grid_stats, x, y, grid_center, &radius, grid, &rng1);
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
  StatsDestroy(&grid_stats);

  return 0;
}
